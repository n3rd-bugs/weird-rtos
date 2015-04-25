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

#ifdef CONFIG_SEMAPHORE
    /* Create the networking device instance semaphore. */
    semaphore_create(&net_dev_data.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

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

    /* Will only work with buffered file descriptors. */
    OS_ASSERT((((FS *)fd)->flags & FS_BUFFERED) == 0);

    /* Clear the networking device instance data. */
    memset(net_device, 0, sizeof(NET_DEV));

    /* Save the file descriptor for this networking device. */
    net_device->fd = fd;

    /* Save the transmit function. */
    net_device->tx = tx;

    /* Initialize connection watcher. */
    net_device->connection_watcher.data = net_device;
    net_device->connection_watcher.connected = &net_device_connected;
    net_device->connection_watcher.disconnected = &net_device_disconnected;

    /* Register connection watcher. */
    fs_connection_watcher_set(fd, &net_device->connection_watcher);

#ifdef CONFIG_SEMAPHORE
    /* Create the semaphore for this networking device. */
    semaphore_create(&net_device->lock, 1, 1, SEMAPHORE_PRIORITY);

    /* Obtain the global data semaphore. */
    OS_ASSERT(semaphore_obtain(&net_dev_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Add this device on the global device list. */
    sll_append(&net_dev_data.devices, net_device, OFFSETOF(NET_DEV, next));

#ifdef NET_IPV4

    /* Initialize IPv4 data for this networking device. */
    ipv4_device_initialize(net_device);

#endif

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&net_dev_data.lock);
#endif

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
#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data semaphore. */
    OS_ASSERT(semaphore_obtain(&net_dev_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Pick the device list head. */
    ret_device = net_dev_data.devices.head;

    /* Search the device list for the required device. */
    while ((ret_device) && (ret_device->fd != fd))
    {
        /* Get the next device. */
        ret_device = ret_device->next;
    }

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&net_dev_data.lock);
#endif

    /* Return the required device. */
    return (ret_device);

} /* net_device_get_fd */

/*
 * net_device_get_lock
 * @net_device: Networking device for which lock is needed to be acquired.
 * @return: A success status will be returned if lock for the device was
 *  successfully obtained.
 * This function will acquire lock or a networking device.
 */
int32_t net_device_get_lock(void *net_device)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain the lock for this networking device. */
    return (semaphore_obtain(&((NET_DEV *)net_device)->lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();

    /* Always return success. */
    return (SUCESS);
#endif
} /* net_device_get_lock */

/*
 * net_device_release_lock
 * @net_device: Networking device for which lock is needed to be released.
 * This function will acquire lock or a networking device.
 */
void net_device_release_lock(NET_DEV *net_device)
{
#ifdef CONFIG_SEMAPHORE
    /* Release the lock for this networking device. */
    semaphore_release(&net_device->lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* net_device_release_lock */

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

    /* Obtain the lock for this networking device. */
    OS_ASSERT(net_device_get_lock(net_device) != SUCCESS);

    /* Set the MTU for this networking device. */
    net_device->mtu = mtu;

    /* Release the lock for this networking device. */
    net_device_release_lock(net_device);

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

    /* Obtain the lock for this networking device. */
    OS_ASSERT(net_device_get_lock(net_device) != SUCCESS);

    /* Save the MTU for this networking device. */
    ret_mtu = net_device->mtu;

    /* Release the lock for this networking device. */
    net_device_release_lock(net_device);

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

    /* Write this buffer to the networking buffer file descriptor. */
    OS_ASSERT(fs_write(net_buff_fd, (char *)buffer, sizeof(FS_BUFFER *)) != sizeof(FS_BUFFER *));

} /* net_device_buffer_receive */

/*
 * net_device_buffer_transmit
 * @buffer: A net buffer needed to be transmitted.
 * @protocol: Packet protocol as parsed by the upper layers.
 * @return: A success status will be returned if given buffer was successfully
 *  transmitted.
 * This function will be called by networking protocols when a packet is needed
 * to be transmitted.
 */
int32_t net_device_buffer_transmit(FS_BUFFER *buffer, uint8_t protocol)
{
    int32_t status = SUCCESS;
    NET_DEV *net_device = net_device_get_fd(buffer->fd);
    FS_BUFFER *next_buffer;

    /* If networking device was successfully resolved. */
    if (net_device != NULL)
    {
        /* Obtain the lock for this networking device. */
        OS_ASSERT(net_device_get_lock(net_device) != SUCCESS);

        /* While we have a buffer to transmit. */
        while ((buffer != NULL) & (status == SUCCESS))
        {
            /* Save the next buffer we need to send. */
            next_buffer = buffer->next;

            /* Push the protocol on this buffer. */
            OS_ASSERT(fs_buffer_push(buffer, &protocol, sizeof(uint8_t), FS_BUFFER_HEAD) != SUCCESS);

            /* Transmit this buffer on the networking device. */
            status = net_device->tx(buffer);

            /* Pick the next buffer. */
            buffer = next_buffer;
        }

        /* Release the lock for this networking device. */
        net_device_release_lock(net_device);
    }

    else
    {
        /* We did not find a valid networking device for given buffer. */
        status = NET_INVALID_FD;
    }

    /* Return status to the caller. */
    return (status);

} /* net_device_buffer_transmit */

/*
 * net_device_connected
 * @fd: File descriptor for which connection is established.
 * @net_device: Networking device that is now connected.
 * This function will be called whenever a connection is established for a
 * registered file descriptor.
 */
void net_device_connected(void *fd, void *net_device)
{
    /* Obtain the lock for this networking device. */
    OS_ASSERT(net_device_get_lock(net_device) != SUCCESS);

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Release the lock for this networking device. */
    net_device_release_lock(net_device);

} /* net_device_connected */

/*
 * net_device_disconnected
 * @fd: File descriptor for which connection was terminated.
 * @net_device: Networking device instance data.
 * This function will be called whenever a connection is disconnected for a
 * registered file descriptor.
 */
void net_device_disconnected(void *fd, void *net_device)
{
    /* Obtain the lock for this networking device. */
    OS_ASSERT(net_device_get_lock(net_device) != SUCCESS);

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Release the lock for this networking device. */
    net_device_release_lock(net_device);

} /* net_device_disconnected */

#endif /* CONFIG_NET */
