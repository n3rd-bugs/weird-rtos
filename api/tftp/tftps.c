/*
 * tftps.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

#include <kernel.h>

#ifdef CONFIG_TFTPS
#include <tftps.h>
#include <string.h>

/* Internal function definition. */
static void tftp_server_process(void *, int32_t);

/*
 * tftp_server_init
 * @tftp_server: TFTP server instance to initialize.
 * @socket_address: Socket address at which TFTP server will be listening on.
 * @name: Name of the TFTP server.
 * This function will initialize TFTP server.
 */
void tftp_server_init(TFTP_SERVER *tftp_server, SOCKET_ADDRESS *socket_address, char *name)
{
    SYS_LOG_FUNCTION_ENTRY(TFTPS);

    /* Clear the given server structure. */
    memset(tftp_server, 0, sizeof(TFTP_SERVER));

    /* Use buffered mode for this UDP port. */
    tftp_server->port.console.fs.flags = FS_BUFFERED;

    /* As we will be using net condition to process data on this port so it
     * would not be okay to suspend for buffers. */
    tftp_server->port.flags = UDP_FLAG_THR_BUFFERS;

    /* Register the UDP port. */
    udp_register((FD)&tftp_server->port, name, socket_address);

    /* Get read condition for UDP port. */
    fs_condition_get((FD)&tftp_server->port, &tftp_server->port_condition, &tftp_server->port_suspend, &tftp_server->port_fs_param, FS_BLOCK_READ);

    /* For now disable the timer. */
    tftp_server->port_suspend.timeout_enabled = FALSE;
    tftp_server->port_suspend.priority = NET_SOCKET_PRIORITY;

    /* Lets never block on this socket. */
    tftp_server->port.console.fs.flags &= (uint16_t)~(FS_BLOCK);

    /* Add a networking condition for this UDP port. */
    net_condition_add(tftp_server->port_condition, &tftp_server->port_suspend, &tftp_server_process, (void *)tftp_server);

} /* tftp_server_init */

/*
 * tftp_server_process
 * @data: TFTP server data for which a request is needed to be processed.
 * @resume_status: Resumption status.
 * This is callback function to process a request for a TFTP server.
 */
static void tftp_server_process(void *data, int32_t resume_status)
{
    TFTP_SERVER *tftp_server = (TFTP_SERVER *)data;
    FS_BUFFER *rx_buffer;
    uint32_t data_len;
    int32_t status = SUCCESS, received;
    uint16_t opcode, block;
    uint8_t last_block = FALSE, data_buffer[TFTP_BUFFER_SIZE];
    FD fd;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(resume_status);

    SYS_LOG_FUNCTION_ENTRY(TFTPS);

    /* If the timeout was enabled and it has now occurred. */
    if ((tftp_server->port_suspend.timeout_enabled == TRUE) && (INT32CMP(current_system_tick(), tftp_server->port_suspend.timeout) >= 0))
    {
        /* If we have a connection. */
        if (tftp_server->fd != NULL)
        {
            /* Close the file descriptor. */
            fs_close(&tftp_server->fd);

            SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "file transfered interrupted", "");
        }

        /* Reset the current timeout. */
        tftp_server->port_suspend.timeout = MAX_WAIT;
        tftp_server->port_suspend.timeout_enabled  = FALSE;
    }

    /* Receive incoming data from the UDP port. */
    received = fs_read(&tftp_server->port, (uint8_t *)&rx_buffer, sizeof(FS_BUFFER));

    /* If some data was received. */
    if (received >= (int32_t)sizeof(uint32_t))
    {
        /* Acquire lock for the buffer file descriptor. */
        ASSERT(fd_get_lock(rx_buffer->fd) != SUCCESS);

        /* Pull the opcode from the buffer. */
        status = fs_buffer_pull(rx_buffer, &opcode, sizeof(uint16_t), (FS_BUFFER_PACKED));

        if (status == SUCCESS)
        {
            SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_DEBUG, "received a request 0x%04X", opcode);

            /* Process the TFTP opcode. */
            switch (opcode)
            {
            /* Read request. */
            case TFTP_OP_READ_REQ:

            /* Write request. */
            case TFTP_OP_WRITE_REQ:

                /* If we can establish connection with a new client. */
                if (tftp_server->fd == NULL)
                {
                    /* Pull the file name from the frame. */
                    for (data_len = 0; ((status == SUCCESS) && (data_len < TFTP_BUFFER_SIZE)); data_len++)
                    {
                        /* Pull the filename from the buffer. */
                        status = fs_buffer_pull(rx_buffer, &data_buffer[data_len], sizeof(uint8_t), 0);

                        /* If filename was terminated. */
                        if (data_buffer[data_len] == '\0')
                        {
                            break;
                        }
                    }

                    /* If file name was successfully parsed. */
                    if ((status == SUCCESS) && (data_len < TFTP_BUFFER_SIZE))
                    {
                        /* Release lock for buffer file descriptor. */
                        fd_release_lock(rx_buffer->fd);

                        /* If read was requested. */
                        if (opcode == TFTP_OP_READ_REQ)
                        {
                            SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "read requested for \"%s\"", data_buffer);

                            /* Open the required file. */
                            fd = fs_open((char *)data_buffer, (FS_READ));
                        }
                        else
                        {
                            SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "write requested for \"%s\"", data_buffer);

                            /* Open the required file. */
                            fd = fs_open((char *)data_buffer, (FS_WRITE | FS_CREATE));
                        }

                        /* If file was not found or was not opened. */
                        if (fd == NULL)
                        {
                            /* A file system error was detected. */
                            status = TFTP_ERROR_FS;

                            SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_ERROR, "error opening file \"%s\"", data_buffer);
                        }
                        else
                        {
                            /* Save the opened file descriptor. */
                            tftp_server->fd = fd;

                            /* Save the client address. */
                            memcpy(&tftp_server->client_address, &tftp_server->port.last_datagram_address, sizeof(SOCKET_ADDRESS));

                            /* Initialize session variables. */
                            tftp_server->block_num = 0;
                            tftp_server->tx_block_len = 0;
                            tftp_server->last_block = FALSE;
                        }

                        /* Acquire lock for the buffer file descriptor. */
                        ASSERT(fd_get_lock(rx_buffer->fd) != SUCCESS);
                    }
                    else
                    {
                        /* File name was too long. */
                        status = TFTP_LONG_FILENAME;
                    }
                }
                else
                {
                    /* No more clients can be connected. */
                    status = TFTP_ERROR_EXHAUSTED;
                }

                break;

            /* Data request. */
            case TFTP_OP_DATA:

                /* If we have a open file and this is the client who opened it. */
                if ((tftp_server->fd != NULL) && (memcmp(&tftp_server->client_address, &tftp_server->port.last_datagram_address, sizeof(SOCKET_ADDRESS)) == 0))
                {
                    /* Pull the block number from the buffer. */
                    status = fs_buffer_pull(rx_buffer, &block, sizeof(uint16_t), (FS_BUFFER_PACKED));

                    if (status == SUCCESS)
                    {
                        /* If this is the last block. */
                        if (rx_buffer->total_length != TFTP_BLOCK_SIZE)
                        {
                            /* Mark this as last block. */
                            last_block = TRUE;
                        }

                        /* If client skipped a block. */
                        if (block > (tftp_server->block_num + 1))
                        {
                            /* This is an unknown block. */
                            status = TFTP_OUTOFBOUND_BLOCK;
                        }

                        /* If this is the anticipated block. */
                        else if (block == (tftp_server->block_num + 1))
                        {
                            /* While we have some data to write. */
                            while ((status == SUCCESS) && (rx_buffer->total_length > 0))
                            {
                                /* Pull some data from the buffer. */
                                data_len = ((rx_buffer->total_length > TFTP_BUFFER_SIZE) ? TFTP_BUFFER_SIZE : rx_buffer->total_length);
                                status = fs_buffer_pull(rx_buffer, data_buffer, data_len, 0);

                                if (status == SUCCESS)
                                {
                                    /* Release lock for buffer file descriptor. */
                                    fd_release_lock(rx_buffer->fd);

                                    /* Write a chuck on the file. */
                                    if (fs_write(tftp_server->fd, data_buffer, (int32_t)data_len) <= 0)
                                    {
                                        /* File error. */
                                        status = TFTP_ERROR_FS;
                                    }

                                    /* Acquire lock for the buffer file descriptor. */
                                    ASSERT(fd_get_lock(rx_buffer->fd) != SUCCESS);
                                }
                            }

                            if (status == SUCCESS)
                            {
                                /* Save the current block number. */
                                tftp_server->block_num = block;
                            }
                        }

                        /* In case this is a retransmission, just send an ACK. */

                        /* If this was the last block. */
                        if (last_block == TRUE)
                        {
                            /* Close the file descriptor. */
                            fs_close(&tftp_server->fd);

                            /* Stop the timer. */
                            tftp_server->port_suspend.timeout = MAX_WAIT;
                            tftp_server->port_suspend.timeout_enabled = FALSE;

                            SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "file transfered successfully", "");
                        }
                    }
                }
                else
                {
                    /* Transaction ID is not known. */
                    status = TFTP_UNKNOWN_TID;
                }

                break;

            /* ACK was received. */
            case TFTP_OP_ACK:

                /* If we have a session open for this client. */
                if ((tftp_server->fd != NULL) && (memcmp(&tftp_server->client_address, &tftp_server->port.last_datagram_address, sizeof(SOCKET_ADDRESS)) == 0))
                {
                    /* Pull the block number from the buffer. */
                    status = fs_buffer_pull(rx_buffer, &block, sizeof(uint16_t), (FS_BUFFER_PACKED));

                    if (status == SUCCESS)
                    {
                        /* If other side has ACKed the block we  sent earlier. */
                        if (block == tftp_server->block_num)
                        {
                            /* If we just received ACK for the last block. */
                            if (tftp_server->last_block == TRUE)
                            {
                                /* Close the file descriptor. */
                                fs_close(&tftp_server->fd);

                                /* Stop the timer. */
                                tftp_server->port_suspend.timeout = MAX_WAIT;
                                tftp_server->port_suspend.timeout_enabled = FALSE;

                                SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "file transfered successfully", "");

                                /* Nothing to be sent in reply. */
                                status = TFTP_FRAME_DROP;
                            }
                        }
                        else if (block > tftp_server->block_num)
                        {
                            /* This is an unknown block. */
                            status = TFTP_OUTOFBOUND_BLOCK;
                        }
                        else
                        {
                            /* Lets send the old frame again. */
                            /* TODO seek the file back 1 block. */

                            /* For now send an error. */
                            status = TFTP_ERROR_FS;
                        }
                    }
                }
                else
                {
                    /* Transaction ID is not known. */
                    status = TFTP_UNKNOWN_TID;
                }

                break;

            case TFTP_OP_ERR:

                /* If we have a session open for this client. */
                if ((tftp_server->fd != NULL) && (memcmp(&tftp_server->client_address, &tftp_server->port.last_datagram_address, sizeof(SOCKET_ADDRESS)) == 0))
                {
                    /* Close the file descriptor. */
                    fs_close(&tftp_server->fd);

                    /* Stop the timer. */
                    tftp_server->port_suspend.timeout = MAX_WAIT;
                    tftp_server->port_suspend.timeout_enabled = FALSE;

                    SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "file transfered interrupted", "");
                }

                /* Lets drop this frame. */
                status = TFTP_FRAME_DROP;

                break;

            /* Unknown opcode. */
            default:

                SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_ERROR, "unknown or unsupported opcode 0x%04X", opcode);

                /* This opcode is not supported. */
                status = TFTP_NOT_SUPPORTED;

                break;
            }

            /* Discard any data in the buffer. */
            fs_buffer_pull(rx_buffer, NULL, rx_buffer->total_length, 0);

            /* If we have a session open for this client and we just received some data from it. */
            if ((tftp_server->fd != NULL) && (memcmp(&tftp_server->client_address, &tftp_server->port.last_datagram_address, sizeof(SOCKET_ADDRESS)) == 0))
            {
                /* Reset the timer to terminate this connection. */
                tftp_server->port_suspend.timeout = current_system_tick() + TFTP_CLI_TIMEOUT;
                tftp_server->port_suspend.timeout_enabled  = TRUE;
            }

            /* If request was processed successfully. */
            if (status == SUCCESS)
            {
                /* Process the request. */
                switch (opcode)
                {
                /* We successfully processed a write request. */
                case TFTP_OP_WRITE_REQ:
                case TFTP_OP_DATA:

                    /* Push the acknowledgment opcode. */
                    opcode = TFTP_OP_ACK;

                    break;

                /* We successfully processed a read request. */
                case TFTP_OP_READ_REQ:
                case TFTP_OP_ACK:

                    /* We will send file content in reply. */
                    opcode = TFTP_OP_DATA;

                    /* Send the next block. */
                    tftp_server->block_num++;

                    break;
                }

                /* Push required opcode on the buffer. */
                status = fs_buffer_push_offset(rx_buffer, &opcode, 2, 0, (FS_BUFFER_PACKED | FS_BUFFER_TAIL));

                if (status == SUCCESS)
                {
                    /* Push block we are transmitting or acknowledging. */
                    status = fs_buffer_push_offset(rx_buffer, &tftp_server->block_num, 2, 0, (FS_BUFFER_PACKED | FS_BUFFER_TAIL));
                }

                /* If we need to add data. */
                if (opcode == TFTP_OP_DATA)
                {
                    /* Add data block. */
                    for (data_len = 0; ((status == SUCCESS) && (data_len < TFTP_BLOCK_SIZE));)
                    {
                        /* Calculate the number of bytes we need to read. */
                        status = (int32_t)(((data_len + TFTP_BUFFER_SIZE) > TFTP_BLOCK_SIZE) ? (TFTP_BLOCK_SIZE - data_len) : (TFTP_BUFFER_SIZE));

                        /* Read a chunk of buffer. */
                        status = fs_read(tftp_server->fd, data_buffer, status);

                        /* If we did read some data. */
                        if (status > 0)
                        {
                            /* Update the data length. */
                            data_len += (uint32_t)status;

                            /* Push the read buffer on the buffer. */
                            status = fs_buffer_push(rx_buffer, data_buffer, (uint32_t)status, (FS_BUFFER_TAIL));
                        }
                        else
                        {
                            /* We must be at the end of the file. */
                            break;
                        }
                    }

                    /* Save the number of bytes we sent in this block. */
                    tftp_server->tx_block_len = (uint16_t)data_len;

                    /* If this was the last chunk. */
                    if (data_len != TFTP_BLOCK_SIZE)
                    {
                        /* We will be sending the last block. */
                        tftp_server->last_block = TRUE;
                    }
                }
            }

            /* If we are not dropping this frame. */
            else if (status != TFTP_FRAME_DROP)
            {
                /* If we have a session open for this client. */
                if ((tftp_server->fd != NULL) && (memcmp(&tftp_server->client_address, &tftp_server->port.last_datagram_address, sizeof(SOCKET_ADDRESS)) == 0))
                {
                    /* Close the file descriptor. */
                    fs_close(&tftp_server->fd);

                    /* Stop the timer. */
                    tftp_server->port_suspend.timeout = MAX_WAIT;
                    tftp_server->port_suspend.timeout_enabled  = FALSE;

                    SYS_LOG_FUNCTION_MSG(TFTPS, SYS_LOG_INFO, "file transfered interrupted", "");
                }

                /* Push the error opcode. */
                opcode = TFTP_OP_ERR;

                /* Push error response on the buffer. */
                if (fs_buffer_push_offset(rx_buffer, &opcode, 2, 0, (FS_BUFFER_PACKED | FS_BUFFER_TAIL)) == SUCCESS)
                {
                    switch (status)
                    {
                    /* If client is not connected or we received an out of
                     * bound frame. */
                    case TFTP_UNKNOWN_TID:
                    case TFTP_OUTOFBOUND_BLOCK:

                        /* Send transaction ID error. */
                        block = TFTP_ERROR_TID;

                        break;

                    /* Error is not mapped on the TFTP error. */
                    default:
                        /* Push the error code. */
                        block = TFTP_ERROR_GEN;
                        break;
                    }

                    /* Push error code on the buffer. */
                    if (fs_buffer_push(rx_buffer, &block, 2, (FS_BUFFER_PACKED | FS_BUFFER_TAIL)) == SUCCESS)
                    {
                        /* Push the required error message. */
                        switch (status)
                        {
                        /* TFTP command not supported. */
                        case TFTP_NOT_SUPPORTED:

                            /* Set error that opcode is not supported. */
                            status = fs_buffer_push(rx_buffer, TFTP_ERRMSG_NOT_SUPPORTED, sizeof(TFTP_ERRMSG_NOT_SUPPORTED), (FS_BUFFER_TAIL));

                            break;

                        /* Filename was too long. */
                        case TFTP_LONG_FILENAME:

                            /* Send error that file name is too long. */
                            status = fs_buffer_push(rx_buffer, TFTP_ERRMSG_FILENAME, sizeof(TFTP_ERRMSG_FILENAME), (FS_BUFFER_TAIL));

                            break;

                        /* Filename was not opened. */
                        case TFTP_ERROR_FS:

                            /* Send error that file system did not open the file. */
                            status = fs_buffer_push(rx_buffer, TFTP_ERRMSG_FS, sizeof(TFTP_ERRMSG_FS), (FS_BUFFER_TAIL));

                            break;

                        /* If no more clients can be connected. */
                        case TFTP_ERROR_EXHAUSTED:

                            /* Send error that connections are exhausted. */
                            status = fs_buffer_push(rx_buffer, TFTP_ERRMSG_EXHAUSTED, sizeof(TFTP_ERRMSG_EXHAUSTED), (FS_BUFFER_TAIL));

                            break;

                        /* If client is not connected. */
                        case TFTP_UNKNOWN_TID:

                            /* Send error that client's transaction ID was not resolved. */
                            status = fs_buffer_push(rx_buffer, TFTP_ERRMSG_TID, sizeof(TFTP_ERRMSG_TID), (FS_BUFFER_TAIL));

                            break;

                        /* Out of bound frame we received. */
                        case TFTP_OUTOFBOUND_BLOCK:

                            /* Send error that an out of bound block was received. */
                            status = fs_buffer_push(rx_buffer, TFTP_ERRMSG_BLOCK, sizeof(TFTP_ERRMSG_BLOCK), (FS_BUFFER_TAIL));

                            break;

                        /* Unknown error message. */
                        default:

                            /* Error message not known. */
                            status = fs_buffer_push(rx_buffer, "", 1, (FS_BUFFER_TAIL));

                            break;
                        }
                    }
                }
            }
        }

        /* If we are not sending a reply. */
        if (status != SUCCESS)
        {
            /* Free the buffer. */
            fs_buffer_add_buffer_list(rx_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Release lock for buffer file descriptor. */
        fd_release_lock(rx_buffer->fd);

        if (status == SUCCESS)
        {
            /* Send this buffer back to the host. */
            tftp_server->port.destination_address = tftp_server->port.last_datagram_address;
            tftp_server->port.destination_address.local_ip = IPV4_ADDR_UNSPEC;
            ipv4_get_device_address(rx_buffer->fd, &tftp_server->port.destination_address.local_ip, NULL);

            /* Send received data back on the UDP port. */
            received = fs_write(&tftp_server->port, (uint8_t *)rx_buffer, sizeof(FS_BUFFER));
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(TFTPS, status);

} /* tftp_server_process */

#endif /* CONFIG_TFTPS */
