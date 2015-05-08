/*
 * net_device.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_NET
#include <string.h>
#include <sll.h>
#include <net.h>

/* Global network device data. */
NET_DEV_DATA net_dev_data;
/*
 * net_devices_init
 * This function initialize global data structure for networking devices.
 */
void net_devices_init()
{
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
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Will only work with buffered file descriptors. */
    OS_ASSERT((((FS *)fd)->flags & FS_BUFFERED) == 0);

    /* Clear the networking device instance data. */
    memset(net_device, 0, sizeof(NET_DEV));

    /* Save the file descriptor for this networking device. */
    net_device->fd = fd;

    /* Save the transmit function. */
    net_device->tx = tx;

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Add this device on the global device list. */
    sll_append(&net_dev_data.devices, net_device, OFFSETOF(NET_DEV, next));

#ifdef NET_IPV4
    /* Initialize IPv4 data for this networking device. */
    ipv4_device_initialize(net_device);
#endif

    /* Restore the IRQ interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Get the condition data for this file descriptor. */
    fs_condition_get(fd, &condition, &net_device->suspend, &net_device->fs_param, FS_BLOCK_READ);

    /* Add networking condition for this file descriptor. */
    net_condition_add(condition, &net_device->suspend, rx, fd);

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
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Pick the device list head. */
    ret_device = net_dev_data.devices.head;

    /* Search the device list for the required device. */
    while ((ret_device) && (ret_device->fd != fd))
    {
        /* Get the next device. */
        ret_device = ret_device->next;
    }

    /* Restore the IRQ interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

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

    /* Set the MTU for this networking device. */
    net_device->mtu = mtu;

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

    /* Save the MTU for this networking device. */
    ret_mtu = net_device->mtu;

    /* Return the MTU for this networking device. */
    return(ret_mtu);

} /* net_device_get_mtu */

/*
 * net_device_buffer_receive
 * @buffer: A net buffer needed to be added in the receive list.
 * @protocol: Packet protocol as parsed on the lower layer required by upper
 *  layers to parse the contents of this buffer.
 * This function will be called by a device when it wants to transfer a buffer
 * to the networking stack, the device should already have registered itself
 * with the networking stack.
 */
void net_device_buffer_receive(FS_BUFFER *buffer, uint8_t protocol)
{
    /* Push the protocol on the buffer. */
    OS_ASSERT(fs_buffer_push(buffer, &protocol, sizeof(uint8_t), FS_BUFFER_HEAD) != SUCCESS);

    /* Release lock for buffer file descriptor. */
    fd_release_lock(buffer->fd);

    /* Write this buffer to the networking buffer file descriptor. */
    OS_ASSERT(fs_write(net_buff_fd, (uint8_t *)buffer, sizeof(FS_BUFFER *)) != sizeof(FS_BUFFER *));

    /* Again obtain lock for buffer file descriptor. */
    OS_ASSERT(fd_get_lock(buffer->fd));

} /* net_device_buffer_receive */

/*
 * net_device_buffer_transmit
 * @buffer: A net buffer needed to be transmitted.
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
int32_t net_device_buffer_transmit(FS_BUFFER *buffer, uint8_t protocol, uint8_t flags)
{
    int32_t status = SUCCESS;
    NET_DEV *net_device;
    FS_BUFFER *tmp_buffer;

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
                OS_ASSERT(fs_buffer_push(buffer, &protocol, sizeof(uint8_t), (FS_BUFFER_HEAD | flags)) != SUCCESS);

                /* Transmit this buffer on the networking device. */
                status = net_device->tx(buffer, flags);

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
                    fs_buffer_add(buffer->fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
                }

                /* Pick the next buffer. */
                buffer = tmp_buffer;
            }

            /* If operation was not successful and there are still some buffers
             * to be sent. */
            if ((status != SUCCESS) && (buffer != NULL))
            {
                /* Free any buffers remaining on the buffer list. */
                fs_buffer_add_buffer_list(buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
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

    /* Return status to the caller. */
    return (status);

} /* net_device_buffer_transmit */

/*
 * net_device_link_up
 * @fd: File descriptor associated with a networking device.
 * This function will be called whenever link is up for this networking
 * device.
 */
void net_device_link_up(FD fd)
{
    NET_DEV *net_device = net_device_get_fd(fd);

    /* Set this device UP. */
    net_device->flags |= NET_DEVICE_UP;

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

    /* Clear the UP flag for this device. */
    net_device->flags &= (uint32_t)(~(NET_DEVICE_UP));

} /* net_device_link_down */

#endif /* CONFIG_NET */
