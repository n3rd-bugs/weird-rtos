/*
 * ethernet.c
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
#include <header.h>

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


uint8_t eth_dst_mac[ETH_ADDR_LEN];

/*
 * ethernet_regsiter
 * @device: Ethernet device instance needed to be registered.
 * @initialize: Ethernet device initialization function.
 * @transmit: Function that will be called to transmit a packet.
 * @interrupt: Ethernet interrupt callback.
 * This function will register an ethernet device.
 */
void ethernet_regsiter(ETH_DEVICE *device, ETH_INIT *initialize, ETH_TRANSMIT *transmit, ETH_INTERRUPT *interrupt)
{
    FD fd = (FD)&device->fs;

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
    fs_condition_init(fd);

    /* Register this file descriptor with file system. */
    fs_register(fd);

    /* This will block on read, and all data that will be given to write must
     * be flushed, also set the data available flag to start the device
     * initialization, this is a buffered file descriptor. */
    device->fs.flags = (FS_BLOCK | FS_FLUSH_WRITE | ((initialize != NULL) ? FS_DATA_AVAILABLE : 0) | FS_BUFFERED);

    /* Register a networking device. */
    net_register_fd(&device->net_device, fd, &ethernet_buffer_transmit, &ethernet_process);

    /* Set MTU for this device. */
    net_device_set_mtu(fd, ETH_MTU_SIZE);

    /* Initialize Ethernet driver hooks. */
    device->initialize = initialize;
    device->transmit = transmit;
    device->interrupt = interrupt;

    /* If we do have an initialize callback for this device. */
    if (initialize != NULL)
    {
        /* We still need to initialize this device. */
        device->flags = ETH_FLAG_INIT;
    }

} /* ethernet_regsiter */

/*
 * ethernet_random_mac
 * @device: Ethernet device instance for which random MAC address is needed to
 *  be generated.
 * @return: Returns the start of random MAC address generated.
 * This function will generate a random MAC address for this ethernet device.
 */
uint8_t *ethernet_random_mac(ETH_DEVICE *device)
{
    uint8_t i;

    /* Initialize a random MAC address. */
    for (i = 0; i < ETH_ADDR_LEN; i++)
    {
        device->mac[i] = (uint8_t)(current_system_tick64() & 0xFF);
    }

    /* Set the OUI bit and reset the multicast bit. */
    device->mac[0] |= ETH_MAC_OUI;
    device->mac[0] &= ((uint8_t)~(ETH_MAC_MULTICAST));

    /* Return the generated MAC address. */
    return (device->mac);

} /* ethernet_random_mac */

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
    FD fd = (FD)data;
    FS_BUFFER *buffer;
    int32_t status = SUCCESS;

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

    /* Check if we have to transmit a buffer. */
    if (device->flags & ETH_FLAG_TX)
    {
        /* While we have a buffer to transmit. */
        do
        {
            /* Get a buffer we need to transmit. */
            buffer = fs_buffer_get(fd, FS_BUFFER_TX, 0);

            /* If we have a buffer to transmit. */
            if (buffer != NULL)
            {
                /* Transmit this buffer. */
                status = device->transmit(device, buffer);

                if (status == SUCCESS)
                {
                    /* Free this buffer. */
                    fs_buffer_add(fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
                }
                else
                {
                    /* Put back this buffer on the transmission list. */
                    fs_buffer_add(fd, buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);
                }
            }

        } while ((status == SUCCESS) && (buffer != NULL));

        /* Clear the transmit flag. */
        device->flags &= (uint8_t)(~ETH_FLAG_TX);
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

    /* Pull and discard the destination addresses. */
    OS_ASSERT(fs_buffer_pull(buffer, NULL, (ETH_ADDR_LEN), 0) != SUCCESS);

    /* Pull and save the source address. */
    OS_ASSERT(fs_buffer_pull(buffer, eth_dst_mac, (ETH_ADDR_LEN), 0) != SUCCESS);

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
 * @return: NET_NOT_SUPPORTED will be returned if specified protocol is not
 *  supported, NET_BUFFER_CONSUMED will be returned as this packet will be
 *  pushed in the ethernet transmission queue.
 * This function will transmit an ethernet frame.
 */
static int32_t ethernet_buffer_transmit(FS_BUFFER *buffer, uint8_t flags)
{
    ETH_DEVICE *device = (ETH_DEVICE *)buffer->fd;
    int32_t status;
    HDR_GEN_MACHINE hdr_machine;
    uint16_t proto = 0;
    uint8_t net_proto;
    HEADER eth_hdr[] =
    {
        {eth_dst_mac,       ETH_ADDR_LEN,   (flags) },                      /* Destination address. */
        {device->mac,       ETH_ADDR_LEN,   (flags) },                      /* Source address. */
        {(uint8_t *)&proto, 2,              (FS_BUFFER_PACKED | flags) },   /* Ethernet type. */
    };

    /* Skim the protocol from the buffer. */
    OS_ASSERT(fs_buffer_pull(buffer, &net_proto, sizeof(uint8_t), 0) != SUCCESS);

    switch (net_proto)
    {
    case NET_PROTO_IPV4:
        proto = ETH_PROTO_IP;
        break;
    default:
        status = NET_NOT_SUPPORTED;
        break;
    }

    if (status == SUCCESS)
    {
        /* Generate an ethernet header on this buffer. */

        /* Initialize header generator machine. */
        header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

        /* Push the ethernet header on the buffer. */
        status = header_generate(&hdr_machine, eth_hdr, sizeof(eth_hdr)/sizeof(HEADER), buffer);
    }

    if (status == SUCCESS)
    {
        /* Add this buffer in the device transmission list. */
        fs_buffer_add(buffer->fd, buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);

        /* Set flag to tell that we have a packet to transmit. */
        device->flags |= ETH_FLAG_TX;

        /* Set flag that we have some data available on this device. */
        fd_data_available(buffer->fd);

        /* This buffer is now in device transmission queue. */
        status = NET_BUFFER_CONSUMED;
    }

    /* Return status to the caller. */
    return (status);

} /* ethernet_buffer_transmit */

#endif /* CONFIG_ETHERNET */
