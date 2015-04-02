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
#include <net.h>

/*
 * net_register_fd
 * @net_device: Associated networking device structure.
 * @fd: File descriptor needed to be registered with networking layer.
 * This function will register a file descriptor with networking layer.
 */
void net_register_fd(NET_DEV *net_device, FD fd)
{
    /* Will only work with buffered file descriptors. */
    OS_ASSERT((((FS *)fd)->flags & FS_BUFFERED) == 0);

    /* Clear the networking device instance data. */
    memset(net_device, 0, sizeof(NET_DEV));

    /* Initialize data watcher. */
    net_device->data_watcher.data = net_device;
    net_device->data_watcher.data_available = &net_device_rx_watcher;

    /* Register data watcher. */
    fs_data_watcher_set(fd, &net_device->data_watcher);

    /* Initialize connection watcher. */
    net_device->connection_watcher.data = net_device;
    net_device->connection_watcher.connected = &net_device_connected;
    net_device->connection_watcher.disconnected = &net_device_disconnected;

    /* Register connection watcher. */
    fs_connection_watcher_set(fd, &net_device->connection_watcher);

#ifdef CONFIG_SEMAPHORE
    /* Create the PPP instance semaphore. */
    semaphore_create(&net_device->lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

} /* net_register_fd */

/*
 * net_device_buffer_receive
 * @buffer: A net buffer needed to be added in the receive list.
 * @protocol: Packet protocol as parsed on the lower layer needed to parse the
 *  contents of this buffer.
 * This function will be called by a device when it wants to transfer a buffer
 * to the networking stack, the device should already have registered itself
 * with the networking stack. Purpose of this task is to off-load the network
 * packet manipulation to the networking receive task, in-case the driver uses
 * interrupts, please note that for the known reasons it is possible we miss
 * to lock the networking buffer chain so this packet will lie in the receive
 * queue until a RX event is raised.
 */
void net_device_buffer_receive(FS_BUFFER *buffer, uint8_t protocol)
{
    /* Assign this buffer a network buffer ID. */
    buffer->id = NET_BUFFER_ID;

    /* Push the protocol on the buffer. */
    OS_ASSERT(fs_buffer_push(buffer, (char *)&protocol, sizeof(uint8_t), FS_BUFFER_HEAD) != SUCCESS);

    /* Push this buffer in the receive list of the device. */
    fs_buffer_add(buffer->fd, buffer, FS_BUFFER_RX, FS_BUFFER_ACTIVE);

} /* net_device_buffer_receive */

/*
 * net_device_connected
 * @fd: File descriptor for which connection is established.
 * @ppp: PPP file descriptor data.
 * This function will be called whenever a connection is established for a
 * registered file descriptor.
 */
void net_device_connected(void *fd, void *net_device)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Acquire global data lock for PPP. */
    OS_ASSERT(semaphore_obtain(&((NET_DEV *)net_device)->lock, MAX_WAIT) != SUCCESS)
#endif

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global data lock. */
    semaphore_release(&((NET_DEV *)net_device)->lock);
#endif

} /* net_device_connected */

/*
 * net_device_disconnected
 * @fd: File descriptor for which connection was terminated.
 * @net_device: Net device instance data.
 * This function will be called whenever a connection is disconnected for a
 * registered file descriptor.
 */
void net_device_disconnected(void *fd, void *net_device)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Acquire global data lock for PPP. */
    OS_ASSERT(semaphore_obtain(&((NET_DEV *)net_device)->lock, MAX_WAIT) != SUCCESS)
#endif

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global data lock. */
    semaphore_release(&((NET_DEV *)net_device)->lock);
#endif

} /* net_device_disconnected */

/*
 * net_device_rx_watcher
 * @fd: File descriptor for which new data is available.
 * @net_device: Net device instance data.
 * This function will be called when even there is some data available to read
 * from a file descriptor.
 */
void net_device_rx_watcher(void *fd, void *net_device)
{
    FS_BUFFER *buffer;

#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Acquire global data lock for this networking device. */
    if (semaphore_obtain(&((NET_DEV *)net_device)->lock, MAX_WAIT) == SUCCESS)
#endif
    {
        /* Pull all the incoming net buffers on this device. */
        do
        {
            /* Check if we have a networking buffer. */
            buffer = fs_buffer_get_by_id(fd, FS_BUFFER_RX, FS_BUFFER_ACTIVE, NET_BUFFER_ID);

            /* If we do have a buffer. */
            if (buffer)
            {
                /* Write this buffer to the networking buffer file descriptor. */
                fs_write(net_buff_fd, (char *)buffer, sizeof(FS_BUFFER *));
            }

        } while (buffer != NULL);

#ifdef CONFIG_SEMAPHORE
        /* Release the global data lock. */
        semaphore_release(&((NET_DEV *)net_device)->lock);
#else
        /* Enable scheduling. */
        scheduler_unlock();
#endif
    }

} /* net_device_rx_watcher */

#endif /* CONFIG_NET */
