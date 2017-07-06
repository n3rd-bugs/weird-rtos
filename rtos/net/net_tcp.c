/*
 * net_tcp.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_TCP
#include <string.h>
#include <sll.h>
#include <header.h>
#include <net_tcp.h>
#include <net_csum.h>

/* Global TCP data. */
static TCP_DATA tcp_data;
volatile uint32_t tcp_iss;

/* Internal function prototypes. */
static void tcp_port_initialize(TCP_PORT *);
static uint8_t tcp_port_search(void *, void *);
static void tcp_resume_socket(TCP_PORT *, uint8_t);
static int32_t tcp_port_wait(TCP_PORT *, uint8_t);
static int32_t tcp_process_options(FS_BUFFER *, TCP_PORT *, uint32_t, uint16_t);
static int32_t tcp_add_option(FS_BUFFER *, uint8_t, uint8_t, void *, uint8_t);
static int32_t tcp_add_options(FS_BUFFER *, TCP_PORT *, uint8_t, uint8_t *, uint8_t);
static void tcp_timer_register(TCP_PORT *);
static void tcp_timer_unregister(TCP_PORT *);
static void tcp_timeout_update(TCP_PORT *);
static void tcp_fast_rtx(TCP_PORT *, uint32_t);
static void tcp_timeout_callback(void *, int32_t);
static int32_t tcp_send_segment(TCP_PORT *, SOCKET_ADDRESS *, uint32_t, uint32_t, uint16_t, uint16_t, uint8_t *, int32_t, uint8_t, uint8_t);
static uint8_t tcp_check_sequence(uint32_t, uint32_t, uint32_t, uint32_t);
static void tcp_process_finbit(TCP_PORT *, uint32_t);
static uint8_t tcp_oo_buffer_process(void *, void *);
static int32_t tcp_rx_buffer_merge(TCP_PORT *, FS_BUFFER *, uint16_t, uint32_t);
static void tcp_buffer_get_ihl_flags(FS_BUFFER *, uint8_t *, uint16_t *);
static int32_t tcp_read_buffer(void *, uint8_t *, int32_t);
static int32_t tcp_read_data(void *, uint8_t *, int32_t);
static int32_t tcp_write_buffer(void *, uint8_t *, int32_t);
static int32_t tcp_write_data(void *, uint8_t *, int32_t);
static TCP_RTX_DATA *tcp_get_rtx_free(TCP_PORT *);
static uint8_t tcp_rtx_return_buffer(void *, FS_BUFFER *);
static uint8_t tcp_rtx_process_ack(TCP_PORT *, uint32_t);
static void tcp_rtx_free_all(TCP_PORT *);

/*
 * tcp_initialize
 * This function will initialize TCP stack.
 */
void tcp_initialize()
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

#ifdef CONFIG_SEMAPHORE
    /* Create the semaphore to protect global TCP data. */
    semaphore_create(&tcp_data.lock, 1);
#endif

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_initialize */

/*
 * tcp_register
 * @port: TCP port needed to be registered.
 * @name: If not null will hold a name for this TCP port.
 * @socket_address: Socket address for this TCP port.
 * This function will register a TCP port with TCP stack.
 */
void tcp_register(TCP_PORT *port, char *name, SOCKET_ADDRESS *socket_address)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data semaphore. */
    ASSERT(semaphore_obtain(&tcp_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Copy the socket address. */
    memcpy(&port->socket_address, socket_address, sizeof(SOCKET_ADDRESS));

    /* Add this port in the global port list. */
    sll_append(&tcp_data.port_list, port, OFFSETOF(TCP_PORT, next));

    /* Register this TCP port as a console. */
    port->console.fs.name = name;
    port->console.fs.flags |= (FS_BLOCK);
    console_register(&port->console);

    /* If this is buffered descriptor. */
    if (port->console.fs.flags & FS_BUFFERED)
    {
        /* Set buffered APIs for this descriptor. */
        port->console.fs.read = &tcp_read_buffer;
        port->console.fs.write = &tcp_write_buffer;
    }
    else
    {
        /* Set non buffered APIs for this descriptor. */
        port->console.fs.read = &tcp_read_data;
        port->console.fs.write = &tcp_write_data;
    }

    /* Initialize TCP port. */
    tcp_port_initialize(port);

    /* Register retransmission timer for this TCP port. */
    tcp_timer_register(port);

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&tcp_data.lock);
#endif

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_register */

/*
 * tcp_unregister
 * @port: TCP port needed to be unregistered.
 * This function will unregister a TCP port.
 */
void tcp_unregister(TCP_PORT *port)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data semaphore. */
    ASSERT(semaphore_obtain(&tcp_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Remove this port from the global port list. */
    ASSERT(sll_remove(&tcp_data.port_list, port, OFFSETOF(TCP_PORT, next)) != port);

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&tcp_data.lock);
#endif

    /* Acquire lock for this port. */
    fd_get_lock(port);

    /* Free all the buffers in the TCP buffer list. */
    fs_buffer_add_buffer_list(port->buffer_list.head, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

    /* Unregister networking condition for this TCP port. */
    tcp_timer_unregister(port);

    /* Release lock for this port. */
    fd_release_lock(port);

    /* Unregister this TCP port from console. */
    console_unregister(&port->console);

    /* Clear the TCP port structure. */
    memset(port, 0, sizeof(TCP_PORT));

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_unregister */

/*
 * tcp_port_initialize
 * @port: TCP port needed to be initialized.
 * This function initializes TCP port variables.
 */
static void tcp_port_initialize(TCP_PORT *port)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Initialize TCP port configuration. */
    port->rcv_wnd = TCP_WND_SIZE;
    port->rcv_wnd_scale = port->snd_wnd_scale = 0;
    port->nacks = 0;
    port->event_timeout_enable = port->rtx_timeout_enable = FALSE;

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_port_initialize */

/*
 * tcp_port_search
 * @node: A TCP port in the list.
 * @param: TCP port search parameter.
 * @return: Will return true if we matched an exact TCP port for given TCP
 *  parameter.
 * This function is a search callback to find a specific TCP port.
 */
static uint8_t tcp_port_search(void *node, void *param)
{
    TCP_PORT_PARAM *tcp_param = (TCP_PORT_PARAM *)param;
    TCP_PORT *port = (TCP_PORT *)node;
    uint8_t match = FALSE;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Match two TCP socket addresses. */
    match = net_socket_address_match(&port->socket_address, &tcp_param->socket_address);

    /* If we did not fail completely. */
    if (match != FALSE)
    {
        /* Save this port. */
        tcp_param->port = port;
    }

    /* If this was a partial match. */
    if (match == PARTIAL)
    {
        /* SLL don't understand partial yet. */
        match = FALSE;
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

    /* Return if this is required port. */
    return (match);

} /* tcp_port_search */

/*
 * tcp_resume_socket
 * @port: Port for which any waiting tasks are needed to be resumed.
 * @flags: Resume condition flag.
 *  FS_BLOCK_READ: If we need to resume any tasks waiting for a state change or
 *  new data on this socket.
 *  FS_BLOCK_WRITE: If we need to resume tasks waiting for space in TCP window
 *  to send new data.
 * This function will resume any tasks waiting on a TCP socket.
 */
static void tcp_resume_socket(TCP_PORT *port, uint8_t flags)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* If we need to resume tasks waiting on read. */
    if (flags & FS_BLOCK_READ)
    {
        /* Set an event to tell that new data is now available. */
        fd_data_available((FD)port);
    }

    /* If we need to resume tasks waiting for space in TCP send window. */
    if (flags & FS_BLOCK_WRITE)
    {
        /* Set an event to tell that some space is now available. */
        fd_space_available((FD)port);
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_resume_socket */

/*
 * tcp_port_wait
 * @port: TCP port on which we need to wait.
 * @flags: Define if we need to wait for new data or space on this socket.
 * @return: Returns the resumption status as returned from the invoker.
 * This function will wait on a given TCP port, either for space or new data.
 * New data includes state change for this port.
 */
static int32_t tcp_port_wait(TCP_PORT *port, uint8_t flags)
{
    int32_t status;
    CONDITION *condition;
    SUSPEND suspend, *suspend_ptr = &suspend;
    FS_PARAM fs_param;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Get read condition for server TCP port. */
    fs_condition_get((FD)port, &condition, suspend_ptr, &fs_param, flags);

    /* Wait on this TCP socket. */
    status = suspend_condition(&condition, &suspend_ptr, NULL, TRUE);

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_port_wait */

/*
 * tcp_process_options
 * @buffer: Buffer from which TCP options are needed to be processed.
 * @port: TCP port for which TCP options are needed to be processed.
 * @offset: Offset at which TCP options start.
 * @total_opt_size: Total number of bytes we are expecting in the TCP options.
 * @return: A success status will be returned if TCP options were successfully
 *  parsed.
 * This function will parse and process TCP options received in a TCP packet.
 */
static int32_t tcp_process_options(FS_BUFFER *buffer, TCP_PORT *port, uint32_t offset, uint16_t total_opt_size)
{
    int32_t status = SUCCESS;
    uint16_t opt_index = 0, opt_value_16;
    uint8_t opt_type, opt_len;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* If we don't have anticipated number of bytes in the provided buffer. */
    if ((offset + total_opt_size) > buffer->total_length)
    {
        /* Invalid header was parsed. */
        status = NET_INVALID_HDR;
    }

    /* While we have some TCP option to process. */
    while ((status == SUCCESS) && (opt_index < total_opt_size))
    {
        /* Pull the option type. */
        ASSERT(fs_buffer_pull_offset(buffer, &opt_type, 1, (offset + opt_index), FS_BUFFER_INPLACE));
        opt_index ++;

        /* If we are also expecting option length. */
        if ((opt_type != TCP_OPT_END) && (opt_type != TCP_OPT_NOP))
        {
            /* If we do have at least a byte to parse the option length. */
            if (opt_index < total_opt_size)
            {
                /* Pull option length. */
                ASSERT(fs_buffer_pull_offset(buffer, &opt_len, 1, (offset + opt_index), FS_BUFFER_INPLACE));
                opt_index ++;

                /* Option length must be at least 2 bytes. */
                if (opt_len >= 2)
                {
                    /* We have already parsed type and length of the the option. */
                    opt_len = (uint8_t)(opt_len - 2);
                }
                else
                {
                    /* Invalid option was parsed. */
                    status = NET_INVALID_HDR;
                }
            }
            else
            {
                /* Invalid option was parsed. */
                status = NET_INVALID_HDR;
            }
        }
        else
        {
            /* No more data left to parse for this option. */
            opt_len = 0;
        }

        if (status == SUCCESS)
        {
            /* If parsed option length can actually exist in the anticipated
             * TCP option length. */
            if ((opt_len + opt_index) <= total_opt_size)
            {
                /* Process the TCP option. */
                switch (opt_type)
                {
                /* Maximum segment size. */
                case TCP_OPT_MSS:

                    /* If we do have anticipated number of bytes for this
                     * option. */
                    if (opt_len == 2)
                    {
                        /* Pull maximum segment size. */
                        ASSERT(fs_buffer_pull_offset(buffer, &opt_value_16, 2, (offset + opt_index), (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)));

                        /* If remote has smaller maximum segment size. */
                        if (opt_value_16 < port->mss)
                        {
                            /* Update own maximum segment size. */
                            port->mss = opt_value_16;
                        }

                        /* Window scale is being used. */
                        port->flags |= TCP_FLAG_MSS;
                    }
                    else
                    {
                        /* Invalid option was parsed. */
                        status = NET_INVALID_HDR;
                    }

                    break;

                /* Window size scale. */
                case TCP_OPT_WIND_SCALE:

                    /* If we do have anticipated number of bytes for this
                     * option. */
                    if (opt_len == 1)
                    {
                        /* Pull send window scale sent by remote. */
                        ASSERT(fs_buffer_pull_offset(buffer, &port->snd_wnd_scale, 1, (offset + opt_index), FS_BUFFER_INPLACE));

                        /* Window scale is being used. */
                        port->flags |= TCP_FLAG_WND_SCALE;
                    }
                    else
                    {
                        /* Invalid option was parsed. */
                        status = NET_INVALID_HDR;
                    }

                    break;

                /* End of option list. */
                case TCP_OPT_END:

                    /* No next option. */
                    status = NET_NO_NEXT_OPT;

                    break;

                /* Padding option. */
                case TCP_OPT_NOP:

                /* Unknown TCP option. */
                default:

                    /* Nothing to do here. */
                    break;
                }

                /* Move option index past this option. */
                opt_index = (uint16_t)(opt_index + opt_len);
            }
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_process_options */

/*
 * tcp_add_option
 * @buffer: File system buffer needed to be populated.
 * @type: Option type.
 * @length: Length of the TCP option needed to be added.
 * @data: Option value needed to be added.
 * @flags: Option value flags, FS_BUFFER_PACKED if this is a packed option.
 * @return: A success status will be returned if DHCP header was successfully
 *  added to the given buffer.
 * This function will add an option on the provided buffer.
 */
static int32_t tcp_add_option(FS_BUFFER *buffer, uint8_t type, uint8_t length, void *value, uint8_t flags)
{
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Append option type. */
    status = fs_buffer_push(buffer, (uint8_t *)&type, 1, 0);

    /* If we do need to add option value. */
    if ((status == SUCCESS) && (length > 1))
    {
        /* Append option length. */
        status = fs_buffer_push(buffer, (uint8_t *)&length, 1, 0);

        /* If we do have a option value to add. */
        if ((status == SUCCESS) && (value != NULL))
        {
            /* Append option value. */
            status = fs_buffer_push(buffer, (uint8_t *)value, (uint32_t)(length - 2), flags);
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_add_option */

/*
 * tcp_add_options
 * @buffer: File system buffer in which we need to add TCP options.
 * @port: TCP port for which options are needed to be added.
 * @opt_flags: TCP option flags defining what TCP options we need to add.
 * @opt_size: At return this will contain the number of bytes added as part of
 *  TCP options.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if TCP options were successfully
 *  added.
 * This function will add TCP configuration options for the given TCP port, as
 * defined by the option flags.
 */
static int32_t tcp_add_options(FS_BUFFER *buffer, TCP_PORT *port, uint8_t opt_flags, uint8_t *opt_size, uint8_t flags)
{
    int32_t status = SUCCESS;
    uint8_t ret_size = 0, opt_value_8;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* If remote sent a maximum segment size. */
    if (opt_flags & TCP_FLAG_MSS)
    {
        /* Add maximum segment size option. */
        status = tcp_add_option(buffer, TCP_OPT_MSS, 4, &port->mss, (FS_BUFFER_PACKED | flags));

        if (status == SUCCESS)
        {
            /* Add number of bytes we have added for MSS option. */
            ret_size = (uint8_t)(ret_size + 4);
        }
    }

    /* If remote is also using window scale. */
    if ((status == SUCCESS) && (opt_flags & TCP_FLAG_WND_SCALE))
    {
        /* Add window scaling option. */
        opt_value_8 = TCP_WND_SCALE;
        status = tcp_add_option(buffer, TCP_OPT_WIND_SCALE, 3, &opt_value_8, flags);

        if (status == SUCCESS)
        {
            /* Save the receive window scale. */
            port->rcv_wnd_scale = TCP_WND_SCALE;

            /* Add number of bytes we have added for window scale option. */
            ret_size = (uint8_t)(ret_size + 3);
        }
    }

    /* Align TCP options to a 4-byte boundary. */
    while ((status == SUCCESS) && ((ret_size % 4) != 0))
    {
        /* Add a NOP option. */
        status = tcp_add_option(buffer, TCP_OPT_NOP, 1, NULL, flags);
        ret_size = (uint8_t)(ret_size + 1);
    }

    if (status == SUCCESS)
    {
        /* Return to number of bytes we have added for options. */
        *opt_size = ret_size;
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_add_options */

/*
 * tcp_timer_register
 * @port: TCP port for which timer is needed to be registered.
 * This function will register timer for giver TCP port.
 */
static void tcp_timer_register(TCP_PORT *port)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Initialize TCP retransmission condition data. */
    port->timeout_suspend.condition.data = port;

    /* Disable retransmission timer by default. */
    port->timeout_suspend.suspend.timeout = MAX_WAIT;
    port->timeout_suspend.suspend.timeout_enabled = FALSE;

    /* Add networking condition to process retransmission timer events. */
    net_condition_add(&port->timeout_suspend.condition, &port->timeout_suspend.suspend, &tcp_timeout_callback, (FD)port);

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_timer_register */

/*
 * tcp_timer_unregister
 * @port: TCP port for which timer is needed to be unregistered.
 * This function will unregister timer for this TCP port.
 */
static void tcp_timer_unregister(TCP_PORT *port)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Free all the retransmission buffers. */
    tcp_rtx_free_all(port);

    /* Remove networking condition for retransmission timer. */
    net_condition_remove(&port->timeout_suspend.condition);

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_timer_unregister */

/*
 * tcp_timeout_update
 * @port: TCP port for which a timeout was updated.
 * This function will schedule next retransmission on a given TCP port.
 */
static void tcp_timeout_update(TCP_PORT *port)
{
    uint32_t timeout = port->event_timeout;
    uint8_t timeout_enabled = port->event_timeout_enable;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* If we need to use the retransmission timer. */
    if ((port->rtx_timeout_enable == TRUE) && ((timeout_enabled == FALSE) || (INT32CMP(timeout, port->rtx_timeout) > 0)))
    {
        /* We need to perform the retransmission before a TCP event. */
        timeout = port->rtx_timeout;
        timeout_enabled = TRUE;
    }

    /* If we need to enable timeout. */
    if (timeout_enabled == TRUE)
    {
        /* Set required next timeout. */
        port->timeout_suspend.suspend.timeout = timeout;

        /* Set the flag that timeout is enabled for this suspend. */
        port->timeout_suspend.suspend.timeout_enabled = TRUE;
    }
    else
    {
        /* Timeout is not enabled. */
        port->timeout_suspend.suspend.timeout_enabled = FALSE;
    }

    /* Networking condition data has been updated. */
    net_condition_updated();

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_timeout_update */

/*
 * tcp_fast_rtx
 * @port: TCP port for which fast retransmission of a segment is required.
 * @seq_num: Segment sequence number for which fast retransmission is required.
 * This function will schedule fast retransmission of a TCP segment.
 */
static void tcp_fast_rtx(TCP_PORT *port, uint32_t seq_num)
{
    uint32_t i;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Traverse the RTX list. */
    for (i = 0; i < TCP_NUM_RTX; i++)
    {
        /* Test if this frame has sequence equal or  greater than the required
         * sequence number. */
        if ((port->rtx[i].flags & TCP_RTX_IN_USE) && (port->rtx[i].seq_num == seq_num))
        {
            /* If the buffer we are supposed to retransmit has returned. */
            if (port->rtx[i].flags & TCP_RTX_BUFFER_RETURNED)
            {
                /* Clear the buffer returned flag. */
                port->rtx[i].flags &= (uint8_t)~(TCP_RTX_BUFFER_RETURNED);

                /* Lock the buffer descriptor. */
                ASSERT(fd_get_lock(port->rtx[i].buffer->fd));

                /* Retransmit a TCP buffer. */
                net_device_buffer_transmit(port->rtx[i].buffer, NET_PROTO_IPV4, 0);

                /* Release the buffer lock. */
                fd_release_lock(port->rtx[i].buffer->fd);
            }
        }
    }

    /* Update timeout for this port. */
    tcp_timeout_update(port);

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_fast_rtx */

/*
 * tcp_timeout_callback
 * @data: TCP port data for this timer.
 * @status: Resumption status.
 * This function is networking condition callback for a TCP timer.
 */
static void tcp_timeout_callback(void *data, int32_t status)
{
    TCP_PORT *port = (TCP_PORT *)data;
    int32_t i, least_rtx = -1;
    uint8_t rtx_picked = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(status);

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Get lock for this port. */
    if (fd_get_lock((FD)port) == SUCCESS)
    {
        switch (port->state)
        {
        /* If we are in time wait state. */
        case TCP_SOCK_TIME_WAIT:

            /* Move to the closed state. */
            port->state = TCP_SOCK_COLSED;

            /* Stop the port timer. */
            port->event_timeout = MAX_WAIT;
            port->event_timeout_enable = FALSE;

            /* Resume any tasks waiting for state change. */
            tcp_resume_socket(port, (FS_BLOCK_READ | FS_BLOCK_WRITE));

            break;

        /* If we have sent a SYN. */
        case TCP_SOCK_SYN_SENT:

        /* If we are in SYN received state. */
        case TCP_SOCK_SYN_RCVD:

        /* If we are waiting for last ACK. */
        case TCP_SOCK_LAST_ACK:

        /* If we are in FIN-WAIT-1 state. */
        case TCP_SOCK_FIN_WAIT_1:

        /* If we are in closing state. */
        case TCP_SOCK_CLOSING:

        /* If we are in established state. */
        case TCP_SOCK_ESTAB:

            /* If a retransmission is needed to be performed. */
            if (INT32CMP(current_system_tick(), port->rtx_timeout) >= 0)
            {
                /* Traverse the RTX list and try to find the least sequence
                 * number for this tick. */
                for (i = 0; i < TCP_NUM_RTX; i++)
                {
                    /* If this entry is enabled. */
                    if (port->rtx[i].flags & TCP_RTX_IN_USE)
                    {
                        /* If this entry has the least sequence number. */
                        if ((rtx_picked == FALSE) || (INT32CMP(port->rtx[least_rtx].seq_num, port->rtx[i].seq_num) >= 0))
                        {
                            /* Flag that we just picked a retransmission structure and save it's index. */
                            rtx_picked = TRUE;
                            least_rtx = i;
                        }
                    }
                }

                /* If we have an retransmission to be invoked. */
                if ((rtx_picked == TRUE) && (least_rtx >= 0))
                {
                    /* If the buffer we are supposed to retransmit has returned. */
                    if (port->rtx[least_rtx].flags & TCP_RTX_BUFFER_RETURNED)
                    {
                        /* Clear the buffer returned flag. */
                        port->rtx[least_rtx].flags &= (uint8_t)~(TCP_RTX_BUFFER_RETURNED);

                        ASSERT(fd_get_lock(port->rtx[least_rtx].buffer->fd));

                        /* Retransmit a TCP buffer. */
                        net_device_buffer_transmit(port->rtx[least_rtx].buffer, NET_PROTO_IPV4, 0);

                        /* Release the buffer lock. */
                        fd_release_lock(port->rtx[least_rtx].buffer->fd);
                    }

                    switch (port->state)
                    {
                    /* If we have sent a SYN. */
                    case TCP_SOCK_SYN_SENT:

                    /* If we are in SYN received state. */
                    case TCP_SOCK_SYN_RCVD:

                    /* If we are waiting for last ACK. */
                    case TCP_SOCK_LAST_ACK:

                    /* If we are in FIN-WAIT-1 state. */
                    case TCP_SOCK_FIN_WAIT_1:

                    /* If we are in closing state. */
                    case TCP_SOCK_CLOSING:

                        /* Next time out will be at RTO. */
                        port->rtx_timeout = current_system_tick() + TCP_RTO;

                        break;

                    /* If we are in established state. */
                    case TCP_SOCK_ESTAB:

                        /* Double the retransmission timer. */
                        port->rtx_time = port->rtx_time * 2;

                        /* If we exceeded the maximum value. */
                        if (port->rtx_time > TCP_MAX_RTO)
                        {
                            /* Use the maximum value. */
                            port->rtx_time = TCP_MAX_RTO;
                        }

                        /* Adjust the retransmission timer. */
                        port->rtx_timeout = current_system_tick() + port->rtx_time;

                        break;
                    }
                }

            }

            break;

        default:

            /* Free all the retransmission buffers. */
            tcp_rtx_free_all(port);

            break;
        }

        /* Update timeout for this port. */
        tcp_timeout_update(port);

        /* Release lock for this TCP port. */
        fd_release_lock((FD)port);
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_timeout_callback */

/*
 * tcp_send_segment
 * @port: TCP port on which a segment is needed to be sent.
 * @socket_address: Socket address for which this segment is needed to be sent.
 * @seq_num: Sequence number to be sent.
 * @ack_num: Acknowledgment number to be sent.
 * @flags: TCP flags to be sent.
 * @wnd_size: TCP window size to be sent.
 * @data: TCP segment data needed to be attached to this segment.
 * @data_len: Number of bytes in the data buffer.
 * @rtx_on: If TRUE this segment will be retransmitted until stopped, if so any
 *  members passed to this function must remain valid until the life time of
 *  retransmission. Any previous segment queued for retransmission will be
 *  removed.
 * @buffer_flags: Buffer flags to be used.
 * @return: A success status will be returned if a TCP segment was successfully
 *  sent, NET_NO_RTX_AVAILABLE will be returned if a free retransmission
 *  structure was not found, FS_BUFFER_NO_SPACE will be returned if we ran out
 *  of buffers, NET_INVALID_FD will be returned if a valid device was not found
 *  to send this frame.
 * This function will send a TCP segment on the networking interface.
 */
static int32_t tcp_send_segment(TCP_PORT *port, SOCKET_ADDRESS *socket_address, uint32_t seq_num, uint32_t ack_num, uint16_t flags, uint16_t wnd_size, uint8_t *data, int32_t data_len, uint8_t rtx_on, uint8_t buffer_flags)
{
    NET_DEV *net_device;
    FS_BUFFER *buffer = NULL;
    int32_t status = SUCCESS;
    FD buffer_fd;
    uint8_t opt_size = 0, opt_flags;
    uint16_t csum;
    TCP_RTX_DATA *rtx = NULL;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Keep updating the ISS we are using for each sequence number
     * we increment. */
    if (tcp_iss < port->snd_nxt)
    {
        /* Next TCP ISS should be greater than our last segment's
         * sequence number. */
        tcp_iss = port->snd_nxt;
    }

    /* Get the local networking interface descriptor. */
    net_device = ipv4_get_source_device(socket_address->local_ip);

    /* If we have a valid networking device. */
    if ((status == SUCCESS) && (net_device != NULL))
    {
        SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_DEBUG, "will be using %s for %d.%d.%d.%d", ((FS *)net_device->fd)->name, SYS_LOG_IP(socket_address->local_ip));

        /* Save the device descriptor. */
        buffer_fd = net_device->fd;

        /* Release lock for the port. */
        fd_release_lock(port);

        /* Obtain lock for buffer file descriptor. */
        ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

        /* Get a buffer keeping threshold buffers on the descriptor. */
        buffer = fs_buffer_get(buffer_fd, FS_BUFFER_LIST, buffer_flags);

        /* If buffer was not allocated. */
        if (buffer != NULL)
        {
            /* If we need to retransmit this frame. */
            if (rtx_on == TRUE)
            {
                /* Release lock for buffer descriptor. */
                fd_release_lock(buffer_fd);

                /* Lock the TCP port. */
                ASSERT(fd_get_lock(port));

                /* Get a free retransmission structure. */
                rtx = tcp_get_rtx_free(port);

                /* If a free retransmission structure was found. */
                if (rtx != NULL)
                {
                    /* Lets return this buffer to us if required. */
                    buffer->free = &tcp_rtx_return_buffer;
                    buffer->free_data = rtx;

                    /* Save the buffer we will retransmit. */
                    rtx->buffer = buffer;

                    /* Save the port for which we will do a retransmission. */
                    rtx->port = port;

                    /* Save the segment length and the sequence number of this buffer. */
                    rtx->seq_num = seq_num;
                    rtx->seg_len = (uint16_t)data_len;

                    /* If retransmission timer is not already running. */
                    if (port->rtx_timeout_enable == FALSE)
                    {
                        /* Save the tick at which we want this buffer to be retransmitted. */
                        port->rtx_timeout = (current_system_tick() + TCP_RTO);
                        port->rtx_timeout_enable = TRUE;
                        port->rtx_time = TCP_RTO;
                    }
                }
                else
                {
                    /* Return an error. */
                    status = NET_NO_RTX_AVAILABLE;
                }

                /* Release lock for the port. */
                fd_release_lock(port);

                /* Obtain lock for buffer file descriptor. */
                ASSERT(fd_get_lock(buffer_fd) != SUCCESS);
            }

            /* If we do have some data to attach on this segment. */
            if ((status == SUCCESS) && (data != NULL))
            {
                /* Add given data on the buffer. */
                status = fs_buffer_push(buffer, data, (uint32_t)data_len, buffer_flags);
            }

            /* If segment data was successfully added on the buffer. */
            if (status == SUCCESS)
            {
                /* If SYN is being sent. */
                if (flags & TCP_HDR_FLAG_SYN)
                {
                    /* Release lock for buffer descriptor. */
                    fd_release_lock(buffer_fd);

                    /* Lock the TCP port. */
                    ASSERT(fd_get_lock(port));

                    /* If we are not ACKing the options sent by remote. */
                    if ((flags & TCP_HDR_FLAG_ACK) == 0)
                    {
                        /* Send all supported options. */
                        opt_flags = (TCP_FLAG_WND_SCALE | TCP_FLAG_MSS);
                    }
                    else
                    {
                        /* Add only the options we received from remote. */
                        opt_flags = port->flags;
                    }

                    /* Add TCP configuration options. */
                    status = tcp_add_options(buffer, port, opt_flags, &opt_size, (buffer_flags & (uint8_t)(~FS_BUFFER_SUSPEND)));

                    /* Release lock for the port. */
                    fd_release_lock(port);

                    /* Obtain lock for buffer file descriptor. */
                    ASSERT(fd_get_lock(buffer_fd) != SUCCESS);
                }
            }

            if (status == SUCCESS)
            {
                /* Add TCP header with ACK and SYN flag. */
                status = tcp_header_add(buffer, socket_address, seq_num, ack_num, flags, wnd_size, opt_size, buffer_flags);
            }

            /* If TCP header was successfully added. */
            if (status == SUCCESS)
            {
                /* Calculate checksum for TCP header. */
                status = net_pseudo_csum_calculate(buffer, socket_address->local_ip, socket_address->foreign_ip, IP_PROTO_TCP, (uint16_t)buffer->total_length, 0, buffer_flags, &csum);

                /* If checksum was successfully calculated. */
                if (status == SUCCESS)
                {
                    /* Push the TCP checksum on the buffer. */
                    status = fs_buffer_push_offset(buffer, &csum, 2, TCP_HRD_CSUM_OFFSET, (uint8_t)(buffer_flags | FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
                }
            }

            if (status == SUCCESS)
            {
                /* Add IP header on this buffer. */
                status = ipv4_header_add(buffer, IP_PROTO_TCP, socket_address->local_ip, socket_address->foreign_ip, buffer_flags);
            }

            /* If IP header was successfully added. */
            if (status == SUCCESS)
            {
                /* If we want retransmission of this segment. */
                if (rtx != NULL)
                {
                    /* Lets return this buffer to us if required. */
                    buffer->free = &tcp_rtx_return_buffer;
                    buffer->free_data = rtx;
                }

                /* Transmit this TCP packet. */
                status = net_device_buffer_transmit(buffer, NET_PROTO_IPV4, buffer_flags);
            }

            /* If a frame was transmitted but was not consumed. */
            if (status == SUCCESS)
            {
                /* If retransmission is enabled. */
                if (rtx != NULL)
                {
                    /* Release lock for buffer descriptor. */
                    fd_release_lock(buffer_fd);

                    /* Lock the TCP port. */
                    ASSERT(fd_get_lock(port));

                    /* Buffer was not consumed. */
                    rtx->flags |= TCP_RTX_BUFFER_RETURNED;

                    /* Release lock for the port. */
                    fd_release_lock(port);

                    /* Obtain lock for buffer file descriptor. */
                    ASSERT(fd_get_lock(buffer_fd) != SUCCESS);
                }
            }

            /* If buffer was consumed. */
            else if (status == NET_BUFFER_CONSUMED)
            {
                /* Reset the status. */
                status = SUCCESS;
            }
            else
            {
                /* Clear the free data. */
                buffer->free = NULL;
                buffer->free_data = 0;

                /* Add the allocated buffer back to the descriptor. */
                fs_buffer_add_buffer_list(buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            }

            /* If segment was not sent and we allocated a retransmission structure. */
            if ((status != SUCCESS) && (rtx != NULL))
            {
                /* Clear the RTX flags to free it. */
                rtx->flags = 0;
            }
        }
        else
        {
            /* There are not buffers available to send a TCP segment. */
            status = FS_BUFFER_NO_SPACE;
        }

        /* Release lock for buffer descriptor. */
        fd_release_lock(buffer_fd);

        /* Lock the TCP port. */
        ASSERT(fd_get_lock(port));

        if (status == SUCCESS)
        {
            /* If retransmission is enabled. */
            if (rtx != NULL)
            {
                /* TCP port timer has been updated. */
                tcp_timeout_update(port);
            }
        }
    }
    else if (net_device == NULL)
    {
        SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_WARN, "unable to find a suitable device for %d.%d.%d.%d", SYS_LOG_IP(socket_address->local_ip));

        /* Networking device was not resolved. */
        status = NET_INVALID_FD;
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_send_segment */

/*
 * tcp_check_sequence
 * @seg_seq: Received segment sequence.
 * @seg_len: Received number of bytes in the segment.
 * @rcv_nxt: Sequence expected to receive next.
 * @rcv_wnd: Receive window size.
 * @dst_ip: Destination address from the IP header.
 * @return: TRUE will be return if received sequence number is acceptable,
 *  otherwise FALSE will be returned.
 * This function will verify that received sequence of incoming segment is
 * acceptable.
 */
static uint8_t tcp_check_sequence(uint32_t seg_seq, uint32_t seg_len, uint32_t rcv_nxt, uint32_t rcv_wnd)
{
    uint8_t seq_ok = FALSE;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* RCV.WND = 0 ? */
    if (rcv_wnd == 0)
    {
        /* SEG.LEN = 0 ? */
        if (seg_len == 0)
        {
            /* SEG.SEQ = RCV.NXT ? */
            if (seg_seq == rcv_nxt)
            {
                /* Received sequence is acceptable. */
                seq_ok = TRUE;
            }
        }
    }

    else
    {
        /* SEG.LEN = 0 ? */
        if (seg_len == 0)
        {
            /* (RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND) ? */
            if ((INT32CMP(rcv_nxt, seg_seq) <= 0) && (INT32CMP(seg_seq, (rcv_nxt + rcv_wnd)) < 0))
            {
                /* Received sequence is acceptable. */
                seq_ok = TRUE;
            }
        }

        /* (RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND)
         * OR
         * (RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND) ? */
        else if (((INT32CMP(rcv_nxt, seg_seq) <= 0) && (INT32CMP(seg_seq, (rcv_nxt + rcv_wnd)) < 0)) || ((INT32CMP(rcv_nxt, (seg_seq + (seg_len - 1))) <= 0) && (INT32CMP((seg_seq + (seg_len - 1)), (rcv_nxt + rcv_wnd)) < 0)))
        {
            /* Received sequence is acceptable. */
            seq_ok = TRUE;
        }
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

    /* Return if received sequence number is acceptable or not. */
    return (seq_ok);

} /* tcp_check_sequence */

/*
 * tcp_process_finbit
 * @port: TCP port on which FIN bit was received.
 * @fin_seq: Sequence number at which the FIN control actually exists.
 * This function process FIN bit received on a TCP port.
 */
static void tcp_process_finbit(TCP_PORT *port, uint32_t fin_seq)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* RCV.NXT = SEG.SEQ + 1. */
    port->rcv_nxt = fin_seq + 1;

    /* Segment (SEQ=SND.NXT, ACK=RCV.NXT, CTL=ACK). */
    tcp_send_segment(port, &port->socket_address, port->snd_nxt, port->rcv_nxt, (TCP_HDR_FLAG_ACK | TCP_HDR_FLAG_FIN), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);

    /* FIN was sent. */
    port->snd_nxt = (port->snd_una + 1);

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_process_finbit */

/*
 * tcp_oo_buffer_process
 * @node: An existing buffer in the list.
 * @param: Out-of-order buffer parameter.
 * @return: TRUE will be returned if we need to stop this process and given
 *  buffer should be inserted before this segment.
 * This function will check if we can insert a new out-of-order TCP segment in
 * the existing TCP segments.
 */
static uint8_t tcp_oo_buffer_process(void *node, void *param)
{
    FS_BUFFER *buffer = (FS_BUFFER *)node;
    TCP_OO_PARAM *oo_param = (TCP_OO_PARAM *)param;
    uint32_t seg_seq;
    uint8_t stop = FALSE;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Pull the sequence number for these buffers. */
    ASSERT(fs_buffer_pull(buffer, &seg_seq, 4, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* If this segment comes before the segment we need to insert. */
    if (seg_seq > oo_param->seg_seq)
    {
        /* If this segment overlaps the next segment. */
        if ((oo_param->seg_seq + oo_param->seg_len) >= seg_seq)
        {
            /* This is a conflicting segment. */
            oo_param->flags |= TCP_FLAG_SEG_CONFLICT;
        }

        /* Insert the new segment before this segment. */
        stop = TRUE;
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

    /* Return if we need to insert this fragment here. */
    return (stop);

} /* tcp_oo_buffer_process */

/*
 * tcp_rx_buffer_merge
 * @port: TCP port for which receive buffers are needed to be merged.
 * @buffer: Received buffer needed to be merged.
 * @seg_len: Segment length of this buffer.
 * @seg_seq: Received segment sequence.
 * @return: A success status will be returned if the received buffer was
 *  successfully merged.
 * This function will merge received buffers on this port.
 */
static int32_t tcp_rx_buffer_merge(TCP_PORT *port, FS_BUFFER *buffer, uint16_t seg_len, uint32_t seg_seq)
{
    int32_t status = SUCCESS;
    FD buffer_fd = buffer->fd;
    FS_BUFFER *prev_buffer = NULL;
    TCP_OO_PARAM oo_param;
    uint8_t new_data = FALSE;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Acquire lock for the buffer file descriptor. */
    ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

    /* Remove all the data from the buffer except the actual TCP segment. */
    fs_buffer_pull(buffer, NULL, (buffer->total_length - seg_len), 0);

    /* SEG.SEQ = RCV.NXT ? */
    if (seg_seq == port->rcv_nxt)
    {
        /* Copy segment's data to the RCV Buffer. */

        /* If we do have a receive buffer. */
        if (port->rx_buffer.buffer != NULL)
        {
            /* Should never happen. */
            ASSERT(port->rx_buffer.buffer->fd != buffer_fd);

            /* Move all the data from this buffer to the existing receive
             * buffer. */
            fs_buffer_move_data(port->rx_buffer.buffer, buffer, 0);
        }
        else
        {
            /* Use this buffer as the receive buffer. */
            port->rx_buffer.buffer = buffer;

            /* This buffer is now consumed. */
            status = NET_BUFFER_CONSUMED;
        }

        /* RCV.NXT := SEG.SEQ + SEG.LEN */
        port->rcv_nxt = seg_seq + seg_len;

        /* Process any out-of-order buffers we have already received. */
        for (buffer = port->rx_buffer.oorx_list.head; (buffer != NULL); buffer = buffer->next)
        {
            /* Pull the sequence number for this buffer. */
            ASSERT(fs_buffer_pull(buffer, &seg_seq, 4, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

            /* If this is the anticipated next segment. */
            if (port->rcv_nxt == seg_seq)
            {
                /* Pull and remove the sequence number we added. */
                ASSERT(fs_buffer_pull(buffer, NULL, 4, 0) != SUCCESS);

                /* RCV.NXT := HSEG.SEQ + HSEG.LEN */
                port->rcv_nxt = seg_seq + buffer->total_length;

                /* Move all the data from this buffer to the receive buffer. */
                fs_buffer_move_data(port->rx_buffer.buffer, buffer, 0);
            }
            else
            {
                /* Just break as this list is already sorted. */
                break;
            }
        }

        if (port->rx_buffer.oorx_list.head != NULL)
        {
            /* Clear any remaining buffers in the out-of-order buffer list. */
            fs_buffer_add_buffer_list(port->rx_buffer.oorx_list.head, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            port->rx_buffer.oorx_list.head = port->rx_buffer.oorx_list.tail = NULL;
        }

        /* Move receive window back to original. */
        port->rcv_wnd = TCP_WND_SIZE;

        /* We have received some new data. */
        new_data = TRUE;
    }
    else
    {
        /* Should never happen. */
        ASSERT((port->rx_buffer.oorx_list.head != NULL) && (port->rx_buffer.oorx_list.head->fd != buffer->fd));

        /* Initialize out-of-order buffer parameter. */
        oo_param.seg_seq = seg_seq;
        oo_param.seg_len = seg_len;
        oo_param.flags = 0;

        /* Put this buffer on the out-of-order buffer list. */
        sll_search(&port->rx_buffer.oorx_list, (void **)&prev_buffer, &tcp_oo_buffer_process, &oo_param, OFFSETOF(FS_BUFFER, next));

        /* If given segment did not conflict with any of the existing segments. */
        if ((oo_param.flags & TCP_FLAG_SEG_CONFLICT) == 0)
        {
            /* Push the sequence number for this buffer on the buffer head. */
            status = fs_buffer_push(buffer, &seg_seq, 4, (FS_BUFFER_PACKED));

            if (status == SUCCESS)
            {
                /* Put this buffer on the out-of-order buffer list. */
                sll_add_node(&port->rx_buffer.oorx_list, buffer, prev_buffer, OFFSETOF(FS_BUFFER, next));

                /* This buffer is now consumed. */
                status = NET_BUFFER_CONSUMED;

                /* RCV.WND := RCV.WND + SEG.LEN */
                port->rcv_wnd = (port->rcv_wnd + seg_len);
            }
        }
    }

    /* Release semaphore for the buffer file descriptor. */
    fd_release_lock(buffer_fd);

    if ((status == SUCCESS) || (status == NET_BUFFER_CONSUMED))
    {
        /* If we have received new data. */
        if (new_data == TRUE)
        {
            /* Data available to read. */
            tcp_resume_socket(port, FS_BLOCK_READ);
        }

        /* Segment (SEQ=SND.NXT, ACK=RCV.NXT, CTL=ACK) */
        tcp_send_segment(port, &port->socket_address, port->snd_nxt, port->rcv_nxt, (TCP_HDR_FLAG_ACK), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /*  Return status to the caller. */
    return (status);

} /* tcp_rx_buffer_merge */

/*
 * tcp_buffer_get_ihl_flags
 * @buffer: Buffer from which headers are required.
 * @ihl: Internet header length will be returned here.
 * @flags: TCP flag field will be returned here.
 * This function will read and return the IHL and TCP header flags for the
 * given TCP buffer.
 */
static void tcp_buffer_get_ihl_flags(FS_BUFFER *buffer, uint8_t *ihl, uint16_t *flags)
{
    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Peek the version and IHL. */
    ASSERT(fs_buffer_pull_offset(buffer, ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);
    (*ihl) = (uint8_t)(((*ihl) & IPV4_HDR_IHL_MASK) << 2);

    /* Pull the TCP flags. */
    ASSERT(fs_buffer_pull_offset(buffer, flags, 2, (uint32_t)((*ihl) + TCP_HRD_FLAGS_OFFSET), (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)));

    SYS_LOG_FUNCTION_EXIT(TCP);

} /* tcp_buffer_get_ihl_flags */

/*
 * tcp_read_buffer
 * @fd: File descriptor.
 * @buffer: File system buffer will be returned here.
 * @size: Size of buffer.
 * @return: Number of bytes read.
 * This function will read a buffer from the given TCP port.
 */
static int32_t tcp_read_buffer(void *fd, uint8_t *buffer, int32_t size)
{
    TCP_PORT *port = (TCP_PORT *)fd;
    int32_t nbytes = 0;

    /* For now unused. */
    UNUSED_PARAM(size);

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* If we do have some data on this port. */
    if (port->rx_buffer.buffer != NULL)
    {
        /* Return number of bytes available. */
        nbytes = (int32_t)port->rx_buffer.buffer->total_length;

        /* Return the read buffer to the caller. */
        *(FS_BUFFER **)buffer = port->rx_buffer.buffer;
        port->rx_buffer.buffer = NULL;
    }

    /* If we are no longer in established state. */
    if (port->state != TCP_SOCK_ESTAB)
    {
        /* If we are not actually returning any data. */
        if (nbytes == 0)
        {
            /* Return status that this socket is now closed. */
            nbytes = NET_CLOSED;
        }
    }
    else
    {
        /* If there is nothing more to read from this port. */
        fd_data_flushed(fd);
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

    return (nbytes);

} /* tcp_read_buffer */

/*
 * tcp_read_data
 * @fd: File descriptor.
 * @buffer: Buffer in which data will be read.
 * @size: Size of buffer.
 * @return: Number of bytes read.
 * This function will read data from a TCP port.
 */
static int32_t tcp_read_data(void *fd, uint8_t *buffer, int32_t size)
{
    TCP_PORT *port = (TCP_PORT *)fd;
    FS_BUFFER *fs_buffer;
    int32_t ret_size = 0;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Read a buffer from given TCP port. */
    ret_size = tcp_read_buffer(fd, (uint8_t *)&fs_buffer, 0);

    /* If we did read some data. */
    if (ret_size > 0)
    {
        /* Get lock for the buffer file descriptor. */
        ASSERT(fd_get_lock(fs_buffer->fd) != SUCCESS);

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
        ASSERT(fs_buffer_pull(fs_buffer, buffer, (uint32_t)ret_size, 0) != SUCCESS);

        /* If we still have some data left on this buffer. */
        if (fs_buffer->total_length != 0)
        {
            /* If we have again set the receive buffer somehow? */
            if (port->rx_buffer.buffer != NULL)
            {
                /* Should never happen. */
                ASSERT(port->rx_buffer.buffer->fd != fs_buffer->fd);

                /* Move all the data from this buffer to the new receive buffer on head. */
                fs_buffer_move_data(port->rx_buffer.buffer, fs_buffer, FS_BUFFER_HEAD);

                /* Return this buffer to it's owner. */
                fs_buffer_add(fs_buffer->fd, fs_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            }
            else
            {
                /* Use this buffer as the receive buffer. */
                port->rx_buffer.buffer = fs_buffer;
            }

            /* Data is still available on this socket. */
            fd_data_available(fd);
        }

        else
        {
            /* Return this buffer to it's owner. */
            fs_buffer_add(fs_buffer->fd, fs_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Release lock for the buffer file descriptor. */
        fd_release_lock(fs_buffer->fd);
    }

    SYS_LOG_FUNCTION_EXIT(TCP);

    /* Return number of bytes. */
    return (ret_size);

} /* tcp_read_data */

/*
 * tcp_write_buffer
 * @fd: File descriptor.
 * @buffer: File system buffer needed to be written.
 * @size: Size of buffer.
 * @return: Number of bytes sent.
 *  NET_CLOSED will be returned if socket is not in established state and we
 *  cannot send any more data.
 * This function will write a buffer on the TCP socket.
 */
static int32_t tcp_write_buffer(void *fd, uint8_t *buffer, int32_t size)
{
    TCP_PORT *port = (TCP_PORT *)fd;
    int32_t nbytes = 0;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* For now unused. */
    UNUSED_PARAM(size);
    UNUSED_PARAM(port);
    UNUSED_PARAM(nbytes);
    UNUSED_PARAM(buffer);

    /* Not supported. */
    ASSERT(TRUE);

    SYS_LOG_FUNCTION_EXIT(TCP);

    /* Return number of bytes sent. */
    return (nbytes);

} /* tcp_write_buffer */

/*
 * tcp_write_data
 * @fd: File descriptor.
 * @buffer: Data buffer needed to be sent.
 * @size: Number of bytes in the buffer to be sent.
 * @return: Number of bytes sent.
 *  NET_CLOSED will be returned if socket is not in established state and we
 *  cannot send any more data.
 * This function will write a given data buffer on the TCP socket.
 */
static int32_t tcp_write_data(void *fd, uint8_t *buffer, int32_t size)
{
    TCP_PORT *port = (TCP_PORT *)fd;
    int32_t nbytes = 0, status = SUCCESS;
    int32_t sent = 0;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* We will only send data in established state. */
    if (port->state == TCP_SOCK_ESTAB)
    {
        SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_DEBUG, "SND_NXT: %ld, SND_UNA: %ld, SND_WND: %ld, MSS: %d", port->snd_nxt, port->snd_una, port->snd_wnd, port->mss);

        /* While we have something to transmit. */
        while ((status == SUCCESS) && (size > 0))
        {
            /* Verify that we do have space in the send window to send some data. */
            if ((port->snd_wnd != 0) && (port->mss != 0))
            {
                /* If we need to send more data then we can send in a single segment. */
                if (size > port->mss)
                {
                    /* Only send data that can be sent in a single segment. */
                    nbytes = port->mss;
                }

                /* If we need to send more data then we can send in send window. */
                else if (size > (int32_t)port->snd_wnd)
                {
                    /* Only send data that can be sent in a send window. */
                    nbytes = (int32_t)port->snd_wnd;
                }
                else
                {
                    /* Send number of bytes we can send. */
                    nbytes = size;
                }

                if (nbytes > 0)
                {
                    /* Send a TCP segment with required data. */
                    status = tcp_send_segment(port, &port->socket_address, port->snd_nxt, port->rcv_nxt, (TCP_HDR_FLAG_ACK), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), buffer, nbytes, TRUE, (FS_BUFFER_TH | FS_BUFFER_SUSPEND));

                    if (status == SUCCESS)
                    {
                        /* If we are still in established state. */
                        if (port->state == TCP_SOCK_ESTAB)
                        {
                            /* SND.NXT := SND.NXT + SEG.LEN */
                            port->snd_nxt = (uint32_t)(port->snd_nxt + (uint32_t)nbytes);

                            /* If we have space on the send window. */
                            if (port->snd_wnd > (uint32_t)nbytes)
                            {
                                /* We just sent a segment adjust the send window. */
                                port->snd_wnd -= (uint32_t)nbytes;
                            }
                            else
                            {
                                /* No space on the send window. */
                                port->snd_wnd = 0;
                            }

                            /* Add the number of bytes sent. */
                            sent += nbytes;

                            /* If there is space on the send window to send some data. */
                            if (port->snd_wnd == 0)
                            {
                                /* Space is now consumed for this fd. */
                                fd_space_consumed(fd);
                            }

                            /* Move ahead the buffer pointer. */
                            buffer += nbytes;
                            size -= nbytes;
                        }
                        else
                        {
                            /* We cannot write data on this socket, return an error. */
                            status = sent = NET_CLOSED;
                        }
                    }
                    else
                    {
                        /* Return this status to the caller. */
                        sent = status;
                    }
                }
            }
            else
            {
                /* Set flag that we don't have any more space in this TCP port. */
                fd_space_consumed(fd);

                break;
            }
        }
    }
    else
    {
        /* We cannot write data on this socket, return an error. */
        status = sent = NET_CLOSED;
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return number of bytes sent. */
    return (sent);

} /* tcp_write_data */

/*
 * tcp_get_rtx_free
 * @port: Port for which retransmission structure is required.
 * This function will allocate and return a free retransmission structure.
 */
static TCP_RTX_DATA *tcp_get_rtx_free(TCP_PORT *port)
{
    uint32_t i;
    TCP_RTX_DATA *rtx = NULL;

    /* Traverse the retransmission list. */
    for (i = 0; i < TCP_NUM_RTX; i++)
    {
        /* If this is a free retransmission structure. */
        if ((port->rtx[i].flags & TCP_RTX_IN_USE) == 0)
        {
            rtx = &port->rtx[i];

            /* Set the flag this is being used. */
            rtx->flags |= TCP_RTX_IN_USE;

            break;
        }
    }

    /* Return the free retransmission structure. */
    return (rtx);

} /* tcp_get_rtx_free */

/*
 * tcp_rtx_return_buffer
 * @data: Retransmission structure to which we need to return this frame.
 * @return: True will be returned if the buffer was successfully returned,
 *  otherwise false will be returned.
 * This function will return a buffer to a TCP socket for retransmission.
 */
static uint8_t tcp_rtx_return_buffer(void *data, FS_BUFFER *buffer)
{
    TCP_RTX_DATA *rtx = (TCP_RTX_DATA *)data;
    uint8_t returned = TRUE;

    fd_release_lock(buffer->fd);

    /* Lock the required TCP port. */
    if (fd_get_lock(rtx->port) == SUCCESS)
    {
        /* If we are still using this retransmission structure. */
        if (rtx->flags & TCP_RTX_IN_USE)
        {
            /* Set the flag that buffer has returned. */
            rtx->flags |= TCP_RTX_BUFFER_RETURNED;
        }
        else
        {
            /* We don't want this buffer. */
            returned = FALSE;
        }

        /* Release the port lock. */
        fd_release_lock(rtx->port);
    }

    /* Again lock the buffer descriptor. */
    ASSERT(fd_get_lock(buffer->fd));

    /* Return if buffer was successfully returned. */
    return (returned);

} /* tcp_rtx_return_buffer */

/*
 * tcp_rtx_process_ack
 * @port: Port for which an ACK is needed to be processed.
 * @ack_num: Received ACK number.
 * @return: True will be returned if this ACK has removed a segment from the
 * retransmission list, otherwise false will be returned.
 * This function will process a received ACK and remove any segments from
 * retransmission list.
 */
static uint8_t tcp_rtx_process_ack(TCP_PORT *port, uint32_t ack_num)
{
    uint8_t freed = FALSE, do_rtx = FALSE;
    uint32_t i;

    /* Traverse the retransmission list. */
    for (i = 0; i < TCP_NUM_RTX; i++)
    {
        /* If this entry is in use. */
        if (port->rtx[i].flags & TCP_RTX_IN_USE)
        {
            /* If remote has ACKed this segment */
            if (INT32CMP(port->rtx[i].seq_num + port->rtx[i].seg_len, ack_num) <= 0)
            {
                ASSERT(port->rtx[i].buffer == NULL);
                ASSERT(fd_get_lock(port->rtx[i].buffer->fd));

                /* Free this buffer when required. */
                port->rtx[i].buffer->free = NULL;
                port->rtx[i].buffer->free_data = NULL;

                /* If RTX buffer has returned. */
                if (port->rtx[i].flags & TCP_RTX_BUFFER_RETURNED)
                {
                    /* Free this buffer. */
                    fs_buffer_add(port->rtx[i].buffer->fd, port->rtx[i].buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
                }

                /* Free this retransmission structure as free. */
                port->rtx[i].flags = 0;

                /* Release buffer lock. */
                fd_release_lock(port->rtx[i].buffer->fd);

                /* We just freed a segment. */
                freed = TRUE;
            }
            else
            {
                /* We do have an entry for which we will need to do
                 * retransmission. */
                do_rtx = TRUE;
            }
        }
    }

    /* If retransmission is needed to be performed. */
    if (do_rtx == TRUE)
    {
        /* Enable the retransmission timer. */
        port->rtx_timeout = current_system_tick() + TCP_RTO;
        port->rtx_time = TCP_RTO;
        port->rtx_timeout_enable = TRUE;
    }
    else
    {
        /* Disable the retransmission timer. */
        port->rtx_timeout_enable = FALSE;
    }

    /* If we did free a segment or need to perform retransmission. */
    if ((freed == TRUE) || (do_rtx == TRUE))
    {
        /* Update timer for this port. */
        tcp_timeout_update(port);
    }

    /* Return if we did free a segment. */
    return (freed);

} /* tcp_rtx_process_ack */

/*
 * tcp_rtx_free_all
 * @port: Port for which we need to free all the retransmission buffers.
 * This function will free all the buffers on the retransmission list, and
 * disable the retransmission timer.
 */
static void tcp_rtx_free_all(TCP_PORT *port)
{
    uint32_t i;

    /* Traverse and free all the retransmission buffers. */
    for (i = 0; i < TCP_NUM_RTX; i++)
    {
        /* If this is being used. */
        if (port->rtx[i].flags & TCP_RTX_IN_USE)
        {
            ASSERT(port->rtx[i].buffer == NULL);
            ASSERT(fd_get_lock(port->rtx[i].buffer->fd));

            /* Lets free this buffer when required. */
            port->rtx[i].buffer->free = NULL;
            port->rtx[i].buffer->free_data = NULL;

            /* If RTX buffer has returned. */
            if (port->rtx[i].flags & TCP_RTX_BUFFER_RETURNED)
            {
                /* Free this buffer. */
                fs_buffer_add(port->rtx[i].buffer->fd, port->rtx[i].buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            }

            /* Release buffer lock. */
            fd_release_lock(port->rtx[i].buffer->fd);
        }

        /* Disable this entry. */
        port->rtx[i].flags = 0;
    }

    /* Disable the retransmission timer. */
    port->rtx_timeout_enable = FALSE;

    /* Update the retransmission timer. */
    tcp_timeout_update(port);

} /* tcp_rtx_free_all */

/*
 * net_process_tcp
 * @buffer: File system buffer needed to be processed.
 * @ihl: IPv4 header length.
 * @iface_addr: Interface IP address on which this packet was received.
 * @src_ip: Source address from the IP header.
 * @dst_ip: Destination address from the IP header.
 * @return: A success status will be returned if TCP packet was successfully
 *  processed.
 *  NET_DST_PRT_UNREACHABLE will be returned if destination port is
 *  unreachable.
 *  NET_INVALID_HDR will be returned if invalid header was parsed.
 *  NET_INVALID_CSUM will be returned if an invalid checksum was received.
 *  FS_BUFFER_NO_SPACE if we have ran out of buffers.
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and caller
 *  don't need to free it,
 *  NET_THRESHOLD will be returned if we cannot pass the buffer to networking
 *  stack as that will cause buffer starvation.
 * This function will process an incoming TCP packet.
 */
int32_t net_process_tcp(FS_BUFFER *buffer, uint32_t ihl, uint32_t iface_addr, uint32_t src_ip, uint32_t dst_ip)
{
    int32_t status = SUCCESS;
    uint16_t csum, flags;
    TCP_PORT_PARAM port_param;
    TCP_PORT *port;
    uint32_t seg_ack, seg_seq;
    uint16_t seg_wnd, seg_len;
    uint8_t resume_task = FALSE, resume_flags = 0, stop_timer = FALSE, invalid_ack = FALSE;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Check if we don't have enough number of bytes in the incoming packet. */
    if ((buffer->total_length - ihl) < TCP_HRD_SIZE)
    {
        /* Return an error to the caller. */
        status = NET_INVALID_HDR;
    }

    if (status == SUCCESS)
    {
        /* Calculate checksum for this TCP packet. */
        status = net_pseudo_csum_calculate(buffer, src_ip, dst_ip, IP_PROTO_TCP, (uint16_t)(buffer->total_length - ihl), ihl, 0, &csum);

        /* If checksum was successfully calculated and we don't have the
         * anticipated checksum. */
        if ((status == SUCCESS) && (csum != 0))
        {
            /* Return an error to the caller. */
            status = NET_INVALID_CSUM;
        }
    }

    if (status == SUCCESS)
    {
        /* Pull the source and destination ports. */
        ASSERT(fs_buffer_pull_offset(buffer, &port_param.socket_address.foreign_port, 2, (ihl + TCP_HRD_SRC_PORT_OFFSET), (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)));
        ASSERT(fs_buffer_pull_offset(buffer, &port_param.socket_address.local_port, 2, (ihl + TCP_HRD_DST_PORT_OFFSET), (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)));

        /* Save the ACK number. */
        ASSERT(fs_buffer_pull_offset(buffer, &seg_ack, 4, (uint32_t)(ihl + TCP_HRD_ACK_NUM_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
        ASSERT(fs_buffer_pull_offset(buffer, &seg_seq, 4, (uint32_t)(ihl + TCP_HRD_SEQ_NUM_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Pull the TCP flags. */
        ASSERT(fs_buffer_pull_offset(buffer, &flags, 2, (ihl + TCP_HRD_FLAGS_OFFSET), (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)));

        /* Pull TCP window size. */
        ASSERT(fs_buffer_pull_offset(buffer, &seg_wnd, 2, (uint32_t)(ihl + TCP_HRD_WND_SIZE_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Calculate segment length. */
        seg_len = (uint16_t)(buffer->total_length - (ihl + (uint32_t)(((flags & TCP_HDR_HDR_LEN_MSK) >> TCP_HDR_HDR_LEN_SHIFT) * 4)));

        /* Release semaphore for the buffer file descriptor. */
        fd_release_lock(buffer->fd);

#ifdef CONFIG_SEMAPHORE
        /* Obtain the global data semaphore. */
        ASSERT(semaphore_obtain(&tcp_data.lock, MAX_WAIT) != SUCCESS);
#else
        /* Lock the scheduler. */
        scheduler_lock();
#endif
        /* Initialize search parameter for this TCP packet. */
        port_param.socket_address.local_ip = dst_ip;
        port_param.socket_address.foreign_ip = src_ip;
        port_param.port = NULL;

        /* Search for a TCP port that can be used to receive this packet. */
        sll_search(&tcp_data.port_list, NULL, &tcp_port_search, &port_param, OFFSETOF(TCP_PORT, next));

        /* Save the resolved port. */
        port = port_param.port;

        /* If we do have a TCP port for this buffer. */
        if (port != NULL)
        {
            /* Obtain lock for this TCP port. */
            status = fd_get_lock((FD)port);
        }

#ifndef CONFIG_SEMAPHORE
        /* Enable scheduling. */
        scheduler_unlock();
#else
        /* Release the global semaphore. */
        semaphore_release(&tcp_data.lock);
#endif

        /* If we do have a port to process this TCP packet. */
        if (port != NULL)
        {
            /* If we have successfully acquired lock for the intended TCP
             * port. */
            if (status == SUCCESS)
            {
                /* Save the send window size. */
                port->snd_wnd = ((uint32_t)seg_wnd << port->snd_wnd_scale);

                SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_DEBUG, "SND_WND: %ld, SND_WND_SCALE: %d", port->snd_wnd, port->snd_wnd_scale);

                /* Process this packet according to the current state of the TCP
                 * socket. */
                switch (port->state)
                {
                /* If TCP socket is in listen state. */
                case TCP_SOCK_LISTEN:

                    /* If RST flag is set. */
                    if (flags & TCP_HDR_FLAG_RST)
                    {
                        /* Nothing to do here. */
                        ;
                    }

                    /* If ACK flag is set. */
                    else if (flags & TCP_HDR_FLAG_ACK)
                    {
                        /* Send a RST in response. */
                        /* Segment (SEQ=SEG.ACK, CTL=RST) */
                        tcp_send_segment(port, &port_param.socket_address, seg_ack, 0, (TCP_HDR_FLAG_RST), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);
                    }

                    /* A connection request is identified by a SYN request. */
                    else if (flags & TCP_HDR_FLAG_SYN)
                    {
                        /* If system is not deprived to buffers. */
                        if (fs_buffer_threshold_locked(buffer->fd) == FALSE)
                        {
                            /* Add this buffer in the buffer list for TCP port. */
                            sll_append(&port->buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

                            /* Buffer was passed to the port, return status to
                             * the caller. */
                            status = NET_BUFFER_CONSUMED;

                            /* Resume any tasks waiting to accept new
                             * connections on this port. */
                            resume_task = TRUE;
                            resume_flags = (FS_BLOCK_READ);
                        }
                        else
                        {
                            /* Threshold buffers are now being used, it is
                             * batter to drop this request. */
                            status = NET_THRESHOLD;
                        }
                    }

                    break;

                /* If we have sent a SYN. */
                case TCP_SOCK_SYN_SENT:

                    /* SEG.ACK is on? */
                    if (flags & TCP_HDR_FLAG_ACK)
                    {
                        /* Is ACK acceptable. */
                        /* (SEG.ACK >= SND.UNA) AND (SEG.ACK <= SND.NXT) ? */
                        if ((INT32CMP(seg_ack, port->snd_una) >= 0) && (INT32CMP(seg_ack, port->snd_nxt) <= 0))
                        {
                            /* If RST is on. */
                            if (flags & TCP_HDR_FLAG_RST)
                            {
                                /* Move to the closed state. */
                                port->state = TCP_SOCK_COLSED;

                                /* Stop the TCP timer.  */
                                stop_timer = TRUE;
                                resume_task = TRUE;
                                resume_flags = (FS_BLOCK_READ);
                            }

                            else
                            {
                                /* If SYN is set. */
                                if (flags & TCP_HDR_FLAG_SYN)
                                {
                                    /* If our SYN was ACKed. */
                                    if (seg_ack == port->snd_nxt)
                                    {
                                        /* Clear the TCP port option flag for window scale. */
                                        port->flags &= (TCP_FLAG_WND_SCALE);

                                        /* Process received TCP options. */
                                        status = tcp_process_options(buffer, port, (uint32_t)(ihl + TCP_HRD_SIZE), (uint16_t)(((flags & TCP_HDR_HDR_LEN_MSK) >> (TCP_HDR_HDR_LEN_SHIFT - 2)) - TCP_HRD_SIZE));

                                        /* If TCP options were successfully parsed. */
                                        if (status == SUCCESS)
                                        {
                                            /* If remote did not sent a window scale option. */
                                            if ((port->flags & TCP_FLAG_WND_SCALE) == 0)
                                            {
                                                /* Don't use window scale for both receive and send. */
                                                port->snd_wnd_scale = port->rcv_wnd_scale = 0;
                                            }

                                            /* RCV.NXT := SEG.SEQ + 1 */
                                            port->rcv_nxt = seg_seq + 1;

                                            /* SND.UNA := SEG.ACK */
                                            port->snd_una = seg_ack;

                                            /* Reset the duplicate ACK counter. */
                                            port->nacks = 0;

                                            /* Segment (SEQ=SND.NXT, ACK=RCV.NXT, CTL=ACK) */
                                            tcp_send_segment(port, &port->socket_address, port->snd_nxt, port->rcv_nxt, (TCP_HDR_FLAG_ACK), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);

                                            /* Move to the established state. */
                                            port->state = TCP_SOCK_ESTAB;

                                            /* If we have some space in send window. */
                                            if (port->snd_wnd > 0)
                                            {
                                                /* Resume any tasks waiting to write on
                                                 * this TCP port. */
                                                resume_flags = FS_BLOCK_WRITE;
                                            }
                                        }
                                        else
                                        {
                                            /* Send a RST in response. */
                                            /* Segment (SEQ=SEG.ACK, CTL=RST) */
                                            tcp_send_segment(port, &port_param.socket_address, seg_ack, 0, (TCP_HDR_FLAG_RST), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);

                                            /* Move to the closed state. */
                                            port->state = TCP_SOCK_COLSED;
                                        }

                                        /* Stop the TCP timer.  */
                                        stop_timer = TRUE;
                                        resume_task = TRUE;
                                        resume_flags |= (FS_BLOCK_READ);
                                    }
                                }
                            }
                        }

                        /* ACK is not acceptable. */
                        else
                        {
                            /* Send a RST in response. */
                            /* Segment (SEQ=SND.NXT, CTL=RST) */
                            tcp_send_segment(port, &port->socket_address, port->snd_nxt, 0, (TCP_HDR_FLAG_RST), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);
                        }

                    }

                    /* SEG.RST is on? */
                    else if (flags & TCP_HDR_FLAG_RST)
                    {
                        ;
                    }

                    break;

                /* If SYN was received and we have also sent a SYN in response and
                 * we were waiting for an ACK. */
                case TCP_SOCK_SYN_RCVD:

                /* If connection is established. */
                case TCP_SOCK_ESTAB:

                /* If we are waiting for last ACK. */
                case TCP_SOCK_LAST_ACK:

                /* If we are in FIN-WAIT-1 state. */
                case TCP_SOCK_FIN_WAIT_1:

                /* If we are in FIN-WAIT-2 state. */
                case TCP_SOCK_FIN_WAIT_2:

                /* If we are in closing state. */
                case TCP_SOCK_CLOSING:

                /* If we are in time wait state. */
                case TCP_SOCK_TIME_WAIT:

                    /* Verify received sequence number. */
                    /* Check Segment SEQ (SEG.SEQ, SEG.LEN, RCV.NXT, RCV.WND) */
                    if (tcp_check_sequence(seg_seq, seg_len, port->rcv_nxt, port->rcv_wnd) == TRUE)
                    {
                        /* If RST is set. */
                        if (flags & TCP_HDR_FLAG_RST)
                        {
                            /* TCP_SOCK_SYN_RCVD is only valid for client socket. */

                            /* Move to close state. */
                            port->state = TCP_SOCK_COLSED;

                            /* Stop the TCP timer.  */
                            stop_timer = TRUE;
                            resume_task = TRUE;
                            resume_flags = (FS_BLOCK_READ | FS_BLOCK_WRITE);
                        }

                        /* If SYN is set. */
                        else if (flags & TCP_HDR_FLAG_SYN)
                        {
                            /* Send a RST in response. */
                            /* Segment (SEQ=SND.NXT, CTL=RST) */
                            tcp_send_segment(port, &port->socket_address, port->snd_nxt, 0, (TCP_HDR_FLAG_RST), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);

                            /* Move to closed state. */
                            port->state = TCP_SOCK_COLSED;

                            /* Stop the TCP timer.  */
                            stop_timer = TRUE;
                            resume_task = TRUE;
                            resume_flags = (FS_BLOCK_READ | FS_BLOCK_WRITE);
                        }

                        /* If we have received an ACK from the remote we will move
                         * to established state. */
                        else if (flags & TCP_HDR_FLAG_ACK)
                        {
                            /* Process socket state. */
                            switch (port->state)
                            {

                            /* SR2. */
                            case TCP_SOCK_SYN_RCVD:

                                /* Match the SEG.ACK number. */
                                /* SND.UNA =< SEG.ACK =< SND.NXT ? */
                                if ((INT32CMP(port->snd_una, seg_ack) <= 0) && (INT32CMP(seg_ack, port->snd_nxt) <= 0))
                                {
                                    /* Connection is now established. */
                                    port->state = TCP_SOCK_ESTAB;

                                    /* SND.UNA := SEG.ACK. */
                                    port->snd_una = seg_ack;

                                    /* Reset the duplicate ACK counter. */
                                    port->nacks = 0;

                                    /* Upon receiving a <SYN> segment with a Window Scale
                                     * option containing shift.cnt = S, a TCP MUST set
                                     * Snd.Wind.Shift to S and MUST set Rcv.Wind.Shift
                                     * to R; otherwise, it MUST set both Snd.Wind.Shift
                                     * and Rcv.Wind.Shift to zero.*/

                                    /* If window scale option was received from remote. */
                                    if (port->flags & TCP_FLAG_WND_SCALE)
                                    {
                                        /* Start using window scale as configured during
                                         * connection establishment. */
                                        port->rcv_wnd_scale = TCP_WND_SCALE;
                                    }
                                    else
                                    {
                                        /* Window scale option must be set to 0. */
                                        port->rcv_wnd_scale = port->snd_wnd_scale = 0;
                                    }

                                    /* Stop the TCP timer.  */
                                    stop_timer = TRUE;
                                    resume_task = TRUE;
                                    resume_flags = (FS_BLOCK_READ);

                                    /* If we have some space in send window. */
                                    if (port->snd_wnd > 0)
                                    {
                                        /* Also resume any tasks waiting to write on
                                         * this TCP port. */
                                        resume_flags |= FS_BLOCK_WRITE;
                                    }
                                }

                                /* If connection is terminating. */
                                else if (flags & TCP_HDR_FLAG_FIN)
                                {
                                    /* FIN bit Processing (SND.NXT, RCV.NXT, SEG.SEQ). */
                                    tcp_process_finbit(port, seg_seq);

                                    /* Wait for last ACK from other side state. */
                                    port->state = TCP_SOCK_LAST_ACK;

                                    /* Indicate this status change. */
                                    resume_task = TRUE;
                                    resume_flags = (FS_BLOCK_READ | FS_BLOCK_WRITE);
                                }

                                /* Move to the closed state. */
                                else
                                {
                                    /* Send a RST in response. */
                                    /* Segment (SEQ=SND.NXT, CTL=RST). */
                                    tcp_send_segment(port, &port->socket_address, port->snd_nxt, 0, (TCP_HDR_FLAG_RST), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);

                                    /* Connection was not accepted, move to closed state. */
                                    port->state = TCP_SOCK_COLSED;

                                    /* Stop the TCP timer.  */
                                    stop_timer = TRUE;
                                    resume_task = TRUE;
                                    resume_flags = (FS_BLOCK_READ | FS_BLOCK_WRITE);
                                }

                                break;

                            /* If connection is established. */
                            case TCP_SOCK_ESTAB:

                            /* If we are in FIN-WAIT-1 state. */
                            case TCP_SOCK_FIN_WAIT_1:

                            /* If we are in FIN-WAIT-2 state. */
                            case TCP_SOCK_FIN_WAIT_2:

                                /* Match the SEG.ACK number. */
                                /* SND.UNA < SEG.ACK =< SND.NXT ? */
                                if ((INT32CMP(port->snd_una, seg_ack) < 0) && (INT32CMP(seg_ack, port->snd_nxt) <= 0))
                                {
                                    /* SND.UNA := SEG.ACK. */
                                    port->snd_una = seg_ack;

                                    /* Reset the duplicate ACK counter. */
                                    port->nacks = 0;

                                    /* Process and see if we have more space on this TCP socket. */
                                    if (tcp_rtx_process_ack(port, seg_ack) == TRUE)
                                    {
                                        /* If we have space on the send window. */
                                        if (port->snd_wnd > 0)
                                        {
                                            /* We can now send more data on this port. */
                                            resume_task = TRUE;
                                            resume_flags = (FS_BLOCK_WRITE);
                                        }
                                    }
                                }

                                /* Process according to port state. */
                                switch (port->state)
                                {

                                /* If we are in established state. */
                                case TCP_SOCK_ESTAB:

                                    /* An ACK is received. */
                                    /* SEG.ACK = SND.UNA ? */
                                    if (port->snd_una == seg_ack)
                                    {
                                        /* If we have not received 3 ACKs. */
                                        if (port->nacks < 3)
                                        {
                                            /* dACK := dACK+1. */
                                            port->nacks = (uint8_t)(port->nacks + 1);
                                        }

                                        /* First ACK. */
                                        if (port->nacks == 1)
                                        {
                                            SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_DEBUG, "First ACK", "");
                                        }

                                        /* Second ACK. */
                                        else if (port->nacks == 2)
                                        {
                                            SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_DEBUG, "First duplicate ACK", "");
                                        }

                                        /* Third ACK. */
                                        else if (port->nacks == 3)
                                        {
                                            SYS_LOG_FUNCTION_MSG(TCP, SYS_LOG_DEBUG, "Second duplicate ACK", "");

                                            /* Fart retransmit the segment in the RTX queue. */
                                            /* Segment (SEQ=SEG.ACK, ACK=[?], CTL =[?]) */
                                            tcp_fast_rtx(port, seg_ack);
                                        }
                                    }
                                    else
                                    {
                                        /* Invalid TCP ACK, drop this packet. */
                                        invalid_ack = TRUE;
                                    }

                                    break;
                                }

                                /* If we have not got an invalid TCP ACK. */
                                if (invalid_ack != TRUE)
                                {
                                    /* If we did actually receive a segment. */
                                    if (seg_len > 0)
                                    {
                                        /* Merge the received buffer with rest of the received buffers. */
                                        status = tcp_rx_buffer_merge(port, buffer, seg_len, seg_seq);
                                    }

                                    /* Process according to port state. */
                                    switch (port->state)
                                    {

                                    /* If we are in established state. */
                                    case TCP_SOCK_ESTAB:

                                        /* SEG.FIN is on ? */
                                        if (flags & TCP_HDR_FLAG_FIN)
                                        {
                                            /* FIN bit Processing (SND.NXT, RCV.NXT, SEG.SEQ) */
                                            tcp_process_finbit(port, (seg_seq + seg_len));

                                            /* Move to last ACK. */
                                            port->state = TCP_SOCK_LAST_ACK;

                                            /* Indicate this status change. */
                                            resume_task = TRUE;
                                            resume_flags = (FS_BLOCK_READ | FS_BLOCK_WRITE);
                                        }
                                        break;

                                    /* If we are in FIN-WAIT-1 state. */
                                    case TCP_SOCK_FIN_WAIT_1:

                                        /* If remote has ACKed our FIN segment. */
                                        if (seg_ack == port->snd_nxt)
                                        {
                                            /* SEG.FIN is on? */
                                            if (flags & TCP_HDR_FLAG_FIN)
                                            {
                                                /* FIN bit Processing (SND.NXT, RCV.NXT, SEG.SEQ) */
                                                tcp_process_finbit(port, (seg_seq + seg_len));

                                                /* Enable the event timer. */
                                                port->event_timeout = current_system_tick() + (2 * TCP_MSL);
                                                port->event_timeout_enable = TRUE;
                                                tcp_timeout_update(port);

                                                /* Move to the TIME-WAIT state. */
                                                port->state = TCP_SOCK_TIME_WAIT;
                                            }
                                            else
                                            {
                                                /* Move to the FIN-WAIT-2 state. */
                                                port->state = TCP_SOCK_FIN_WAIT_2;
                                            }
                                        }
                                        else
                                        {
                                            /* SEG.FIN is on? */
                                            if (flags & TCP_HDR_FLAG_FIN)
                                            {
                                                /* FIN bit Processing (SND.NXT, RCV.NXT, SEG.SEQ) */
                                                tcp_process_finbit(port, (seg_seq + seg_len));

                                                /* Move to the closing state. */
                                                port->state = TCP_SOCK_CLOSING;
                                            }
                                        }

                                        break;

                                    /* If we are in FIN-WAIT-2 state. */
                                    case TCP_SOCK_FIN_WAIT_2:

                                        /* SEG.FIN is on? */
                                        if (flags & TCP_HDR_FLAG_FIN)
                                        {
                                            /* FIN bit Processing (SND.NXT, RCV.NXT, SEG.SEQ) */
                                            tcp_process_finbit(port, (seg_seq + seg_len));

                                            /* Enable the event timer. */
                                            port->event_timeout = current_system_tick() + (2 * TCP_MSL);
                                            port->event_timeout_enable = TRUE;
                                            tcp_timeout_update(port);

                                            /* Move to the TIME-WAIT state. */
                                            port->state = TCP_SOCK_TIME_WAIT;
                                        }

                                        break;
                                    }
                                }

                                break;

                            /* If we are waiting for last ACK. */
                            case TCP_SOCK_LAST_ACK:

                            /* If we are in closing state. */
                            case TCP_SOCK_CLOSING:

                            /* If we are in time wait state. */
                            case TCP_SOCK_TIME_WAIT:

                                /* Match the SEG.ACK number. */
                                /* SND.UNA < SEG.ACK =< SND.NXT ? */
                                if ((INT32CMP(port->snd_una, seg_ack) < 0) && (INT32CMP(seg_ack, port->snd_nxt) <= 0))
                                {
                                    /* Our FIN has been ACKed? */
                                    if (seg_ack == port->snd_nxt)
                                    {
                                        switch (port->state)
                                        {
                                        /* If we are waiting for last ACK. */
                                        case TCP_SOCK_LAST_ACK:

                                            /* Move to the closed state. */
                                            port->state = TCP_SOCK_COLSED;

                                            /* Stop the TCP timer.  */
                                            stop_timer = TRUE;
                                            resume_task = TRUE;
                                            resume_flags = (FS_BLOCK_READ | FS_BLOCK_WRITE);

                                            break;

                                        /* If we are in closing state. */
                                        case TCP_SOCK_CLOSING:

                                        /* If we are in time wait state. */
                                        case TCP_SOCK_TIME_WAIT:

                                            /* Enable the event timer. */
                                            port->event_timeout = current_system_tick() + (2 * TCP_MSL);
                                            port->event_timeout_enable = TRUE;
                                            tcp_timeout_update(port);

                                            /* Move to the TIME-WAIT state. */
                                            port->state = TCP_SOCK_TIME_WAIT;

                                            break;
                                        }
                                    }
                                }

                                break;
                            }
                        }
                    }
                    else
                    {
                        /* If RST is not set. */
                        if ((flags & TCP_HDR_FLAG_RST) == 0)
                        {
                            /* Send an ACK. */
                            /* Segment (SEQ=SND.NXT, ACK=RCV.NXT, CTL=ACK). */
                            tcp_send_segment(port, &port->socket_address, port->snd_nxt, port->rcv_nxt, (TCP_HDR_FLAG_ACK), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, FALSE, FS_BUFFER_TH);
                        }
                    }

                    break;
                }

                /* If we need to stop the event timer. */
                if (stop_timer == TRUE)
                {
                    /* Stop any event timer on the port. */
                    port->event_timeout = MAX_WAIT;
                    port->event_timeout_enable = FALSE;

                    /* Update TCP timeout. */
                    tcp_timeout_update(port);
                }

                /* if we need to indicate status change. */
                if (resume_task == TRUE)
                {
                    /* Resume tasks for the calculated condition. */
                    tcp_resume_socket(port, resume_flags);
                }

                /* Release lock for this TCP port. */
                fd_release_lock((FD)port);
            }
        }

        /* If this packet was intended for us. */
        else if (iface_addr == dst_ip)
        {
            /* Destination port is not reachable. */
            status = NET_DST_PRT_UNREACHABLE;
        }

        /* Obtain lock for buffer file descriptor. */
        ASSERT(fd_get_lock(buffer->fd) != SUCCESS);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, (status == NET_BUFFER_CONSUMED) ? SUCCESS : status);

    /* Return status to the caller. */
    return (status);

} /* net_process_tcp */

/*
 * tcp_header_add
 * @buffer: File system buffer on which we need to add a TCP header.
 * @socket_address: Socket address needed to be added.
 * @seq_num: Sequence number to be sent for this TCP packet.
 * @ack_num: Acknowledge a remote sequence number.
 * @tcp_flags: TCP flags needed to be sent.
 * @wnd_size: Window size.
 * @opt_len: Number of bytes of TCP options already added to this TCP packet.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if TCP header was successfully
 *  added.
 * This function will add a TCP header on the given buffer.
 */
int32_t tcp_header_add(FS_BUFFER *buffer, SOCKET_ADDRESS *socket_address, uint32_t seq_num, uint32_t ack_num, uint16_t tcp_flags, uint16_t wnd_size, uint32_t opt_len, uint8_t flags)
{
    int32_t status = SUCCESS;
    HDR_GEN_MACHINE hdr_machine;
    HEADER tcp_hdr[] =
    {
        {&socket_address->local_port,       2, (FS_BUFFER_PACKED | flags) },    /* Source port. */
        {&socket_address->foreign_port,     2, (FS_BUFFER_PACKED | flags) },    /* Destination port. */
        {&seq_num,                          4, (FS_BUFFER_PACKED | flags) },    /* Sequence number. */
        {&ack_num,                          4, (FS_BUFFER_PACKED | flags) },    /* Acknowledgment number. */
        {&tcp_flags,                        2, (FS_BUFFER_PACKED | flags) },    /* Data offset, TCP flags. */
        {&wnd_size,                         2, (FS_BUFFER_PACKED | flags) },    /* Window size. */
        {(uint32_t []){ 0x0 },              4, (flags) },                       /* Checksum, urgent pointer. */
    };

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Add TCP header header length. */
    tcp_flags = (uint16_t)(tcp_flags | (((opt_len + TCP_HRD_SIZE) << (TCP_HDR_HDR_LEN_SHIFT - 2)) & TCP_HDR_HDR_LEN_MSK));

    /* Initialize header generator machine. */
    header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

    /* Push the TCP header on the buffer. */
    status = header_generate(&hdr_machine, tcp_hdr, sizeof(tcp_hdr)/sizeof(HEADER), buffer);

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_header_add */

/*
 * tcp_listen
 * @port: Port on which we need to to listen.
 * @return: A success status will be returned if TCP port was successfully
 *  configured to listen.
 * This function will configure a TCP port to listen for incoming connections.
 */
int32_t tcp_listen(TCP_PORT *port)
{
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Obtain lock for this TCP port. */
    status = fd_get_lock((FD)port);

    if (status == SUCCESS)
    {
        /* Put this socket in the listen state. */
        port->state = TCP_SOCK_LISTEN;

        /* Release lock for this TCP port. */
        fd_release_lock((FD)port);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_listen */

/*
 * tcp_connect
 * @port: TCP port for which we need to establish connect.
 * @return: A success status will be returned if TCP port was successfully
 *  connected to the remote.
 *  NET_NO_ACTION will be returned if TCP socket state is not closed.
 * This function will try to establish a TCP connection with the foreign TCP
 * address.
 */
int32_t tcp_connect(TCP_PORT *port)
{
    int32_t status = SUCCESS;
    uint32_t iss;
    NET_DEV *net_device;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Obtain lock for this port. */
    status = fd_get_lock((FD)port);

    if (status == SUCCESS)
    {
        /* Get the local networking interface descriptor. */
        net_device = ipv4_get_source_device(port->socket_address.local_ip);

        /* If we have a valid networking device and we are in closed state. */
        if ((net_device != NULL) && (port->state == TCP_SOCK_COLSED))
        {
            /* Select a new ISS. */
            iss = (tcp_iss + 1);

            /* SND.UNA = ISS. */
            port->snd_una = iss;

            /* SND.NXT = ISS + 1. */
            port->snd_nxt = iss + 1;

            /* Initialize TCP max segment size for this socket. */
            port->mss = (uint16_t)MIN((net_device_get_mtu(net_device->fd) - (IPV4_HDR_SIZE + TCP_HRD_SIZE)), TCP_WND_SIZE);

            /* Initialize TCP port configuration. */
            tcp_port_initialize(port);

            /* Send SYN-ACK in response with our TCP options. */
            /* Segment (SEQ=ISS, CTL=SYN) */
            status = tcp_send_segment(port, &port->socket_address, iss, 0, (TCP_HDR_FLAG_SYN), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, TRUE, (FS_BUFFER_TH | FS_BUFFER_SUSPEND));
        }
        else
        {
            /* Return an error that specified action was not taken. */
            status = NET_NO_ACTION;
        }

        if (status == SUCCESS)
        {
            /* Move to the SYN sent state. */
            port->state = TCP_SOCK_SYN_SENT;

            /* Wait for a state change. */
            while ((status == SUCCESS) && (port->state == TCP_SOCK_SYN_SENT))
            {
                /* Keep waiting until we get out of this state. */
                status = tcp_port_wait(port, FS_BLOCK_READ);
            }

            if (status == SUCCESS)
            {
                /* If connection was not established. */
                if (port->state != TCP_SOCK_ESTAB)
                {
                    /* Connection was refused. */
                    status = NET_REFUSED;
                }
            }
        }

        /* Release lock for this port. */
        fd_release_lock((FD)port);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_connect */

/*
 * tcp_accept
 * @server_port: Server port for which a connection is needed to be accepted.
 * @client_port: Port which will be used to accept the new connection. Caller
 *  is responsible for registering this port.
 * @return: A success status will be returned if a TCP connection was
 *  successfully accepted.
 *  NET_NO_ACTION will be returned if we got forcefully resumed.
 *  NET_CLOSED will be returned if server socket was closed while accepting
 *  this connection.
 * This function will accept a TCP connection on the given TCP server port,
 * socket address of new port is updated for the accepted connection.
 */
int32_t tcp_accept(TCP_PORT *server_port, TCP_PORT *client_port)
{
    int32_t status = SUCCESS;
    FS_BUFFER *buffer;
    uint32_t irs, iss;
    uint16_t flags;
    uint8_t ihl;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Obtain lock for server port. */
    status = fd_get_lock((FD)server_port);

    /* If lock for the TCP port was successfully obtained. */
    if (status == SUCCESS)
    {
        /* Wait for data on the server socket. */
        status = tcp_port_wait(server_port, FS_BLOCK_READ);

        /* If a connection request was received on this port. */
        if (status == SUCCESS)
        {
            /* Get the request buffer from the port buffer list. */
            buffer = sll_pop(&server_port->buffer_list, OFFSETOF(FS_BUFFER, next));

            /* If we don't have any more buffers to read from this port. */
            if (server_port->buffer_list.head == NULL)
            {
                /* No more buffers on this port to read. */
                fd_data_flushed((FD)server_port);
            }

            /* If we do have a buffer to process the connection request. */
            if (buffer != NULL)
            {
                /* Clear the next buffer pointer. */
                buffer->next = NULL;

                /* If we are accepting client connection in a separate port. */
                if (server_port != client_port)
                {
                    /* Obtain lock for the client TCP port that we will be using to
                     * further process this client. */
                    status = fd_get_lock((FD)client_port);
                }

                if (status == SUCCESS)
                {
                    /* Obtain lock for buffer file descriptor. */
                    ASSERT(fd_get_lock(buffer->fd));

                    /* Get value for IHL and TCP header flags. */
                    tcp_buffer_get_ihl_flags(buffer, &ihl, &flags);

                    /* Initialize TCP max segment size for the client socket. */
                    client_port->mss = (uint16_t)MIN((net_device_get_mtu(buffer->fd) - (IPV4_HDR_SIZE + TCP_HRD_SIZE)), TCP_WND_SIZE);

                    /* Initialize TCP port configuration. */
                    tcp_port_initialize(client_port);

                    /* Clear the TCP port flags for the option we received and
                     * will also be sent. */
                    client_port->flags &= (TCP_FLAG_WND_SCALE | TCP_FLAG_MSS);

                    /* Process TCP options. */
                    status = tcp_process_options(buffer, client_port, (uint32_t)(ihl + TCP_HRD_SIZE), (uint16_t)(((flags & TCP_HDR_HDR_LEN_MSK) >> (TCP_HDR_HDR_LEN_SHIFT - 2)) - TCP_HRD_SIZE));

                    /* If TCP options were successfully processed. */
                    if (status == SUCCESS)
                    {
                        /* Save the socket address for this connection request. */

                        /* Save the IP addresses for this socket. */
                        ASSERT(fs_buffer_pull_offset(buffer, &client_port->socket_address.foreign_ip, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
                        ASSERT(fs_buffer_pull_offset(buffer, &client_port->socket_address.local_ip, 4, IPV4_HDR_DST_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

                        /* Save the port addresses for this socket. */
                        ASSERT(fs_buffer_pull_offset(buffer, &client_port->socket_address.foreign_port, 2, (uint32_t)(ihl + TCP_HRD_SRC_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
                        ASSERT(fs_buffer_pull_offset(buffer, &client_port->socket_address.local_port, 2, (uint32_t)(ihl + TCP_HRD_DST_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

                        /* Save the remote sequence number. */
                        /* Set IRS = SEG.SEQ. */
                        ASSERT(fs_buffer_pull_offset(buffer, &irs, 4, (uint32_t)(ihl + TCP_HRD_SEQ_NUM_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

                        /* Set RCV.NXT = SEG.SEQ + 1. */
                        client_port->rcv_nxt = (uint32_t)(irs + 1);

                        /* Generate a ISS. */
                        iss = (tcp_iss + 1);

                        /* Segment will be sent in TCP event. */

                        /* SND.NXT = ISS + 1. */
                        client_port->snd_nxt = iss + 1;

                        /* SND.UNA = ISS. */
                        client_port->snd_una = iss;

                        /* A SYN was received. */

                        /* Update the port state. */
                        client_port->state = TCP_SOCK_SYN_RCVD;

                        /* Save the window size and scale. */
                        client_port->snd_wnd = server_port->snd_wnd;
                        client_port->snd_wnd_scale = server_port->snd_wnd_scale;

                        /* Release lock for buffer file descriptor. */
                        fd_release_lock(buffer->fd);

                        /* Send SYN-ACK in response with our TCP options. */
                        /* Segment (SEQ=ISS, ACK=RCV.NXT, CTL=SYN,ACK) */
                        tcp_send_segment(client_port, &client_port->socket_address, iss, client_port->rcv_nxt, (TCP_HDR_FLAG_ACK | TCP_HDR_FLAG_SYN), (uint16_t)(client_port->rcv_wnd >> client_port->rcv_wnd_scale), NULL, 0, TRUE, (FS_BUFFER_TH | FS_BUFFER_SUSPEND));

                        /* Obtain lock for buffer file descriptor. */
                        ASSERT(fd_get_lock(buffer->fd));
                    }

                    /* If we are accepting client connection in a separate port. */
                    if (server_port != client_port)
                    {
                        /* Release lock for client port. */
                        fd_release_lock((FD)client_port);
                    }
                }

                /* Add the received buffer back to the descriptor. */
                fs_buffer_add_buffer_list(buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

                /* Release lock for buffer file descriptor. */
                fd_release_lock(buffer->fd);
            }
            else
            {
                /* We resumed as the socket was closing. */
                if (server_port->state == TCP_SOCK_COLSED)
                {
                    /* Return error that socket is closing. */
                    status = NET_CLOSED;
                }
                else
                {
                    /* We did resume but we don't have a connection request. */
                    status = NET_NO_ACTION;
                }
            }
        }

        /* Release lock for server port. */
        fd_release_lock((FD)server_port);
    }

    /* If SYN was processed and transmission of SYN-ACK is queued, wait for a
     * reply from other end. */
    if (status == SUCCESS)
    {
        /* Obtain lock for client port. */
        status = fd_get_lock((FD)client_port);

        if (status == SUCCESS)
        {
            /* Wait for data on the client socket. */
            status = tcp_port_wait(server_port, FS_BLOCK_READ);

            /* If a reply was received. */
            if (status == SUCCESS)
            {
                /* If we don't have any more buffers to read from this port. */
                if (client_port->buffer_list.head == NULL)
                {
                    /* No more buffers on this port to read. */
                    fd_data_flushed((FD)client_port);
                }

                /* Check if connection was successful. */
                if (client_port->state == TCP_SOCK_ESTAB)
                {
                    /* If we have some space available to write data on this port. */
                    if (client_port->snd_wnd > 0)
                    {
                        /* There is space available to write data on this port. */
                        fd_space_available((FD)client_port);
                    }
                }
                else
                {
                    /* Return error that connection was refused. */
                    status = NET_REFUSED;
                }
            }

            /* Release lock for client port. */
            fd_release_lock((FD)client_port);
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

    /* Return status to the caller. */
    return (status);

} /* tcp_accept */

/*
 * tcp_close
 * @port: TCP port needed to be closed.
 * This function will close the given TCP port.
 */
void tcp_close(TCP_PORT *port)
{
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(TCP);

    /* Obtain lock for this port. */
    if (fd_get_lock((FD)port) == SUCCESS)
    {
        /* Process the close call according to the state of this port. */
        switch (port->state)
        {
        /* If we are at listen state. */
        case TCP_SOCK_LISTEN:

        /* If we have sent a SYN. */
        case TCP_SOCK_SYN_SENT:

            /* Move to the closed state. */
            port->state = TCP_SOCK_COLSED;

            break;

        /* If we have received SYN. */
        case TCP_SOCK_SYN_RCVD:

        /* If we are at established state. */
        case TCP_SOCK_ESTAB:

            /* Queue this CLOSE request until all queued SENDs have been
             * segmentized and sent and ACKed. */
            status = tcp_port_wait(port, FS_BLOCK_WRITE);

            if (status == SUCCESS)
            {
                /* Segment (SEQ=SND.NXT, ACK=RCV.NXT, CTL=FIN,ACK) */
                status = tcp_send_segment(port, &port->socket_address, port->snd_nxt, port->rcv_nxt, (TCP_HDR_FLAG_FIN | TCP_HDR_FLAG_ACK), (uint16_t)(port->rcv_wnd >> port->rcv_wnd_scale), NULL, 0, TRUE, (FS_BUFFER_TH | FS_BUFFER_SUSPEND));
            }

            if (status == SUCCESS)
            {
                /* We have sent a FIN. */
                port->snd_nxt = port->snd_nxt + 1;

                /* Move to FIN wait state. */
                port->state = TCP_SOCK_FIN_WAIT_1;
            }

            break;
        }

        /* Wait for this port to go into closed state. */
        while ((status == SUCCESS) && (port->state != TCP_SOCK_COLSED))
        {
            /* Wait for data on this port. */
            status = tcp_port_wait(port, FS_BLOCK_READ);

            if (status == SUCCESS)
            {
                /* Flush the port state. */
                fd_data_flushed(port);
            }
        }

        /* If we have successfully  moved to the closed state. */
        if (status == SUCCESS)
        {
            /* Free all the retransmission buffers. */
            tcp_rtx_free_all(port);

            /* Resume any tasks waiting for state change for this port. */
            tcp_resume_socket(port, (FS_BLOCK_READ | FS_BLOCK_WRITE));
        }

        /* Release lock for TCP port. */
        fd_release_lock((FD)port);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TCP, status);

} /* tcp_close */

#endif /* NET_TCP */
#endif /* CONFIG_NET */
