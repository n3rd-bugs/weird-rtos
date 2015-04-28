/*
 * net_udp.c
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
#include <net.h>

#ifdef NET_UDP
#include <net_udp.h>
#include <net_csum.h>
#include <header.h>
#include <string.h>
#include <sll.h>

/* Global UDP data. */
UDP_DATA udp_data;

/* Internal function prototypes. */
static uint8_t net_port_seach(void *, void *);
static int32_t udp_read(void *, char *, int32_t);
static int32_t udp_write(void *, char *, int32_t);

/*
 * udp_initialize
 * This function will initialize UDP stack.
 */
void udp_initialize()
{
    /* Clear the global UDP data. */
    memset(&udp_data, 0, sizeof(UDP_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create the semaphore to protect global UDP data. */
    semaphore_create(&udp_data.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

} /* udp_initialize */

/*
 * udp_register
 * @port: UDP port needed to be registered.
 * @name: If not null will hold a name for this UDP port.
 * @socket_address: Socket address for this UDP port.
 * This function will register a UDP port with UDP stack.
 */
void udp_register(UDP_PORT *port, char *name, SOCKET_ADDRESS *socket_address)
{
    /* Clear the UDP port data. */
    memset(port, 0, sizeof(UDP_PORT));

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data semaphore. */
    OS_ASSERT(semaphore_obtain(&udp_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Copy the socket address. */
    memcpy(&port->socket_address, socket_address, sizeof(SOCKET_ADDRESS));

    /* Add this port in the global port list. */
    sll_append(&udp_data.port_list, port, OFFSETOF(UDP_PORT, next));

    /* Register this UDP port as a console. */
    port->console.fs.name = name;
    port->console.fs.read = &udp_read;
    port->console.fs.write = &udp_write;
    port->console.fs.flags |= (FS_BLOCK | FS_SPACE_AVAILABLE);
    console_register(&port->console);

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&udp_data.lock);
#endif

} /* udp_register */

/*
 * udp_unregister
 * @port: UDP port needed to be unregistered.
 * This function will unregister a UDP port.
 */
void udp_unregister(UDP_PORT *port)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data semaphore. */
    OS_ASSERT(semaphore_obtain(&udp_data.lock, MAX_WAIT) != SUCCESS);

    /* Obtain semaphore for this UDP port. */
    OS_ASSERT(fd_get_lock((FD)port));

    /* Unregister this UDP port from console. */
    console_unregister(&port->console);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Remove this port from the global port list. */
    OS_ASSERT(sll_remove(&udp_data.port_list, port, OFFSETOF(UDP_PORT, next)) != port);

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&udp_data.lock);
#endif

} /* udp_unregister */

/*
 * udp_unregister
 * @node: A UDP port in the list.
 * @param: UDP port search parameter.
 * @return: Will return true if we matched an exact UDP port for given UDP
 *  parameter.
 * This function is a search callback to find a specific UDP port.
 */
static uint8_t net_port_seach(void *node, void *param)
{
    UDP_PORT_PARAM *udp_param = (UDP_PORT_PARAM *)param;
    UDP_PORT *port = (UDP_PORT *)node;
    uint8_t match = FALSE;

    /* Match two UDP socket addresses. */
    match = net_socket_address_match(&port->socket_address, &udp_param->socket_address);

    /* If we did not fail completely. */
    if (match != FALSE)
    {
        /* Save this port. */
        udp_param->port = port;
    }

    /* If this was a partial match. */
    if (match == PARTIAL)
    {
        /* SLL don't understand partial yet. */
        match = FALSE;
    }

    /* Return if this is required port. */
    return (match);

} /* net_port_seach */

/*
 * net_process_udp
 * @buffer: File system buffer needed to be processed.
 * @ihl: IPv4 header length.
 * @iface_addr: Interface IP address on which this packet was received.
 * @src_ip: Source address from the IP header.
 * @dst_ip: Destination address from the IP header.
 * @return: A success status will be returned if UDP packet was successfully
 *  processed.
 *  NET_DST_PRT_UNREACHABLE will be returned if destination port is
 *  unreachable.
 *  NET_INVALID_HDR will be returned if invalid header was parsed.
 *  NET_INVALID_CSUM will be returned if an invalid checksum was received.
 *  NET_NO_BUFFERS if we ran out of buffers, NET_BUFFER_CONSUMED will be returned
 *  if buffer was consumed and caller don't need to free it.
 * This function will process an incoming UDP header.
 */
int32_t net_process_udp(FS_BUFFER *buffer, uint32_t ihl, uint32_t iface_addr, uint32_t src_ip, uint32_t dst_ip)
{
    int32_t status = SUCCESS;
    uint16_t length;
    UDP_PORT *udp_port;
    UDP_PORT_PARAM port_param;
#ifdef UDP_CSUM
    uint16_t csum_hdr;
    uint32_t csum;
    FS_BUFFER *csum_buffer;
    HDR_GEN_MACHINE hdr_machine;
    HEADER pseudo_hdr[] =
    {
        {(uint8_t *)&src_ip,            4, FS_BUFFER_PACKED},   /* Source address. */
        {(uint8_t *)&dst_ip,            4, FS_BUFFER_PACKED},   /* Destination address. */
        {(uint8_t []){0},               1, 0},                  /* Zero. */
        {(uint8_t []){IP_PROTO_UDP},    1, 0},                  /* Protocol. */
        {(uint8_t *)&length,            2, FS_BUFFER_PACKED},   /* UDP length. */
    };
#endif

    /* Pull the length of UDP datagram. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &length, 2, (ihl + UDP_HRD_LEN_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

#ifdef UDP_CSUM
    /* Pull the checksum for UDP datagram. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &csum_hdr, 2, (ihl + UDP_HRD_CSUM_OFFSET), (FS_BUFFER_INPLACE)) != SUCCESS);

    /* If we can verify the checksum for UDP header. */
    if (csum_hdr != 0)
    {
        /* Allocate a buffer and initialize a pseudo header. */
        csum_buffer = fs_buffer_get(buffer->fd, FS_BUFFER_LIST, 0);

        /* If we have to buffer to compute checksum. */
        if (csum_buffer != NULL)
        {
            /* Initialize header generator machine. */
            header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

            /* Push the ICMP header on the buffer. */
            status = header_generate(&hdr_machine, pseudo_hdr, sizeof(pseudo_hdr)/sizeof(HEADER), csum_buffer);

            /* If pseudo header was successfully generated. */
            if (status == SUCCESS)
            {
                /* Calculate checksum for the pseudo header. */
                csum = net_csum_calculate(csum_buffer, -1, 0);

                /* Calculate and add the checksum for UDP datagram. */
                NET_CSUM_ADD(csum, net_csum_calculate(buffer, -1, ihl));

                /* If we don't have anticipated checksum. */
                if (csum != 0)
                {
                    /* Return an error to the caller. */
                    status = NET_INVALID_CSUM;
                }
            }

            /* Free the pseudo header buffer. */
            fs_buffer_add(buffer->fd, csum_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }
        else
        {
            /* There are no buffers. */
            status = NET_NO_BUFFERS;
        }
    }
#endif

    if (status == SUCCESS)
    {
        /* Peek the UDP ports fields.. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &port_param.socket_address.foreign_port, 2, (ihl + UDP_HRD_SRC_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
        OS_ASSERT(fs_buffer_pull_offset(buffer, &port_param.socket_address.local_port, 2, (ihl + UDP_HRD_DST_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* If UDP header length value is correct. */
        if (buffer->total_length == (ihl + length))
        {
#ifdef CONFIG_SEMAPHORE
            /* Obtain the global data semaphore. */
            OS_ASSERT(semaphore_obtain(&udp_data.lock, MAX_WAIT) != SUCCESS);
#else
            /* Lock the scheduler. */
            scheduler_lock();
#endif
            /* Initialize search parameter for this UDP datagram. */
            port_param.socket_address.local_ip = dst_ip;
            port_param.socket_address.foreign_ip = src_ip;
            port_param.port = NULL;

            /* Search for a UDP port that can be used to receive this packet. */
            sll_search(&udp_data.port_list, NULL, &net_port_seach, &port_param, OFFSETOF(UDP_PORT, next));

            /* Save the resolved port. */
            udp_port = port_param.port;

#ifndef CONFIG_SEMAPHORE
            /* Enable scheduling. */
            scheduler_unlock();
#else
            /* Release the global semaphore. */
            semaphore_release(&udp_data.lock);
#endif
        }

        else
        {
            /* Invalid UDP header. */
            status = NET_INVALID_HDR;
        }

        if (status == SUCCESS)
        {
            /* If we have a valid UDP port for this datagram. */
            if (udp_port != NULL)
            {
                /* Obtain lock for this UDP port. */
                OS_ASSERT(fd_get_lock((FD)udp_port));

                /* Add this buffer in the buffer list for UDP port. */
                sll_append(&udp_port->buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

                /* Set an event to tell that new data is now available. */
                fd_data_available((FD)udp_port);

                /* Release lock for this UDP port. */
                fd_release_lock((FD)udp_port);

                /* This buffer is now consumed by the UDP port. */
                status = NET_BUFFER_CONSUMED;
            }

            /* If this datagram was intended for us. */
            else if (dst_ip == iface_addr)
            {
                /* Destination port is unreachable. */
                status = NET_DST_PRT_UNREACHABLE;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_udp */

/*
 * udp_read
 * @fd: File descriptor.
 * @buffer: Buffer in which data will be read.
 * @size: Size of buffer.
 * @return: Number of bytes read.
 * This function will read data from a UDP port.
 */
static int32_t udp_read(void *fd, char *buffer, int32_t size)
{
    UDP_PORT *port = (UDP_PORT *)fd;
    FS_BUFFER *fs_buffer;
    int32_t ret_size = 0;
    uint8_t ihl;

    /* Get a buffer from the UDP port. */
    fs_buffer = sll_pop(&port->buffer_list, OFFSETOF(FS_BUFFER, next));

    /* If we do have a buffer. */
    if (fs_buffer != NULL)
    {
        /* Get lock for the buffer file descriptor. */
        OS_ASSERT(fd_get_lock(fs_buffer->fd));

        /* Peek the version and IHL. */
        OS_ASSERT(fs_buffer_pull_offset(fs_buffer, &ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);
        ihl = (uint8_t)((ihl & IPV4_HDR_IHL_MASK) << 2);

        /* Save the IP addresses for this UDP datagram. */
        OS_ASSERT(fs_buffer_pull_offset(fs_buffer, &port->last_datagram_address.foreign_ip, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
        OS_ASSERT(fs_buffer_pull_offset(fs_buffer, &port->last_datagram_address.local_ip, 4, IPV4_HDR_DST_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Save the port addresses for this UDP datagram. */
        OS_ASSERT(fs_buffer_pull_offset(fs_buffer, &port->last_datagram_address.foreign_port, 2, (uint32_t)(ihl + UDP_HRD_SRC_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
        OS_ASSERT(fs_buffer_pull_offset(fs_buffer, &port->last_datagram_address.local_port, 2, (uint32_t)(ihl + UDP_HRD_DST_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Pull the IP and UDP headers from the packet. */
        OS_ASSERT(fs_buffer_pull_offset(fs_buffer, NULL, (uint32_t)(ihl + UDP_HRD_LENGTH), 0, 0) != SUCCESS);

        /* If we need to copy more data. */
        if (size > (int32_t)fs_buffer->total_length)
        {
            /* For now copy only required data. */
            ret_size = (int32_t)fs_buffer->total_length;
        }
        else
        {
            /* Copy only the number of bytes that can be copied. */
            ret_size = size;
        }

        /* Pull data from the buffer into the provided buffer. */
        OS_ASSERT(fs_buffer_pull(fs_buffer, buffer, (uint32_t)ret_size, 0) != SUCCESS);

        /* Return this buffer to it's owner. */
        fs_buffer_add(fs_buffer->fd, fs_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

        /* Release lock for the buffer file descriptor. */
        fd_release_lock(fs_buffer->fd);
    }

    /* If there is no more data to read. */
    if (port->buffer_list.head == NULL)
    {
        /* Tell file system that there is no more data to read from us. */
        fd_data_flushed(fd);
    }

    /* Return number of bytes. */
    return (ret_size);

} /* udp_read */

/*
 * udp_write
 * @fd: File descriptor.
 * @buffer: Buffer from which data is needed to be copied.
 * @size: Number of bytes to copy from the buffer.
 * @return: Number of bytes sent.
 * This function will write data on given UDP port.
 */
static int32_t udp_write(void *fd, char *buffer, int32_t size)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(buffer);

    /* Return number of bytes. */
    return (size);

} /* udp_write */

#endif /* NET_UDP */
#endif /* CONFIG_NET */
