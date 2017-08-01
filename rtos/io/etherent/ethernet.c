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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>

#ifdef CONFIG_ETHERNET
#include <net_route.h>
#include <string.h>
#include <ethernet.h>
#include <net_buffer.h>
#include <header.h>
#include <idle.h>
#include <ethernet_target.h>

/* Internal function prototypes. */
static int32_t ethernet_buffer_transmit(FS_BUFFER *, uint8_t);
static void ethernet_process(void *, int32_t);
static int32_t ethernet_lock(void *, uint32_t);
static void ethernet_unlock(void *);

/*
 * ethernet_init
 * This function will initialize ethernet devices.
 */
void ethernet_init(void)
{
    /* Do target initialization for ethernet. */
    ETHERENET_TGT_INIT();

} /* ethernet_init */

/*
 * ethernet_regsiter
 * @device: Ethernet device instance needed to be registered.
 * @initialize: Ethernet device initialization function.
 * @transmit: Function that will be called to transmit a packet.
 * @interrupt: Ethernet interrupt callback.
 * @wdt: Watch dog event callback.
 * @int_poll: Function to be called to poll the status of interrupt.
 * This function will register an ethernet device.
 */
void ethernet_regsiter(ETH_DEVICE *device, ETH_INIT *initialize, ETH_TRANSMIT *transmit, ETH_INTERRUPT *interrupt, ETH_WDT *wdt, ETH_INT_POLL *int_poll)
{
    FD fd = (FD)&device->fs;

    /* Initialize file system data. */
    device->fs.get_lock = &ethernet_lock;
    device->fs.release_lock = &ethernet_unlock;
    device->fs.timeout = MAX_WAIT;

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect this device. */
    memset(&device->lock, 0, sizeof(SEMAPHORE));
    semaphore_create(&device->lock, 1);
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

    /* Disable timeout. */
    device->net_device.suspend.timeout_enabled = FALSE;

    /* Set MTU for this device. */
    net_device_set_mtu(fd, ETH_MTU_SIZE);

    /* Initialize Ethernet driver hooks. */
    device->initialize = initialize;
    device->transmit = transmit;
    device->interrupt = interrupt;
    device->wdt = wdt;
    device->int_poll = int_poll;

    /* If we do have an initialize callback for this device. */
    if (initialize != NULL)
    {
        /* We still need to initialize this device. */
        device->flags = ETH_FLAG_INIT;
    }

    /* If we need to poll the interrupt line. */
    if (int_poll != NULL)
    {
        /* Register interrupt polling work for this device. */
        ASSERT(idle_add_work(int_poll, device) != SUCCESS);
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
        device->mac[i] = (uint8_t)(current_hardware_tick() & 0xFF);
    }

    /* Set the OUI bit and reset the multicast bit. */
    device->mac[0] |= ETH_MAC_OUI;
    device->mac[0] &= ((uint8_t)~(ETH_MAC_MULTICAST));

    /* Return the generated MAC address. */
    return (device->mac);

} /* ethernet_random_mac */

/*
 * ethernet_get_mac_address
 * @fd: File descriptor associated with an ethernet device.
 * @return: Returns the start of random MAC address generated.
 * This function will return MAC address associated with an ethernet device.
 */
uint8_t *ethernet_get_mac_address(FD fd)
{
    ETH_DEVICE *device = (ETH_DEVICE *)fd;

    /* Return MAC address associated with given device. */
    return (device->mac);

} /* ethernet_get_mac_address */

/*
 * ethernet_wdt_enable
 * @device: Ethernet device for which watch dog timer is needed to be enabled.
 * @ticks: Ticks after which the watch dog event is needed to be triggered.
 * This function will enable watch dog timer for an ethernet device. Caller must
 * have lock for the required device and this function.
 */
void ethernet_wdt_enable(ETH_DEVICE *device, uint32_t ticks)
{
    /* Enable watch dog timer for this device. */
    device->net_device.suspend.timeout = current_system_tick() + ticks;

    /* Enable WDT timer. */
    device->net_device.suspend.timeout_enabled = TRUE;

} /* ethernet_wdt_enable */

/*
 * ethernet_wdt_disable
 * @device: Ethernet device for which watch dog timer is needed to be disabled.
 * This function will reset the watch dog timer for an ethernet device. Caller
 * must have lock for the required device and this function.
 */
void ethernet_wdt_disable(ETH_DEVICE *device)
{
    /* Disable the watch dog timer. */
    device->net_device.suspend.timeout_enabled = FALSE;

} /* ethernet_wdt_disable */

/*
 * ethernet_lock
 * @fd: File descriptor for a ethernet device.
 * @timeout: Number of ticks we need to wait for the lock.
 * This function will get the lock for a given ethernet device.
 */
static int32_t ethernet_lock(void *fd, uint32_t timeout)
{
    ETH_DEVICE *device = (ETH_DEVICE *)fd;

#ifdef CONFIG_SEMAPHORE
    /* Obtain data lock for this ethernet device. */
    return semaphore_obtain(&device->lock, timeout);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(timeout);

    /* If this is ISR accessible. */
    if ((device->interrupt != NULL) && (device->int_poll == NULL))
    {
        /* Save interrupt status for this device. */
        device->int_status = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();
    }

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
    ETH_DEVICE *device = (ETH_DEVICE *)fd;

#ifdef CONFIG_SEMAPHORE
    /* Release data lock for this ethernet device. */
    semaphore_release(&device->lock);
#else

    /* If this is ISR accessible. */
    if ((device->interrupt != NULL) && (device->int_poll == NULL))
    {
        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(device->int_status);
    }

    /* Enable scheduling. */
    scheduler_unlock();
#endif
} /* ethernet_unlock */

/*
 * ethernet_process
 * @data: Ethernet device for which an event is needed to be processed.
 * @resume_status: Resumption status.
 * This function will process an ethernet event, this responsible for
 * initializing a device and any other device specific event.
 */
static void ethernet_process(void *data, int32_t resume_status)
{
    ETH_DEVICE *device = (ETH_DEVICE *)data;
    FD fd = (FD)data;
    FS_BUFFER *buffer;
    int32_t status = SUCCESS;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(resume_status);

    /* Acquire lock for this device. */
    ASSERT(fd_get_lock((FD)device) != SUCCESS);

    /* If watch dog interrupt was triggered for this device. */
    if ((device->net_device.suspend.timeout_enabled == TRUE) && (INT32CMP(device->net_device.suspend.timeout, current_system_tick()) <= 0))
    {
        /* Disable watch dog timer. */
        ethernet_wdt_disable(device);

        /* Trigger watch dog event. */
        device->wdt(device);
    }

    /* Check if we need to do device initialization. */
    if (device->flags & ETH_FLAG_INIT)
    {
        /* Initialize this device. */
        device->initialize(device);

        /* We have initialized this device. */
        device->flags &= (uint8_t)(~ETH_FLAG_INIT);
    }

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
                    /* Pull ethernet header from the buffer. */
                    ASSERT(fs_buffer_pull(buffer, NULL, ETH_HRD_SIZE, 0) != SUCCESS);

                    /* Free this buffer. */
                    fs_buffer_add(fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
                }
                else
                {
                    /* Put back this buffer on the head of transmission list. */
                    fs_buffer_add(fd, buffer, FS_BUFFER_TX, (FS_BUFFER_ACTIVE | FS_BUFFER_HEAD));
                }
            }

        } while ((status == SUCCESS) && (buffer != NULL));

        /* Clear the transmit flag. */
        device->flags &= (uint8_t)(~ETH_FLAG_TX);
    }

    /* If we don't have any more event to process. */
    if ((device->flags & (ETH_FLAG_INIT | ETH_FLAG_INT |ETH_FLAG_TX)) == 0)
    {
        /* Just clear the data available flag as we don't have any other event to
         * process for this device. */
        fd_data_flushed((FD)device);
    }

    /* Release lock for this device. */
    fd_release_lock((FD)device);

} /* ethernet_process */

/*
 * ethernet_interrupt
 * @data: Ethernet device for which an interrupt has happened.
 * This function will process an interrupt event for given ethernet device.
 * This is to be called from the interrupt handler for a particular device.
 */
int32_t ethernet_interrupt(ETH_DEVICE *device)
{
    /* If we are handling the interrupt for this device, there must be no
     * holder of the lock so no need to acquire it. */

    /* Set flag to tell that we have an interrupt to process. */
    device->flags |= ETH_FLAG_INT;

    /* Set flag that we have some data available on this device. */
    fd_data_available((FD)device);

    /* Always return success. */
    return (SUCCESS);

} /* ethernet_interrupt */

/*
 * ethernet_buffer_receive
 * @buffer: A net buffer needed to be added in the receive list.
 * @return: NET_NOT_SUPPORTED will be returned if the given ethernet protocol
 *  is not supported, NET_BUFFER_CONSUMED will be returned if buffer was passed
 *  to the networking stack.
 * This function will process an ethernet frame and pass it to the networking
 * stack.
 */
int32_t ethernet_buffer_receive(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint32_t flags = 0;
    uint16_t proto;
    uint8_t net_proto;
    uint8_t dst_mac[ETH_ADDR_LEN];

    /* Pull the destination and source addresses. */
    ASSERT(fs_buffer_pull(buffer, dst_mac, (ETH_ADDR_LEN), 0) != SUCCESS);
    ASSERT(fs_buffer_pull(buffer, NULL, (ETH_ADDR_LEN), 0) != SUCCESS);

    /* If this was a broadcast frame. */
    if (memcmp(dst_mac, ETH_BCAST_ADDR, ETH_ADDR_LEN) == 0)
    {
        /* Set the broadcast flag. */
        flags |= ETH_FRAME_BCAST;
    }

    /* Pull the protocol. */
    ASSERT(fs_buffer_pull(buffer, &proto, ETH_PROTO_LEN, FS_BUFFER_PACKED) != SUCCESS);

    /* Process the protocol. */
    switch(proto)
    {
#ifdef NET_IPV4
    /* This is an IPv4 frame. */
    case ETH_PROTO_IP:

        /* Pick IPv4 protocol. */
        net_proto = NET_PROTO_IPV4;

        break;
#endif
#ifdef NET_ARP
    /* This is an ARP frame. */
    case ETH_PROTO_ARP:

        /* Pick ARP protocol. */
        net_proto = NET_PROTO_ARP;

        break;
#endif
    /* An unsupported protocol. */
    default:

        /* Return an error. */
        status = NET_NOT_SUPPORTED;

        break;
    }

    /* If we have a supported protocol. */
    if (status == SUCCESS)
    {
        /* Pass this frame to the networking stack. */
        status = net_device_buffer_receive(buffer, net_proto, flags);

        /* If buffer was successfully passed to the networking stack. */
        if (status == SUCCESS)
        {
            /* Buffer is now consumed. */
            status = NET_BUFFER_CONSUMED;
        }
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
    int32_t status = SUCCESS;
    HDR_GEN_MACHINE hdr_machine;
    uint32_t iface_addr, subnet, orig_len = buffer->total_length;
    uint16_t proto = 0;
#ifdef NET_IPV4
    uint32_t dst_ip;
#endif
    uint8_t net_proto, dst_mac[ETH_ADDR_LEN];
    HEADER eth_hdr[] =
    {
        {dst_mac,           ETH_ADDR_LEN,   (flags) },                      /* Destination address. */
        {device->mac,       ETH_ADDR_LEN,   (flags) },                      /* Source address. */
        {(uint8_t *)&proto, 2,              (FS_BUFFER_PACKED | flags) },   /* Ethernet type. */
    };

    /* Skim the protocol from the buffer. */
    ASSERT(fs_buffer_pull(buffer, &net_proto, sizeof(uint8_t), 0) != SUCCESS);

    switch (net_proto)
    {
#ifdef NET_IPV4
    /* If an IPv4 packet is needed to be transmitted. */
    case NET_PROTO_IPV4:

        /* Pull the intended source and destination IP address. */
        ASSERT(fs_buffer_pull_offset(buffer, &iface_addr, IPV4_ADDR_LEN, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)) != SUCCESS);
        ASSERT(fs_buffer_pull_offset(buffer, &dst_ip, IPV4_ADDR_LEN, IPV4_HDR_DST_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)) != SUCCESS);

        /* Get the subnet mask for the address. */
        ipv4_get_device_address(buffer->fd, &iface_addr, &subnet);

        /* If destination is a broadcast address. */
        if ((dst_ip == IPV4_ADDR_BCAST_NET(iface_addr, subnet)) || (dst_ip == IPV4_ADDR_BCAST))
        {
            /* Use the broadcast ethernet address. */
            memcpy(dst_mac, ETH_BCAST_ADDR, ETH_ADDR_LEN);
        }
        else
        {
            /* Find a route for this destination. */
            status = route_get(&buffer->fd, dst_ip, NULL, &dst_ip, NULL);

            if (status == SUCCESS)
            {
                /* Try to resolve destination MAC address for resolved route. */
                status = arp_resolve(buffer, dst_ip, dst_mac);
            }
        }

        /* Use the ethernet IPv4 type. */
        proto = ETH_PROTO_IP;

        break;
#endif

#ifdef NET_ARP
    /* If an ARP packet is needed to be transmitted. */
    case NET_PROTO_ARP:

        /* Pull the destination HW address from the ARP packet. */
        ASSERT(fs_buffer_pull_offset(buffer, dst_mac, ETH_ADDR_LEN, (ARP_HDR_PRE_LEN + ARP_HDR_TGT_HW_OFFSET), (FS_BUFFER_INPLACE)) != SUCCESS);

        /* If target address is not known. */
        if (memcmp(dst_mac, ETH_UNSPEC_ADDR, ETH_ADDR_LEN) == 0)
        {
            /* Use broadcast address. */
            memcpy(dst_mac, ETH_BCAST_ADDR, ETH_ADDR_LEN);
        }

        /* Use the ethernet ARP type. */
        proto = ETH_PROTO_ARP;

        break;
#endif

    /* Unknown protocol. */
    default:
        /* This protocol is not supported. */
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

    /* If we added partial header on the buffer. */
    else if (buffer->total_length > orig_len)
    {
        /* Pull any excess data from the buffer. */
        ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - orig_len), 0) != SUCCESS);
    }

    /* Return status to the caller. */
    return (status);

} /* ethernet_buffer_transmit */

#endif /* CONFIG_ETHERNET */
