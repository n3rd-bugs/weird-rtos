/*
 * weird_view_server.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_WEIRD_VIEW
#include <weird_view_server.h>
#include <string.h>

/* Internal function prototypes. */
static void weird_view_server_process(void *, int32_t);
static WEIRD_VIEW_PLUGIN *weird_view_get_plugin(WEIRD_VIEW_SERVER *, uint16_t);

/*
 * weird_view_server_init
 * @weird_view: Weird view server data needed to be initialized.
 * @socket_address: Socket address on which we will be serving clients.
 * @name: Device name to be used.
 * @plugin: Weird view plugins needed to be server on this server.
 * @num_plugin: Number of plugins we will be serving.
 * This function will initialize a weird view server instance and start serving
 * the required resources.
 */
void weird_view_server_init(WEIRD_VIEW_SERVER *weird_view, SOCKET_ADDRESS *socket_address, char *name, WEIRD_VIEW_PLUGIN *plugin, uint32_t num_plugin)
{
    /* Clear the given server structure. */
    memset(weird_view, 0, sizeof(WEIRD_VIEW_SERVER));

    /* Initialize server data. */
    weird_view->device_name = name;
    weird_view->plugin      = plugin;
    weird_view->num_plugin  = num_plugin;

    /* Use buffered mode for this UDP port. */
    weird_view->port.console.fs.flags = FS_BUFFERED;

    /* As we will be using net condition to process data on this port so it
     * would not be okay to suspend for buffers. */
    weird_view->port.flags = UDP_FLAG_THR_BUFFERS;

    /* Register the UDP port. */
    udp_register((FD)&weird_view->port, name, socket_address);

    /* Lets never block on this port. */
    weird_view->port.console.fs.flags &= (uint32_t)~(FS_BLOCK);

    /* Get read condition for UDP port. */
    fs_condition_get((FD)&weird_view->port, &weird_view->port_condition, &weird_view->port_suspend, &weird_view->port_fs_param, FS_BLOCK_READ);

    /* Add a networking condition for this UDP port. */
    net_condition_add(weird_view->port_condition, &weird_view->port_suspend, &weird_view_server_process, (void *)weird_view);

} /* weird_view_server_init */

/*
 * weird_view_get_plugin
 * @weird_server: Weird view server data for which a plugin is required.
 * @id: Required plugin id.
 * @return: If not null valid plugin data will be returned.
 * This is callback function to process a request for a wired view server.
 */
static WEIRD_VIEW_PLUGIN *weird_view_get_plugin(WEIRD_VIEW_SERVER *weird_server, uint16_t id)
{
    WEIRD_VIEW_PLUGIN *plugin = NULL;
    uint32_t i;

    /* Search for a required plugin. */
    for (i = 0; i < weird_server->num_plugin; i++)
    {
        /* Check if this is the required plugin. */
        if (weird_server->plugin[i].id == id)
        {
            /* Return this plugin. */
            plugin = &weird_server->plugin[i];

            /* Break out of this loop. */
            break;
        }
    }

    /* Return required plugin to the caller. */
    return (plugin);

} /* weird_view_get_plugin */

/*
 * weird_view_server_process
 * @data: Weird view server data for which a request is needed to be processed.
 * @status: Resumption status.
 * This is callback function to process a request for a wired view server.
 */
static void weird_view_server_process(void *data, int32_t status)
{
    WEIRD_VIEW_SERVER *weird_view = (WEIRD_VIEW_SERVER *)data;
    WEIRD_VIEW_PLUGIN *plugin;
    FS_BUFFER *rx_buffer;
    uint32_t command, i, j, value, value_div, disp_max;
    int32_t received;
    uint16_t id;
    uint8_t state, this_size = 0, str[16];
    P_STR_T name;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(status);

    /* Receive incoming data from the UDP port. */
    received = fs_read(&weird_view->port, (uint8_t *)&rx_buffer, sizeof(FS_BUFFER));

    /* If some data was received. */
    if (received >= (int32_t)sizeof(uint32_t))
    {
        /* Acquire lock for the buffer file descriptor. */
        ASSERT(fd_get_lock(rx_buffer->fd) != SUCCESS);

        /* Pull the command from the buffer. */
        fs_buffer_pull(rx_buffer, &command, sizeof(uint32_t), (FS_BUFFER_PACKED));

        /* Process the given command. */
        switch (command)
        {

        /* A discover was requested. */
        case WV_DISC:

            /* If there is noting else to read. */
            if (rx_buffer->total_length == 0)
            {
                /* Send discover reply. */
                received = fs_buffer_push(rx_buffer, (uint32_t []){ WV_DISC_REPLY }, sizeof(uint32_t), (FS_BUFFER_PACKED));

                if (received == SUCCESS)
                {
                    /* Push the device name. */
                    received = fs_buffer_push(rx_buffer, (uint8_t *)weird_view->device_name, strlen(weird_view->device_name), 0);
                }
            }
            else
            {
                /* Invalid header. */
                received = WV_INAVLID_HRD;
            }

            break;

        /* Need to send list of all the plugins we are serving. */
        case WV_LIST:

            /* If there is noting else to read. */
            if (rx_buffer->total_length == 0)
            {
                /* Send list reply. */
                received = fs_buffer_push(rx_buffer, (uint32_t []){ WV_LIST_REPLY }, sizeof(uint32_t), (FS_BUFFER_PACKED));

                /* Push data for all the registered plugins. */
                for (i = 0; (i < weird_view->num_plugin) && (received == SUCCESS); i ++)
                {
                    /* Push plugin ID. */
                    received = fs_buffer_push(rx_buffer, &weird_view->plugin[i].id, sizeof(uint16_t), (FS_BUFFER_PACKED));

                    if (received == SUCCESS)
                    {
                        /* Push plugin type. */
                        received = fs_buffer_push(rx_buffer, &weird_view->plugin[i].type, sizeof(uint8_t), 0);
                    }

                    /* Push plugin name. */
                    if (received == SUCCESS)
                    {
                        received = fs_buffer_push(rx_buffer, (uint8_t []){ (uint8_t)P_STR_LEN(weird_view->plugin[i].name) }, sizeof(uint8_t), 0);
                    }
                    if (received == SUCCESS)
                    {
                        /* Pick the name to be sent. */
                        name = weird_view->plugin[i].name;

                        for (j = 0; (received == SUCCESS) && (j < P_STR_LEN(weird_view->plugin[i].name)); j += this_size)
                        {
                            /* Pick the number of bytes we need to copy. */
                            this_size = (uint8_t)((P_STR_LEN(name) > sizeof(str)) ? sizeof(str) : P_STR_LEN(name));

                            /* Copy the required bytes. */
                            P_MEM_CPY(str, name, this_size);
                            received = fs_buffer_push(rx_buffer, str, this_size, 0);

                            /* Move name ahead. */
                            name += this_size;
                        }
                    }
                }
            }
            else
            {
                /* Invalid header. */
                received = WV_INAVLID_HRD;
            }

            break;

        /* Update requested for a plugin. */
        case WV_UPDATE:

        /* Need to process a request. */
        case WV_REQ:

            /* If we have at least id on the buffer. */
            if (rx_buffer->total_length >= (int32_t)sizeof(uint16_t))
            {
                /* Pull the plugin id. */
                fs_buffer_pull(rx_buffer, &id, sizeof(uint16_t), (FS_BUFFER_PACKED));

                /* Try to get the required plugin. */
                plugin = weird_view_get_plugin(weird_view, id);

                /* If we do have a plugin. */
                if (plugin != NULL)
                {
                    /* Process the given command. */
                    switch (command)
                    {

                    /* Update requested for a plugin. */
                    case WV_UPDATE:

                        /* If we do have a function registered to fulfill this
                         * request. */
                        if (plugin->data != NULL)
                        {
                            /* If we don't have any more data. */
                            if (rx_buffer->total_length == 0)
                            {
                                /* Process this request according to the plugin type. */
                                switch (plugin->type)
                                {

                                /* Data was requested for log or wave plugin. */
                                case WV_PLUGIN_LOG:
                                case WV_PLUGIN_WAVE:

                                    /* Fill this buffer with the required data. */
                                    received = ((WV_GET_LOG_DATA *)plugin->data)(id, rx_buffer);

                                    break;

                                /* Data was requested for switch plugin. */
                                case WV_PLUGIN_SWITCH:

                                    /* Get current state of the switch. */
                                    received = ((WV_GET_SWITCH_DATA *)plugin->data)(id, &state);

                                    /* If switch state was successfully queried. */
                                    if (received == SUCCESS)
                                    {
                                        /* If switch is active. */
                                        if (state == TRUE)
                                        {
                                            /* Switch is on. */
                                            received = fs_buffer_push(rx_buffer, (uint8_t []){ WV_PLUGIN_SWITCH_ON }, sizeof(uint8_t), 0);
                                        }
                                        else if (state == FALSE)
                                        {
                                            /* Switch is off. */
                                            received = fs_buffer_push(rx_buffer, (uint8_t []){ WV_PLUGIN_SWITCH_OFF }, sizeof(uint8_t), 0);
                                        }
                                        else
                                        {
                                            /* Invalid switch value was given. */
                                            received = WV_NO_DATA;
                                        }
                                    }

                                    break;

                                /* Data was requested for analog plugin. */
                                case WV_PLUGIN_ANALOG:

                                    /* Get data for analog plugin. */
                                    received = ((WV_GET_ANALOG_DATA *)plugin->data)(id, &value, &value_div, &disp_max);

                                    /* If switch state was successfully queried. */
                                    if (received == SUCCESS)
                                    {
                                        /* Push data for analog plugin. */

                                        /* Push analog value. */
                                        received = fs_buffer_push(rx_buffer, &value, sizeof(uint32_t), FS_BUFFER_PACKED);

                                        if (received == SUCCESS)
                                        {
                                            /* Push analog divisor. */
                                            received = fs_buffer_push(rx_buffer, &value_div, sizeof(uint32_t), FS_BUFFER_PACKED);
                                        }

                                        if (received == SUCCESS)
                                        {
                                            /* Push analog maximum value. */
                                            received = fs_buffer_push(rx_buffer, &disp_max, sizeof(uint32_t), FS_BUFFER_PACKED);
                                        }
                                    }

                                    break;

                                /* Unknown plugin type. */
                                default:

                                    /* No data can be added. */
                                    received = WV_NO_DATA;

                                    break;
                                }
                            }
                            else
                            {
                                /* Invalid header. */
                                received = WV_INAVLID_HRD;
                            }
                        }
                        else
                        {
                            /* No reply can be sent for this request. */
                            received = WV_NO_DATA;
                        }

                        break;

                    /* Need to process a request. */
                    case WV_REQ:

                        /* If we do have a function registered to fulfill this
                         * request. */
                        if (plugin->request != NULL)
                        {
                            /* Process this request according to the plugin type. */
                            switch (plugin->type)
                            {
                            /* Data was requested for switch plugin. */
                            case WV_PLUGIN_SWITCH:

                                /* If we have only the new state on the buffer. */
                                if (rx_buffer->total_length == (int32_t)sizeof(uint8_t))
                                {
                                    /* Pull the new state. */
                                    fs_buffer_pull(rx_buffer, &state, sizeof(uint8_t), 0);

                                    /* On requested. */
                                    if (state == WV_PLUGIN_SWITCH_ON)
                                    {
                                        /* Process an ON request. */
                                        ((WV_POC_SWITCH_REQ *)plugin->request)(id, TRUE);
                                    }

                                    /* Off requested. */
                                    else if (state == WV_PLUGIN_SWITCH_OFF)
                                    {
                                        /* Process an OFF request. */
                                        ((WV_POC_SWITCH_REQ *)plugin->request)(id, FALSE);
                                    }
                                }

                                break;
                            }
                        }

                        /* No data is needed to be sent for this. */
                        received = WV_NO_DATA;

                        break;
                    }

                    /* If required data was added to the buffer. */
                    if (received >= 0)
                    {
                        /* Send reply for this request. */
                        received = fs_buffer_push(rx_buffer, (uint32_t []){ WV_UPDATE_REPLY }, sizeof(uint32_t), (FS_BUFFER_HEAD | FS_BUFFER_PACKED));
                    }
                }
                else
                {
                    /* No reply can be sent for this request. */
                    received = WV_NO_DATA;
                }
            }
            else
            {
                /* Invalid header. */
                received = WV_INAVLID_HRD;
            }

            break;

        default:

            /* Unknown command. */
            received = WV_UNKNOWN_CMD;

            break;
        }

        /* If request was not processed. */
        if (received < 0)
        {
            /* Free this buffer. */
            fs_buffer_add(rx_buffer->fd, rx_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Release lock for buffer file descriptor. */
        fd_release_lock(rx_buffer->fd);

        /* If request was successfully processed. */
        if (received >= 0)
        {
            /* Save the address to which we will reply. */
            weird_view->port.destination_address = weird_view->port.last_datagram_address;
            weird_view->port.destination_address.local_ip = IPV4_ADDR_UNSPEC;
            ipv4_get_device_address(rx_buffer->fd, &weird_view->port.destination_address.local_ip, NULL);

            /* Send received data back on the UDP port. */
            received = fs_write(&weird_view->port, (uint8_t *)rx_buffer, sizeof(FS_BUFFER));
        }
    }

} /* weird_view_server_process */

#endif /* CONFIG_WEIRD_VIEW */
