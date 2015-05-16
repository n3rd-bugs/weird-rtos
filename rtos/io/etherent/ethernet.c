/*
 * spi.c
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

#ifdef CONFIG_ETHERNET
#include <string.h>
#include <ethernet.h>
#include <net_buffer.h>

/* Internal function prototypes. */
static int32_t ethernet_buffer_transmit(FS_BUFFER *, uint8_t);
static void ethernet_process(void *);
static int32_t ethernet_lock(void *);
static void ethernet_unlock(void *);

/*
 * ethernet_init
 * This function will initialize ethernet devices.
 */
void ethernet_init()
{
    /* Do target initialization for ethernet. */
    ETHERENET_TGT_INIT();

} /* ethernet_init */

/*
 * ethernet_regsiter
 * @device: Ethernet device instance needed to be registered.
 * @ethernet_init: Ethernet device initialization function.
 * @ethernet_interrupt: Ethernet interrupt callback.
 * This function will register an ethernet device.
 */
void ethernet_regsiter(ETH_DEVICE *device, ETH_INIT *initialize, ETH_INTERRUPT *interrupt)
{
    /* Initialize file system data. */
    device->fs.get_lock = &ethernet_lock;
    device->fs.release_lock = &ethernet_unlock;
    device->fs.timeout = MAX_WAIT;

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect this device. */
    memset(&device->lock, 0, sizeof(SEMAPHORE));
    semaphore_create(&device->lock, 1, 1, (SEMAPHORE_PRIORITY | ((interrupt != NULL) ? SEMAPHORE_IRQ : 0)));
#endif

    /* Initialize file system condition. */
    fs_condition_init(&device->fs);

    /* Register this file descriptor with file system. */
    fs_register(&device->fs);

    /* This will block on read, and all data that will be given to write must
     * be flushed, also set the data available flag to start the device
     * initialization, this is a buffered file descriptor. */
    device->fs.flags = (FS_BLOCK | FS_FLUSH_WRITE | ((initialize != NULL) ? FS_DATA_AVAILABLE : 0) | FS_BUFFERED);

    /* Register a networking device. */
    net_register_fd(&device->net_device, (FD)&device->fs, &ethernet_buffer_transmit, &ethernet_process);

    /* Set MTU for this device. */
    net_device_set_mtu((FD)&device->fs, ETH_MTU_SIZE);

    /* Initialize Ethernet driver hooks. */
    device->initialize = initialize;
    device->interrupt = interrupt;

    /* If we do have an initialize callback for this device. */
    if (initialize != NULL)
    {
        /* We still need to initialize this device. */
        device->flags = ETH_FLAG_INIT;
    }

} /* ethernet_regsiter */

/*
 * ethernet_lock
 * @fd: File descriptor for a ethernet device.
 * This function will get the lock for a given ethernet device.
 */
static int32_t ethernet_lock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    ETH_DEVICE *device = (ETH_DEVICE *)fd;

    /* Obtain data lock for this ethernet device. */
    return semaphore_obtain(&device->lock, MAX_WAIT);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Lock scheduler. */
    scheduler_lock();

    /* Return success. */
    return (SUCCESS);
#endif
} /* ethernet_lock */

/*
 * ethernet_unlock
 * @fd: File descriptor for a ethernet device.
 * This function will release the lock for a given ethernet device.
 */
static void ethernet_unlock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    ETH_DEVICE *device = (ETH_DEVICE *)fd;

    /* Release data lock for this ethernet device. */
    semaphore_release(&device->lock);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Enable scheduling. */
    scheduler_unlock();
#endif
} /* ethernet_unlock */

/*
 * ethernet_process
 * @data: Ethernet device for which an event is needed to be processed.
 * This function will process an ethernet event, this responsible for
 * initializing a device and any other device specific event.
 */
static void ethernet_process(void *data)
{
    ETH_DEVICE *device = (ETH_DEVICE *)data;

    /* Acquire lock for this device. */
    OS_ASSERT(fd_get_lock((FD)device) != SUCCESS);

    /* Check if we have an interrupt to process. */
    if (device->flags & ETH_FLAG_INT)
    {
        /* Process interrupt for this device. */
        device->interrupt(device);

        /* We have processed interrupt for this device. */
        device->flags &= (uint8_t)(~ETH_FLAG_INT);
    }

    /* Check if we need to do device initialization. */
    if (device->flags & ETH_FLAG_INIT)
    {
        /* Initialize this device. */
        device->initialize(device);

        /* We have initialized this device. */
        device->flags &= (uint8_t)(~ETH_FLAG_INIT);
    }

    /* Just clear the data available flag as we don't have any other event to
     * process for this device. */
    fd_data_flushed((FD)device);

    /* Release lock for this device. */
    fd_release_lock((FD)device);

} /* ethernet_process */

/*
 * ethernet_interrupt
 * @data: Ethernet device for which an interrupt has happened.
 * This function will process an interrupt event for given ethernet device.
 */
void ethernet_interrupt(ETH_DEVICE *device)
{
    /* Obtain lock for this device. */
    OS_ASSERT(fd_get_lock((FD)device) != SUCCESS);

    /* Set flag to tell that we have an interrupt to process. */
    device->flags |= ETH_FLAG_INT;

    /* Set flag that we have some data available on this device. */
    fd_data_available((FD)device);

    /* Release lock for this file descriptor. */
    fd_release_lock((FD)device);

} /* ethernet_interrupt */

/*
 * ethernet_buffer_receive
 * @buffer: A net buffer needed to be added in the receive list.
 * This function will process an ethernet frame and pass it to the networking
 * stack.
 */
int32_t ethernet_buffer_receive(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint16_t proto;

    /* Pull and discard the source and destination addresses. */
    OS_ASSERT(fs_buffer_pull(buffer, NULL, (ETH_ADDR_LEN * 2), 0) != SUCCESS);

    /* Pull the protocol. */
    OS_ASSERT(fs_buffer_pull(buffer, &proto, ETH_PROTO_LEN, FS_BUFFER_PACKED) != SUCCESS);

    /* Process the protocol. */
    switch(proto)
    {
    /* This is an IPv4 frame. */
    case ETH_PROTO_IP:

        /* Receive an IPv4 packet. */
        net_device_buffer_receive(buffer, NET_PROTO_IPV4);

        /* Buffer is now consumed. */
        status = NET_BUFFER_CONSUMED;

        break;
    }

    /* Return status to the caller. */
    return (status);

} /* ethernet_buffer_receive */

/*
 * ethernet_buffer_transmit
 * @buffer: A net buffer needed to be be sent.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if buffer was successfully
 *  transmitted, NET_BUFFER_CONSUMED will be returned if buffer is passed to
 *  the device and will be freed from there.
 * This function will transmit an ethernet frame.
 */
static int32_t ethernet_buffer_transmit(FS_BUFFER *buffer, uint8_t flags)
{
    int32_t status = SUCCESS;

    /* TODO */
    UNUSED_PARAM(buffer);
    UNUSED_PARAM(flags);

    /* Return status to the caller. */
    return (status);

} /* ethernet_buffer_transmit */

#endif /* CONFIG_ETHERNET */
