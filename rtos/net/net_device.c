/*
 * net_device.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_NET
#include <string.h>
#include <sll.h>
#include <net.h>
#ifdef NET_DHCP
#include <net_dhcp.h>
#endif
#ifdef DHCP_CLIENT
#include <net_dhcp_client.h>
#endif

/* Global network device data. */
NET_DEV_DATA net_dev_data;

/*
 * net_devices_init
 * This function initialize global data structure for networking devices.
 */
void net_devices_init(void)
{
    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Clear the global data. */
    memset(&net_dev_data, 0, sizeof(NET_DEV_DATA));

} /* net_devices_init */

/*
 * net_register_fd
 * @net_device: Associated networking device structure.
 * @fd: File descriptor needed to be registered with networking layer.
 * @tx: Transmit function that will be called to send a packet on this device.
 * @rx: RX function that will be called to processing the incoming data.
 * This function will register a file descriptor with networking layer.
 */
void net_register_fd(NET_DEV *net_device, FD fd, NET_TX *tx, NET_RX *rx)
{
    CONDITION *condition;

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Will only work with buffered file descriptors. */
    ASSERT((((FS *)fd)->flags & FS_BUFFERED) == 0);

    /* Save the file descriptor for this networking device. */
    net_device->fd = fd;

    /* Save the transmit function. */
    net_device->tx = tx;

    /* Lock the scheduler. */
    scheduler_lock();

    /* Add this device on the global device list. */
    sll_append(&net_dev_data.devices, net_device, OFFSETOF(NET_DEV, next));

#ifdef NET_IPV4
    /* Initialize IPv4 data for this networking device. */
    ipv4_device_initialize(net_device);
#endif

    /* Set the file descriptor priority. */
    ((FS *)fd)->priority = NET_DEVICE_PRIORITY;

    /* Enable scheduling. */
    scheduler_unlock();

    /* Get the condition data for this file descriptor. */
    fs_condition_get(fd, &condition, &net_device->suspend, &net_device->fs_param, FS_BLOCK_READ);

    /* Add networking condition for this file descriptor. */
    net_condition_add(condition, &net_device->suspend, rx, fd);

    SYS_LOG_FUNCTION_EXIT(NET_DEVICE);

} /* net_register_fd */

/*
 * net_device_get_fd
 * @fd: File descriptor for which networking device is required.
 * @return: If not null the networking device associated with the given file
 *  descriptor will be returned here.
 * This function will return a networking device associated with given file
 * descriptor.
 */
NET_DEV *net_device_get_fd(FD fd)
{
    NET_DEV *ret_device;

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Disable preemption. */
    scheduler_lock();

    /* Pick the device list head. */
    ret_device = net_dev_data.devices.head;

    /* Search the device list for the required device. */
    while ((ret_device) && (ret_device->fd != fd))
    {
        /* Get the next device. */
        ret_device = ret_device->next;
    }

    /* Enable scheduling. */
    scheduler_unlock();

    SYS_LOG_FUNCTION_EXIT(NET_DEVICE);

    /* Return the required device. */
    return (ret_device);

} /* net_device_get_fd */

/*
 * net_device_set_mtu
 * @fd: File descriptor associated with a networking device.
 * @mtu: Required value of maximum transmission unit for this device.
 * This function will return a networking device associated with given file
 * descriptor.
 */
void net_device_set_mtu(FD fd, uint32_t mtu)
{
    NET_DEV *net_device = net_device_get_fd(fd);

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Set the MTU for this networking device. */
    net_device->mtu = mtu;

    SYS_LOG_FUNCTION_EXIT(NET_DEVICE);

} /* net_device_set_mtu */

/*
 * net_device_get_mtu
 * @fd: File descriptor associated with a networking device.
 * This function will return a MTU for a networking device associated with
 * given file descriptor.
 */
uint32_t net_device_get_mtu(FD fd)
{
    NET_DEV *net_device = net_device_get_fd(fd);
    uint32_t ret_mtu;

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Save the MTU for this networking device. */
    ret_mtu = net_device->mtu;

    SYS_LOG_FUNCTION_EXIT(NET_DEVICE);

    /* Return the MTU for this networking device. */
    return(ret_mtu);

} /* net_device_get_mtu */

/*
 * net_device_buffer_receive
 * @buffer: A networking buffer needed to be added in the receive list.
 * @protocol: Packet protocol as parsed on the lower layer required by upper
 *  layers to parse the contents of this buffer.
 * @flags: Associated flags for this buffer.
 * @return: A success status will be returned if buffer was successfully added
 *  to the networking buffer list.
 * This function will be called by a device when it wants to transfer a buffer
 * to the networking stack, the device should already have registered itself
 * with the networking stack.
 */
int32_t net_device_buffer_receive(FS_BUFFER_LIST *buffer, uint8_t protocol, uint32_t flags)
{
    int32_t status;

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Push the protocol on the buffer. */
    status = fs_buffer_list_push(buffer, &protocol, sizeof(uint8_t), FS_BUFFER_HEAD);

    if (status == SUCCESS)
    {
        /* Push the flag on the buffer. */
        status = fs_buffer_list_push(buffer, &flags, sizeof(uint32_t), FS_BUFFER_HEAD);
    }

    if (status == SUCCESS)
    {
        /* Release lock for buffer file descriptor. */
        fd_release_lock(buffer->fd);

        /* Write this buffer to the networking buffer file descriptor. */
        ASSERT(fs_write(net_buff_fd, (uint8_t *)buffer, sizeof(FS_BUFFER_LIST *)) != sizeof(FS_BUFFER_LIST *));

        /* Again obtain lock for buffer file descriptor. */
        ASSERT(fd_get_lock(buffer->fd) != SUCCESS);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(NET_DEVICE, status);

    /* Return status to the caller. */
    return (status);

} /* net_device_buffer_receive */

/*
 * net_device_buffer_transmit
 * @buffer: A networking buffer needed to be transmitted.
 * @protocol: Packet protocol as parsed by the upper layers.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if given buffer was successfully
 *  transmitted, NET_INVALID_FD will be returned if a valid device was not
 *  resolved for given file descriptor, NET_LINK_DOWN will be returned if
 *  device is in link-down state, NET_BUFFER_CONSUMED will be returned if the
 *  buffer was freed by the function and caller don't need to do it.
 * This function will be called by networking protocols when a packet is needed
 * to be transmitted.
 */
int32_t net_device_buffer_transmit(FS_BUFFER_LIST *buffer, uint8_t protocol, uint8_t flags)
{
    int32_t status = SUCCESS;
    NET_DEV *net_device;
    FS_BUFFER_LIST *tmp_buffer;

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Resolve the required networking device. */
    net_device = net_device_get_fd(buffer->fd);

    /* If we are actually in link-up state. */
    if (net_device->flags & NET_DEVICE_UP)
    {
        /* If networking device was successfully resolved. */
        if (net_device != NULL)
        {
            /* While we have a buffer to transmit. */
            while ((buffer != NULL) & (status == SUCCESS))
            {
                /* Save the next buffer pointer. */
                tmp_buffer = buffer->next;

                /* Push the protocol on this buffer. */
                status = fs_buffer_list_push(buffer, &protocol, sizeof(uint8_t), (FS_BUFFER_HEAD | flags));

                if (status == SUCCESS)
                {
                    /* Transmit this buffer on the networking device. */
                    status = net_device->tx(buffer, flags);
                }

                /* If driver consumed the buffer. */
                if (status == NET_BUFFER_CONSUMED)
                {
                    /* Reset the status. */
                    status = SUCCESS;
                }

                /* In any other case free this buffer. */
                else
                {
                    /* Free this buffer. */
                    fs_buffer_add(buffer->fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
                }

                /* Pick the next buffer. */
                buffer = tmp_buffer;
            }

            /* If operation was not successful and there are still some buffers
             * to be sent. */
            if ((status != SUCCESS) && (buffer != NULL))
            {
                /* Free any buffers remaining on the buffer list. */
                fs_buffer_add_list_list(buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
            }

            /* Set the status that buffer was consumed. */
            status = NET_BUFFER_CONSUMED;
        }

        else
        {
            /* We did not find a valid networking device for given buffer. */
            status = NET_INVALID_FD;
        }
    }
    else
    {
        /* Networking device on which we need to send data is linked down. */
        status = NET_LINK_DOWN;
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(NET_DEVICE, (status == NET_BUFFER_CONSUMED) ? SUCCESS : status);

    /* Return status to the caller. */
    return (status);

} /* net_device_buffer_transmit */

/*
 * net_device_link_up
 * @fd: File descriptor associated with a networking device.
 * This function will be called whenever link is up for this networking
 * device, the caller must have lock for this device, it will be released and
 * acquired again if required.
 */
void net_device_link_up(FD fd)
{
    NET_DEV *net_device = net_device_get_fd(fd);

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Set this device UP. */
    net_device->flags |= NET_DEVICE_UP;

#ifdef DHCP_CLIENT
    /* Start DHCP client. */
    net_dhcp_client_start(net_device);
#endif

    SYS_LOG_FUNCTION_EXIT(NET_DEVICE);

} /* net_device_link_up */

/*
 * net_device_link_down
 * @fd: File descriptor associated with a networking device.
 * This function will be called whenever link is down for this networking
 * device.
 */
void net_device_link_down(FD fd)
{
    NET_DEV *net_device = net_device_get_fd(fd);

    SYS_LOG_FUNCTION_ENTRY(NET_DEVICE);

    /* Clear the UP flag for this device. */
    net_device->flags &= (uint32_t)(~(NET_DEVICE_UP));

#ifdef DHCP_CLIENT
    /* Stop DHCP client. */
    net_dhcp_client_stop(net_device);
#endif

    /* Release lock for networking file descriptor. */
    fd_release_lock(fd);

#ifdef NET_IPV4
    /* Clear IP address for this device. */
    ipv4_set_device_address(fd, 0x0, 0x0);
#endif

    /* Acquire lock for networking file descriptor. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    SYS_LOG_FUNCTION_EXIT(NET_DEVICE);

} /* net_device_link_down */

#endif /* CONFIG_NET */
