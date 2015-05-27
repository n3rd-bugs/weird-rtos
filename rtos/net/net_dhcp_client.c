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
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

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

/* DHCP client data. */
DHCP_CLIENT_DATA dhcp_client;

/* Internal function prototypes. */
static void dhcp_change_state(DHCP_CLIENT_DEVICE *, uint8_t);
static int32_t net_dhcp_client_build(FD *, FS_BUFFER *, DHCP_CLIENT_DEVICE *, uint8_t);
static void net_dhcp_client_process(void *);
static void dhcp_event(void *);

/*
 * dhcp_change_state
 * @client_data: DHCP client data.
 * This function will change state for a DHCP client, including triggering the
 * state action and initializing any state data.
 */
static void dhcp_change_state(DHCP_CLIENT_DEVICE *client_data, uint8_t state)
{
    /* Update the client state. */
    client_data->state = state;
    client_data->retry = 0;

    /* Process the state change. */
    client_data->current_timeout = (DHCP_BASE_TIMEOUT / 2);

    /* If next state is lease expire state. */
    if (state == DHCP_CLI_LEASE_EXPIRE)
    {
        /* Wait until the lease expire and then trigger this state. */
        client_data->suspend.timeout = (uint32_t)(current_system_tick() + (client_data->lease_time * OS_TICKS_PER_SEC));
    }
    else
    {
        /* Trigger this state now. */
        client_data->suspend.timeout = (uint32_t)current_system_tick();
    }

    /* If we need to start a new transaction in next state. */
    if ((state == DHCP_CLI_DISCOVER) || (state == DHCP_CLI_LEASE_EXPIRE))
    {
        /* Create a new transaction ID. */
        client_data->xid = (uint32_t)(current_system_tick64());
    }

    /* If we are moving to discover state, reinitialize the transaction data. */
    if (state == DHCP_CLI_DISCOVER)
    {
        /* Initialize the DHCP client data. */
        client_data->start_time = (uint16_t)current_system_tick();
        client_data->client_ip = client_data->server_ip = client_data->lease_time = 0;
    }

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

    /* Add DHCP header on the buffer. */
    status = dhcp_add_header(buffer, DHCP_OP_REQUEST, client_data->xid, (uint16_t)(((uint16_t)current_system_tick() - client_data->start_time) / OS_TICKS_PER_SEC), TRUE, ((client_data->state == DHCP_CLI_LEASE_EXPIRE) ? client_data->client_ip : 0x00), 0x00, 0x00, ethernet_get_mac_address(fd));

    if (status == SUCCESS)
    {
        /* Add DHCP message type option with discover type. */
        status = dhcp_add_option(buffer, DHCP_OPT_MSG_TYPE, &dhcp_type, 1, 0);
    }

    if (status == SUCCESS)
    {
        /* Add DHCP message specific options. */
        switch (dhcp_type)
        {
        /* We are requesting a IP lease. */
        case DHCP_MSG_REQUEST:

            /* Add requested IP address option. */
            status = dhcp_add_option(buffer, DHCP_OPT_REQ_IP, &client_data->client_ip, IPV4_ADDR_LEN, FS_BUFFER_PACKED);

            if (status == SUCCESS)
            {
                /* Add server identifier from which we will be acquiring this
                 * lease. */
                status = dhcp_add_option(buffer, DHCP_OPT_SRV_ID, &client_data->server_ip, IPV4_ADDR_LEN, FS_BUFFER_PACKED);
            }

            break;
        }
    }

    if (status == SUCCESS)
    {
        /* End the DHCP option list. */
        status = dhcp_add_option(buffer, DHCP_OPT_END, NULL, 0, 0);
    }

    /* Return status to the caller. */
    return (status);

} /* net_dhcp_client_build */

/*
 * dhcp_event
 * @data: File descriptor associated with the DHCP client.
 * This function will process an event for a DHCP client.
 */
static void dhcp_event(void *data)
{
    FD fd = (FD)data, udp_fd = (FD)&dhcp_client;
    NET_DEV *net_device = net_device_get_fd(fd);
    DHCP_CLIENT_DEVICE *client_data = net_device->ipv4.dhcp_client;
    FS_BUFFER *buffer;

    /* Acquire lock for this file descriptor. */
    OS_ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Get a free buffer from the file descriptor. */
    buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);

    /* If we do have a buffer to send a request. */
    if (buffer != NULL)
    {
        /* If current timeout is less than the maximum timeout. */
        if (client_data->current_timeout < DHCP_MAX_TIMEOUT)
        {
            /* Double the timeout. */
            client_data->current_timeout = (uint16_t)(client_data->current_timeout * 2);
        }

        /* Update the time for which we will be wait for a reply. */
        client_data->suspend.timeout = (uint32_t)(current_system_tick() + client_data->current_timeout);

        /* Process the DHCP event according to current state of the client. */
        switch (net_device->ipv4.dhcp_client->state)
        {
        /* We are still discovering. */
        case DHCP_CLI_DISCOVER:

            /* Build a DHCP discover message. */
            OS_ASSERT(net_dhcp_client_build(fd, buffer, client_data, DHCP_MSG_DICOVER) != SUCCESS);

            break;

        /* We are requesting a new address or an old lease is expired. */
        case DHCP_CLI_REQUEST:
        case DHCP_CLI_LEASE_EXPIRE:

            /* If we have not tried this for maximum number of times. */
            if (client_data->retry < DHCP_MAX_RETRY)
            {
                /* Build a DHCP request message. */
                OS_ASSERT(net_dhcp_client_build(fd, buffer, client_data, DHCP_MSG_REQUEST) != SUCCESS);

                /* We have sent a request. */
                client_data->retry++;
            }
            else
            {
                /* Move back to discover state. */
                dhcp_change_state(client_data, DHCP_CLI_DISCOVER);
            }

            break;
        }

        /* If we have a buffer to send. */
        if (buffer->total_length > 0)
        {
            /* Release lock for this file descriptor. */
            fd_release_lock(fd);

            /* Send this buffer on the UDP client port. */
            fs_write(udp_fd, (uint8_t *)buffer, sizeof(FS_BUFFER));

            /* Acquire lock for this file descriptor. */
            OS_ASSERT(fd_get_lock(fd) != SUCCESS);
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

} /* dhcp_event */

/*
 * net_dhcp_client_process
 * @data: DHCP client data.
 * This function will process data for DHCP client.
 */
static void net_dhcp_client_process(void *data)
{
    uint32_t xid, cli_addr, your_addr, serv_addr, dhcp_serv_addr, lease_time;
    FS_BUFFER *buffer;
    FD udp_fd = (FD)data, net_fd;
    NET_DEV *net_device;
    DHCP_CLIENT_DEVICE *client_data;
    int32_t status;
    uint8_t bootp_op, opt_type, opt_length, dhcp_type, hw_addr[ETH_ADDR_LEN];

    /* While we have some data to read from the client socket. */
    while (fs_read(udp_fd, (uint8_t *)&buffer, sizeof(FS_BUFFER)) > 0)
    {
        /* The networking device file descriptor is same as the one from which
         * this buffer was received. */
        net_fd = buffer->fd;

        /* Acquire lock for networking file descriptor. */
        OS_ASSERT(fd_get_lock(net_fd) != SUCCESS);

        /* Initialize client data. */
        client_data = NULL;

        /* Get the networking device for this packet. */
        net_device = net_device_get_fd(net_fd);

        /* If we have a networking device and DHCP client data for this device. */
        if ((net_device != NULL) && ((client_data = net_device->ipv4.dhcp_client) != NULL))
        {
            /* Parse the DHCP header. */
            status = dhcp_get_header(buffer, &bootp_op, &xid, &cli_addr, &your_addr, &serv_addr, hw_addr);

            /* Verify the transaction ID and hardware addresses match with the
             * DHCP client. */
            if ((xid != client_data->xid) || (memcmp(ethernet_get_mac_address(net_fd), hw_addr, ETH_ADDR_LEN) != 0))
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

                        /* Save the lease time. */
                        client_data->lease_time = lease_time;

                        /* Move client to the request state. */
                        dhcp_change_state(client_data, DHCP_CLI_REQUEST);
                    }

                    break;

                /* If we have sent a request and waiting for an ACK. */
                case DHCP_CLI_REQUEST:
                case DHCP_CLI_LEASE_EXPIRE:

                    /* Process the DHCP message type. */
                    switch (dhcp_type)
                    {
                    /* Got an ACK from the server. */
                    case DHCP_MSG_ACK:

                        /* Save/update the lease time. */
                        client_data->lease_time = lease_time;

                        /* Release lock for networking file descriptor. */
                        fd_release_lock(net_fd);

                        /* We can now use this IP address. */
                        ipv4_set_device_address(net_fd, client_data->client_ip);

                        /* Acquire lock for networking file descriptor. */
                        OS_ASSERT(fd_get_lock(net_fd) != SUCCESS);

                        /* We have a lease, we will try to renew it when it
                         * expires. */
                        dhcp_change_state(client_data, DHCP_CLI_LEASE_EXPIRE);

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
        fs_buffer_add(net_fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

        /* Release lock for networking file descriptor. */
        fd_release_lock(net_fd);
    }

} /* net_dhcp_client_process */

/*
 * net_dhcp_client_initialize
 * This function will initialize the DHCP client.
 */
void net_dhcp_client_initialize()
{
    SOCKET_ADDRESS sock_addr;
    CONDITION *condition;
    FD fd = (FD)&dhcp_client.udp;

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

    /* Don't block on read for this UDP port. */
    dhcp_client.udp.console.fs.flags &= (uint32_t)~(FS_BLOCK);

    /* For networking buffers we will wait for data on networking buffer file descriptor. */
    fs_condition_get(fd, &condition, &dhcp_client.suspend, &dhcp_client.fs_param, FS_BLOCK_READ);

    /* Add networking condition for to process data on this UDP port. */
    net_condition_add(condition, &dhcp_client.suspend, &net_dhcp_client_process, &dhcp_client);

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
    /* Set DHCP client data. */
    net_device->ipv4.dhcp_client = data;

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

    /* If DHCP client data is actually set. */
    if (client_data != NULL)
    {
        /* Initialize condition data. */
        client_data->condition.data = fd;

        /* This will be a timer condition. */
        client_data->suspend.flags = CONDITION_TIMER;

        /* Start from discover state. */
        dhcp_change_state(client_data, DHCP_CLI_DISCOVER);

        /* Add networking condition to process DHCP events for this client. */
        net_condition_add(&client_data->condition, &client_data->suspend, &dhcp_event, fd);
    }

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

    /* If DHCP client data is actually set. */
    if (client_data != NULL)
    {
        /* For now just remove the networking condition for this device. */
        net_condition_remove(&client_data->condition);
    }

} /* net_dhcp_client_stop */

#endif /* DHCP_CLIENT */
#endif /* CONFIG_NET */
