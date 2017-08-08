/*
 * net_dhcp_client.c
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

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_DHCP
#include <net_dhcp.h>
#endif

#ifdef DHCP_CLIENT
#include <string.h>
#include <header.h>
#include <net_udp.h>
#include <ethernet.h>
#include <net_dhcp_client.h>
#include <net_route.h>

/* DHCP client data. */
DHCP_CLIENT_DATA dhcp_client;

/* Internal function prototypes. */
static void dhcp_change_state(DHCP_CLIENT_DEVICE *, uint8_t);
static int32_t net_dhcp_client_build(FD *, FS_BUFFER *, DHCP_CLIENT_DEVICE *, uint8_t);
static void net_dhcp_client_process(void *, int32_t);
static void dhcp_event(void *, int32_t);

/*
 * dhcp_change_state
 * @client_data: DHCP client data.
 * @state: New DHCP state we want to load.
 * This function will change state for a DHCP client, including triggering the
 * state action and initializing any state data.
 */
static void dhcp_change_state(DHCP_CLIENT_DEVICE *client_data, uint8_t state)
{
    uint32_t system_tick = current_system_tick();

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* Update the client state. */
    client_data->state = state;
    client_data->retry = 0;

    /* Process the state change. */
    client_data->current_timeout = (MS_TO_TICK(DHCP_BASE_TIMEOUT) / 2);

    /* Enable DHCP timer. */
    client_data->suspend.timeout_enabled = TRUE;

    /* Times T1 and T2 are configurable by the server through options.
     * T1 defaults to (0.5 * duration_of_lease).
     * T2 defaults to (0.875 * duration_of_lease). */

    /* If next state is lease expire state. */
    if (state == DHCP_CLI_RENEW)
    {
        /* Load T1 to trigger rebind state. */
        client_data->suspend.timeout = (client_data->lease_start + (client_data->lease_time / 2));
    }
    else if (state == DHCP_CLI_REBIND)
    {
        /* Wait until the lease expire and then trigger this state. */
        client_data->suspend.timeout = (client_data->lease_start + ((client_data->lease_time * 7) / 8));
    }
    else
    {
        /* Trigger this state now. */
        client_data->suspend.timeout = system_tick;
    }

    /* We should not wait more than the lease expire. */
    if ((state != DHCP_CLI_DISCOVER) && (INT32CMP(client_data->suspend.timeout, (client_data->lease_start + client_data->lease_time)) > 0))
    {
        /* Lets wait till this lease expires. */
        client_data->suspend.timeout = client_data->lease_start + client_data->lease_time;
    }

    /* If we need to start a new transaction in next state. */
    if ((state == DHCP_CLI_DISCOVER) || (state == DHCP_CLI_RENEW) || (state == DHCP_CLI_REBIND))
    {
        /* Create a new transaction ID. */
        client_data->xid = (uint32_t)(current_hardware_tick());
    }

    /* If we are moving to discover state, reinitialize the transaction data. */
    if (state == DHCP_CLI_DISCOVER)
    {
        /* If we have assigned the IP address. */
        if (client_data->address_assigned == TRUE)
        {
            /* If we had a valid IP address assigned. */
            if (client_data->client_ip != IPV4_ADDR_UNSPEC)
            {
                /* Remove the on-link route. */
                route_remove(client_data->condition.data, IPV4_GATWAY_LL, client_data->client_ip);
            }

            /* If we have a default gateway. */
            if (client_data->gateway_ip != IPV4_ADDR_UNSPEC)
            {
                /* Remove the on-link route. */
                route_remove(client_data->condition.data, client_data->gateway_ip, IPV4_ADDR_UNSPEC);
            }

            /* IP address is no longer assigned. */
            client_data->address_assigned = FALSE;
        }

        /* Initialize the DHCP client data. */
        client_data->start_time = system_tick;
        client_data->client_ip = client_data->server_ip = client_data->gateway_ip = IPV4_ADDR_UNSPEC;
        client_data->lease_time = 0;
    }

    SYS_LOG_FUNCTION_EXIT(DHCPC);

} /* dhcp_change_state */

/*
 * net_dhcp_client_build
 * @fd: File descriptor that will be used to send this request.
 * @buffer: File system buffer that will be populated.
 * @client_data: DHCP client data.
 * @dhcp_type: DHCP request type we are building.
 * @return: A success status will be returned if a DHCP message was
 *  successfully built.
 * This function will build a DHCP client message in the given buffer.
 */
static int32_t net_dhcp_client_build(FD *fd, FS_BUFFER *buffer, DHCP_CLIENT_DEVICE *client_data, uint8_t dhcp_type)
{
    int32_t status;

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* Add DHCP header on the buffer. */
    status = dhcp_add_header(buffer, DHCP_OP_REQUEST, client_data->xid, (uint16_t)((uint32_t)(INT32CMP(current_system_tick(), client_data->start_time)) / SOFT_TICKS_PER_SEC), TRUE, ((client_data->state == DHCP_CLI_RENEW) ? client_data->client_ip : 0x00), 0x00, 0x00, ethernet_get_mac_address(fd));

    if (status == SUCCESS)
    {
        /* Add DHCP message type option with discover type. */
        status = dhcp_add_option(buffer, DHCP_OPT_MSG_TYPE, 1, &dhcp_type, 0);
    }

    if (status == SUCCESS)
    {
        /* Add DHCP message specific options. */
        switch (dhcp_type)
        {
        /* We are requesting a IP lease. */
        case DHCP_MSG_REQUEST:

            /* Add requested IP address option. */
            status = dhcp_add_option(buffer, DHCP_OPT_REQ_IP, IPV4_ADDR_LEN, &client_data->client_ip, FS_BUFFER_PACKED);

            if (status == SUCCESS)
            {
                /* Add server identifier from which we will be acquiring this
                 * lease. */
                status = dhcp_add_option(buffer, DHCP_OPT_SRV_ID, IPV4_ADDR_LEN, &client_data->server_ip, FS_BUFFER_PACKED);
            }

            if (status == SUCCESS)
            {
                /* Add host name option. */
                status = dhcp_add_option(buffer, DHCP_OPT_HOST_NAME, (sizeof(DHCP_CLIENT_HOSTNAME) - 1), DHCP_CLIENT_HOSTNAME, 0);
            }

            break;
        }
    }

    if (status == SUCCESS)
    {
        /* End the DHCP option list. */
        status = dhcp_add_option(buffer, DHCP_OPT_END, 0, NULL, 0);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(DHCPC, status);

    /* Return status to the caller. */
    return (status);

} /* net_dhcp_client_build */

/*
 * dhcp_event
 * @data: File descriptor associated with the DHCP client.
 * @status: Resumption status.
 * This function will process an event for a DHCP client.
 */
static void dhcp_event(void *data, int32_t status)
{
    FD fd = (FD)data, udp_fd = (FD)&dhcp_client;
    uint32_t system_tick = current_system_tick();
    NET_DEV *net_device = net_device_get_fd(fd);
    DHCP_CLIENT_DEVICE *client_data = net_device->ipv4.dhcp_client;
    FS_BUFFER *buffer;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(status);

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* Acquire lock for this file descriptor. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Get a free buffer from the file descriptor. */
    buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);

    /* If we do have a buffer to send a request. */
    if (buffer != NULL)
    {
        /* Our lease is no longer valid. */
        if ((net_device->ipv4.dhcp_client->state != DHCP_CLI_DISCOVER) && (net_device->ipv4.dhcp_client->state != DHCP_CLI_REQUEST) && (INT32CMP(system_tick, (client_data->lease_start + client_data->lease_time)) > 0))
        {
            /* Move to discover state. */
            dhcp_change_state(client_data, DHCP_CLI_DISCOVER);
        }
        else
        {
            /* If current timeout is less than the maximum timeout. */
            if (client_data->current_timeout < MS_TO_TICK(DHCP_MAX_TIMEOUT))
            {
                /* Double the timeout. */
                client_data->current_timeout = (uint16_t)(client_data->current_timeout * 2);
            }

            /* Update the time for which we will be wait for a reply. */
            client_data->suspend.timeout = (system_tick + client_data->current_timeout);

            /* Process the DHCP event according to current state of the client. */
            switch (net_device->ipv4.dhcp_client->state)
            {
            /* We are still discovering. */
            case DHCP_CLI_DISCOVER:

                /* Build a DHCP discover message. */
                net_dhcp_client_build(fd, buffer, client_data, DHCP_MSG_DICOVER);

                break;

            /* We are requesting a new address or trying to renew the lease or
             * it is expired. */
            case DHCP_CLI_REQUEST:
            case DHCP_CLI_RENEW:
            case DHCP_CLI_REBIND:

                /* If we have not tried this for maximum number of times. */
                if (client_data->retry < DHCP_MAX_RETRY)
                {
                    /* Build a DHCP request message. */
                    if (net_dhcp_client_build(fd, buffer, client_data, DHCP_MSG_REQUEST) == SUCCESS)
                    {
                        /* We have sent a request. */
                        client_data->retry++;
                    }
                }
                else
                {
                    /* Load appropriate state. */
                    switch (net_device->ipv4.dhcp_client->state)
                    {

                    /* If we are requesting a new address or we are trying to
                     * renew it. */
                    case DHCP_CLI_REBIND:
                    case DHCP_CLI_REQUEST:

                        /* Move to discover state. */
                        dhcp_change_state(client_data, DHCP_CLI_DISCOVER);

                        break;

                    /* If lease is now expired. */
                    case DHCP_CLI_RENEW:

                        /* Move to rebind state. */
                        dhcp_change_state(client_data, DHCP_CLI_REBIND);

                        break;
                    }
                }

                break;
            }
        }

        /* If we have a buffer to send. */
        if (buffer->total_length > 0)
        {
            /* Release lock for this file descriptor. */
            fd_release_lock(fd);

            /* If we are renewing/re-binding the lease. */
            if (net_device->ipv4.dhcp_client->state == DHCP_CLI_RENEW)
            {
                /* Send the frame to the leasing server. */
                dhcp_client.udp.destination_address.foreign_ip = client_data->server_ip;
            }
            else
            {
                /* Send the frame with broadcast address. */
                dhcp_client.udp.destination_address.foreign_ip = IPV4_ADDR_BCAST;
            }

            /* If we are renewing/re-binding the lease. */
            if ((net_device->ipv4.dhcp_client->state == DHCP_CLI_RENEW) || (net_device->ipv4.dhcp_client->state == DHCP_CLI_REBIND))
            {
                /* Send the frame with leased address. */
                dhcp_client.udp.destination_address.local_ip = client_data->client_ip;
            }
            else
            {
                /* Send the frame with unassigned address. */
                dhcp_client.udp.destination_address.local_ip = IPV4_ADDR_UNSPEC;
            }

            /* Send a DHCP frame. */
            fs_write(udp_fd, (uint8_t *)buffer, sizeof(FS_BUFFER));

            /* Acquire lock for this file descriptor. */
            ASSERT(fd_get_lock(fd) != SUCCESS);
        }

        /* An error has occurred, free this buffer. */
        else
        {
            /* Add the allocated buffer back to the descriptor. */
            fs_buffer_add(fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }
    }

    /* Release lock for this file descriptor. */
    fd_release_lock(fd);

    SYS_LOG_FUNCTION_EXIT(DHCPC);

} /* dhcp_event */

/*
 * net_dhcp_client_process
 * @data: DHCP client data.
 * @resume_status: Resumption status.
 * This function will process data for DHCP client.
 */
static void net_dhcp_client_process(void *data, int32_t resume_status)
{
    uint32_t xid, cli_addr, your_addr, serv_addr, dhcp_serv_addr, lease_time, network = 0;
    FS_BUFFER *buffer;
    FD udp_fd = (FD)data, buffer_fd;
    NET_DEV *net_device;
    DHCP_CLIENT_DEVICE *client_data;
    int32_t status = SUCCESS;
    uint8_t bootp_op, opt_type, opt_length, dhcp_type, hw_addr[ETH_ADDR_LEN];

    /* Remove some compiler warnings. */
    UNUSED_PARAM(resume_status);

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* While we have some data to read from the client socket. */
    while (fs_read(udp_fd, (uint8_t *)&buffer, sizeof(FS_BUFFER)) > 0)
    {
        /* The networking device file descriptor is same as the one from which
         * this buffer was received. */
        buffer_fd = buffer->fd;

        /* Acquire lock for networking file descriptor. */
        ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

        /* Initialize client data. */
        client_data = NULL;

        /* Get the networking device for this packet. */
        net_device = net_device_get_fd(buffer_fd);

        /* If we have a networking device and DHCP client data for this device. */
        if ((net_device != NULL) && ((client_data = net_device->ipv4.dhcp_client) != NULL))
        {
            /* Parse the DHCP header. */
            status = dhcp_get_header(buffer, &bootp_op, &xid, &cli_addr, &your_addr, &serv_addr, hw_addr);

            /* Verify the transaction ID and hardware addresses match with the
             * DHCP client. */
            if ((xid != client_data->xid) || (memcmp(ethernet_get_mac_address(buffer_fd), hw_addr, ETH_ADDR_LEN) != 0))
            {
                /* We are not the intended destination. */
                status = NET_NO_ACTION;
            }

            /* If DHCP header was successfully parsed. */
            while (status == SUCCESS)
            {
                /* Get a DHCP option. */
                status = dhcp_get_option(buffer, &opt_type, &opt_length);

                /* If an option was successfully parsed. */
                if (status == SUCCESS)
                {
                    /* Process the given option type. */
                    switch (opt_type)
                    {
                    /* DHCP message type. */
                    case DHCP_OPT_MSG_TYPE:

                        /* Verify the option length. */
                        if (opt_length == 1)
                        {
                            /* Pull and save the DHCP type. */
                            status = fs_buffer_pull(buffer, &dhcp_type, opt_length, 0);
                        }
                        else
                        {
                            /* Invalid header was parsed. */
                            status = NET_INVALID_HDR;
                        }

                        break;

                    /* Network address. */
                    case DHCP_OPT_NETWORK:

                        /* Verify the option length. */
                        if (opt_length == IPV4_ADDR_LEN)
                        {
                            /* Pull and save the network address. */
                            status = fs_buffer_pull(buffer, &network, opt_length, FS_BUFFER_PACKED);
                        }
                        else
                        {
                            /* Invalid header was parsed. */
                            status = NET_INVALID_HDR;
                        }

                        break;

                    /* Network gateway address. */
                    case DHCP_OPT_GATEWAY:

                        /* Verify the option length. */
                        if (opt_length == IPV4_ADDR_LEN)
                        {
                            /* Pull and save the gateway address. */
                            status = fs_buffer_pull(buffer, &client_data->gateway_ip, opt_length, FS_BUFFER_PACKED);
                        }
                        else
                        {
                            /* Invalid header was parsed. */
                            status = NET_INVALID_HDR;
                        }

                        break;

                    /* DHCP lease time. */
                    case DHCP_OPT_LEASE_TIME:

                        /* Verify the option length. */
                        if (opt_length == 4)
                        {
                            /* Pull and save the lease time. */
                            status = fs_buffer_pull(buffer, &lease_time, opt_length, FS_BUFFER_PACKED);
                        }
                        else
                        {
                            /* Invalid header was parsed. */
                            status = NET_INVALID_HDR;
                        }

                        break;

                    /* DHCP server ID. */
                    case DHCP_OPT_SRV_ID:

                        /* Verify the option length. */
                        if (opt_length == IPV4_ADDR_LEN)
                        {
                            /* Pull and save the server ID (IP address). */
                            status = fs_buffer_pull(buffer, &dhcp_serv_addr, opt_length, FS_BUFFER_PACKED);
                        }
                        else
                        {
                            /* Invalid header was parsed. */
                            status = NET_INVALID_HDR;
                        }

                        break;

                        /* If this is the last option. */
                    case DHCP_OPT_END:

                        /* Break out of the option loop. */
                        status = NET_NO_ACTION;

                        break;

                    /* Option type not handled. */
                    default:

                        /* Pull and discard the option data. */
                        status = fs_buffer_pull(buffer, NULL, opt_length, 0);

                        break;
                    }
                }

                /* If we have parsed all the options. */
                if (status == NET_NO_ACTION)
                {
                    /* Reset the status. */
                    status = SUCCESS;

                    /* Break out of this loop. */
                    break;
                }
            }

            /* If DHCP packet was successfully parsed. */
            if (status == SUCCESS)
            {
                /* Process the received packet according to the client state. */
                switch (client_data->state)
                {

                /* If we are doing discover. */
                case DHCP_CLI_DISCOVER:

                    /* We are waiting for an offer. */
                    if (dhcp_type == DHCP_MSG_OFFER)
                    {
                        /* Save the tentative IP address. */
                        client_data->client_ip = your_addr;

                        /* Save the DHCP server address. */
                        client_data->server_ip = dhcp_serv_addr;

                        /* Move client to the request state. */
                        dhcp_change_state(client_data, DHCP_CLI_REQUEST);
                    }

                    break;

                /* If we have sent a request and waiting for an ACK. */
                case DHCP_CLI_REQUEST:
                case DHCP_CLI_RENEW:
                case DHCP_CLI_REBIND:

                    /* Process the DHCP message type. */
                    switch (dhcp_type)
                    {
                    /* Got an ACK from the server. */
                    case DHCP_MSG_ACK:

                        /* Save the lease start and validity time. */
                        client_data->lease_start = current_system_tick();
                        client_data->lease_time = (lease_time * SOFT_TICKS_PER_SEC);

                        /* Release lock for networking file descriptor. */
                        fd_release_lock(buffer_fd);

                        /* If we have not yet assigned the IP address. */
                        if (client_data->address_assigned == FALSE)
                        {
                            /* We can now use this IP address. */
                            ipv4_set_device_address(buffer_fd, client_data->client_ip, network);

                            /* If we have a default gateway. */
                            if (client_data->gateway_ip != 0)
                            {
                                /* Add a default gateway. */
                                route_add(buffer_fd, client_data->client_ip, client_data->gateway_ip, 0x0, 0x0, 0);
                            }

                            /* IP address is now assigned. */
                            client_data->address_assigned = TRUE;
                        }

                        /* Acquire lock for networking file descriptor. */
                        ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

                        /* We have a lease, we will try to renew it in next state. */
                        dhcp_change_state(client_data, DHCP_CLI_RENEW);

                        break;

                    /* Got a NACK from the server. */
                    case DHCP_MSG_NACK:

                        /* Move back to discover state. */
                        dhcp_change_state(client_data, DHCP_CLI_DISCOVER);

                        break;
                    }

                    break;
                }
            }
        }

        /* Free the received buffer. */
        fs_buffer_add(buffer_fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

        /* Release lock for networking file descriptor. */
        fd_release_lock(buffer_fd);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(DHCPC, status);

} /* net_dhcp_client_process */

/*
 * net_dhcp_client_initialize
 * This function will initialize the DHCP client.
 */
void net_dhcp_client_initialize(void)
{
    SOCKET_ADDRESS sock_addr;
    CONDITION *condition;
    FD fd = (FD)&dhcp_client.udp;

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* DHCP client data. */
    memset(&dhcp_client, 0, sizeof(DHCP_CLIENT_DATA));

    /* Socket address for a DHCP client. */
    sock_addr.foreign_ip = IPV4_ADDR_BCAST;
    sock_addr.foreign_port = DHCP_SRV_PORT;
    sock_addr.local_ip = IPV4_ADDR_UNSPEC;
    sock_addr.local_port = DHCP_CLI_PORT;

    /* This will be a buffered UDP port. */
    dhcp_client.udp.console.fs.flags = FS_BUFFERED;
    dhcp_client.udp.flags = UDP_FLAG_THR_BUFFERS;

    /* Register UDP port for DHCP client. */
    udp_register(fd, "dhcp_client", &sock_addr);

    /* Initialize destination address. */
    dhcp_client.udp.destination_address.foreign_port = DHCP_SRV_PORT;
    dhcp_client.udp.destination_address.local_port = DHCP_CLI_PORT;

    /* Don't block on read for this UDP port. */
    dhcp_client.udp.console.fs.flags &= (uint32_t)~(FS_BLOCK);

    /* For networking buffers we will wait for data on networking buffer file descriptor. */
    fs_condition_get(fd, &condition, &dhcp_client.suspend, &dhcp_client.fs_param, FS_BLOCK_READ);

    /* Add networking condition for to process data on this UDP port. */
    net_condition_add(condition, &dhcp_client.suspend, &net_dhcp_client_process, &dhcp_client);

    SYS_LOG_FUNCTION_EXIT(DHCPC);

} /* net_dhcp_client_initialize */

/*
 * net_dhcp_client_initialize_device
 * @net_device: Networking device for which DHCP client is needed to be
 *  initialized.
 * @data: DHCP client data needed to be assigned to this device.
 * This function will initialize DHCP client for the given device.
 */
void net_dhcp_client_initialize_device(NET_DEV *net_device, DHCP_CLIENT_DEVICE *data)
{
    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* Set DHCP client data. */
    net_device->ipv4.dhcp_client = data;

    /* We are in stopped state. */
    data->state = DHCP_CLI_STOPPED;

    SYS_LOG_FUNCTION_EXIT(DHCPC);

} /* net_dhcp_client_initialize_device */

/*
 * net_dhcp_client_start
 * @net_device: Networking device for which DHCP client is needed to be
 *  initialized.
 * This function will initialize DHCP client operation.
 */
void net_dhcp_client_start(NET_DEV *net_device)
{
    FD fd = net_device->fd;
    DHCP_CLIENT_DEVICE *client_data = net_device->ipv4.dhcp_client;

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* If DHCP client data is actually set. */
    if ((client_data != NULL) && (client_data->state == DHCP_CLI_STOPPED))
    {
        /* Initialize condition data. */
        client_data->condition.data = fd;

        /* Disable timer by default. */
        client_data->suspend.timeout_enabled = FALSE;

        /* Start from discover state. */
        dhcp_change_state(client_data, DHCP_CLI_DISCOVER);

        /* Add networking condition to process DHCP events for this client. */
        net_condition_add(&client_data->condition, &client_data->suspend, &dhcp_event, fd);
    }

    SYS_LOG_FUNCTION_EXIT(DHCPC);

} /* net_dhcp_client_start */

/*
 * net_dhcp_client_stop
 * @net_device: Networking device for which DHCP client is needed to be
 *  stopped.
 * This function will deinitialize DHCP client operation.
 */
void net_dhcp_client_stop(NET_DEV *net_device)
{
    DHCP_CLIENT_DEVICE *client_data = net_device->ipv4.dhcp_client;
    FD *fd = net_device->fd;

    SYS_LOG_FUNCTION_ENTRY(DHCPC);

    /* If DHCP client data is actually set. */
    if ((client_data != NULL) && (client_data->state != DHCP_CLI_STOPPED))
    {
        /* For now just remove the networking condition for this device. */
        net_condition_remove(&client_data->condition);

        /* If we have a valid IP address assigned. */
        if (client_data->client_ip != IPV4_ADDR_UNSPEC)
        {
            /* Remove the on-link route. */
            route_remove(fd, IPV4_GATWAY_LL, client_data->client_ip);
        }

        /* If we have a default gateway. */
        if (client_data->gateway_ip != IPV4_ADDR_UNSPEC)
        {
            /* Remove the on-link route. */
            route_remove(fd, client_data->gateway_ip, IPV4_ADDR_UNSPEC);
        }

        /* We are in stopped state. */
        client_data->state = DHCP_CLI_STOPPED;
    }

    SYS_LOG_FUNCTION_EXIT(DHCPC);

} /* net_dhcp_client_stop */

#endif /* DHCP_CLIENT */
#endif /* CONFIG_NET */
