/*
 * weird_view_server.c
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

#ifdef CONFIG_WEIRD_VIEW
#include <weird_view_server.h>
#include <string.h>

/* Internal function prototypes. */
static void weird_view_server_process(void *);
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

    /* initialize server data. */
    weird_view->device_name = name;
    weird_view->plugin      = plugin;
    weird_view->num_plugin  = num_plugin;

    /* Use buffered mode for this UDP port. */
    weird_view->port.console.fs.flags = FS_BUFFERED;

    /* Register the UDP port. */
    udp_register((FD)&weird_view->port, name, socket_address);

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
    WEIRD_VIEW_PLUGIN   *plugin = NULL;
    uint32_t            i;

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
 * This is callback function to process a request for a wired view server.
 */
static void weird_view_server_process(void *data)
{
    WEIRD_VIEW_SERVER   *weird_view = (WEIRD_VIEW_SERVER *)data;
    WEIRD_VIEW_PLUGIN   *plugin;
    FS_BUFFER           *rx_buffer;
    SOCKET_ADDRESS      socket_address;
    uint32_t            command, i;
    int32_t             received;
    uint16_t            id;

    /* Receive incoming data from the UDP port. */
    received = fs_read(&weird_view->port, (uint8_t *)&rx_buffer, sizeof(FS_BUFFER));

    /* If some data was received. */
    if (received >= (int32_t)sizeof(uint32_t))
    {
        /* Acquire lock for the buffer file descriptor. */
        OS_ASSERT(fd_get_lock(rx_buffer->fd) != SUCCESS);

        /* Pull the command from the buffer. */
        fs_buffer_pull(rx_buffer, &command, sizeof(uint32_t), (FS_BUFFER_HEAD | FS_BUFFER_PACKED));

        /* Process the given command. */
        switch (command)
        {

        /* A discover was requested. */
        case WV_DISC:

            /* If there is noting else to read. */
            if (rx_buffer->total_length == 0)
            {
                /* Send discover reply. */
                OS_ASSERT(fs_buffer_push(rx_buffer, (uint32_t []){ WV_DISC_REPLY }, sizeof(uint32_t), (FS_BUFFER_PACKED)) !=  SUCCESS);

                /* Push the device name. */
                OS_ASSERT(fs_buffer_push(rx_buffer, (uint8_t *)weird_view->device_name, strlen(weird_view->device_name), 0) !=  SUCCESS);
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
                OS_ASSERT(fs_buffer_push(rx_buffer, (uint32_t []){ WV_LIST_REPLY }, sizeof(uint32_t), (FS_BUFFER_PACKED)) !=  SUCCESS);

                /* Push data for all the registered plugins. */
                for (i = 0; i < weird_view->num_plugin; i ++)
                {
                    /* Push plugin ID. */
                    OS_ASSERT(fs_buffer_push(rx_buffer, &weird_view->plugin[i].id, sizeof(uint16_t), (FS_BUFFER_PACKED)) !=  SUCCESS);

                    /* Push plugin type. */
                    OS_ASSERT(fs_buffer_push(rx_buffer, &weird_view->plugin[i].type, sizeof(uint8_t), 0) !=  SUCCESS);

                    /* Push plugin name. */
                    OS_ASSERT(fs_buffer_push(rx_buffer, (uint8_t []){ (uint8_t)strlen(weird_view->plugin[i].name) }, sizeof(uint8_t), 0) !=  SUCCESS);
                    OS_ASSERT(fs_buffer_push(rx_buffer, (uint8_t *)weird_view->plugin[i].name, strlen(weird_view->plugin[i].name), 0) !=  SUCCESS);
                }
            }
            else
            {
                /* Invalid header. */
                received = WV_INAVLID_HRD;
            }

            break;

        /* Update requested for a plugin. */
        case WV_REQ:

            /* If we have only id on the buffer. */
            if (rx_buffer->total_length == (int32_t)sizeof(uint16_t))
            {
                /* Pull the plugin id. */
                fs_buffer_pull(rx_buffer, &id, sizeof(uint16_t), (FS_BUFFER_HEAD | FS_BUFFER_PACKED));

                /* Try to get the required plugin. */
                plugin = weird_view_get_plugin(weird_view, id);

                /* If we do have a plugin. */
                if ((plugin != NULL) && (plugin->data != NULL))
                {
                    /* Process this request according to the plugin type. */
                    switch (plugin->type)
                    {

                    /* Data was requested for log plugin. */
                    case WV_PLUGIN_LOG:

                        /* Fill this buffer with the required log data. */
                        received = ((WV_LOG_DATA *)plugin->data)(id, rx_buffer);

                        break;

                    /* Unknown plugin type. */
                    default:

                        /* No data can be added. */
                        received = WV_NO_DATA;

                        break;
                    }

                    /* If required data was added to the buffer. */
                    if (received >= 0)
                    {
                        /* Send reply for this request. */
                        OS_ASSERT(fs_buffer_push(rx_buffer, (uint32_t []){ WV_REQ_REPLY }, sizeof(uint32_t), (FS_BUFFER_HEAD | FS_BUFFER_PACKED)) !=  SUCCESS);
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
            /* Save the actual socket address. */
            socket_address = weird_view->port.socket_address;

            /* Update the socket address to which we will reply. */
            weird_view->port.socket_address = weird_view->port.last_datagram_address;
            ipv4_get_device_address(rx_buffer->fd, &weird_view->port.socket_address.local_ip);

            /* Send received data back on the UDP port. */
            received = fs_write(&weird_view->port, (uint8_t *)rx_buffer, sizeof(FS_BUFFER));

            /* Restore the actual socket address. */
            weird_view->port.socket_address = socket_address;
        }
    }

} /* weird_view_server_process */

#endif /* CONFIG_WEIRD_VIEW */
