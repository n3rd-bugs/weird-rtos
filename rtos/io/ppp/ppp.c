/*
 * ppp.c
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

#ifdef CONFIG_PPP
#include <sll.h>
#include <string.h>

/*
 * ppp_register_fd
 * @ppp: PPP instance data.
 * @fd: File descriptor to hook with this PPP instance.
 * This function will register a file descriptor with PPP.
 */
void ppp_register_fd(PPP *ppp, FD fd)
{
    /* Will only work with buffered file descriptors. */
    OS_ASSERT((((FS *)fd)->flags & FS_BUFFERED) == 0);

    /* Clear the PPP global instance. */
    memset(ppp, 0, sizeof(PPP));

    /* Initialize data watcher. */
    ppp->data_watcher.data = ppp;
    ppp->data_watcher.data_available = &ppp_rx_watcher;
    ppp->data_watcher.space_available = &ppp_tx_watcher;

    /* Register data watcher. */
    fs_data_watcher_set(fd, &ppp->data_watcher);

    /* Initialize connection watcher. */
    ppp->connection_watcher.data = ppp;
    ppp->connection_watcher.connected = &ppp_connection_established;
    ppp->connection_watcher.disconnected = &ppp_connection_terminated;

    /* Register connection watcher. */
    fs_connection_watcher_set(fd, &ppp->connection_watcher);

    /* Assume that we are already connected. */
    ppp->state = PPP_STATE_CONNECTED;
    ppp->state_data.lcp_id = 0;

#ifdef CONFIG_SEMAPHORE
    /* Create the PPP instance semaphore. */
    semaphore_create(&ppp->lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

} /* ppp_register_fd */

/*
 * ppp_connection_established
 * @fd: File descriptor for which connection is established.
 * @ppp: PPP file descriptor data.
 * This function will be called whenever a connection is established for a
 * registered file descriptor.
 */
void ppp_connection_established(void *fd, void *ppp)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Acquire global data lock for PPP. */
    OS_ASSERT(semaphore_obtain(&((PPP *)ppp)->lock, MAX_WAIT) != SUCCESS)
#endif

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Link layer is now connected. */
    ((PPP *)ppp)->state = PPP_STATE_CONNECTED;

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global data lock. */
    semaphore_release(&((PPP *)ppp)->lock);
#endif

} /* ppp_connection_established */

/*
 * ppp_connection_terminated
 * @fd: File descriptor for which connection was terminated.
 * @ppp: PPP file descriptor data.
 * This function will be called whenever a connection is terminated for a
 * registered file descriptor.
 */
void ppp_connection_terminated(void *fd, void *ppp)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Acquire global data lock for PPP. */
    OS_ASSERT(semaphore_obtain(&((PPP *)ppp)->lock, MAX_WAIT) != SUCCESS)
#endif

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Link layer is now disconnected. */
    ((PPP *)ppp)->state = PPP_STATE_DISCONNECTED;

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global data lock. */
    semaphore_release(&((PPP *)ppp)->lock);
#endif

} /* ppp_connection_terminated */

/*
 * ppp_get_buffer_head_room
 * This function will be called return the number of bytes we need to leave on
 * this buffer so that we can later add them. This includes HDLC frame data,
 * and PPP header,
 */
uint32_t ppp_get_buffer_head_room(PPP *ppp)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* For now we don't support any compression so just return the hard coded
     * values. */
    return (6 + 2);

} /* ppp_get_buffer_head_room */

/*
 * ppp_process_modem_chat
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * This function will be called with the packets that we will receive during
 * link initialization or modem chat phase.
 */
void ppp_process_modem_chat(void *fd, PPP *ppp)
{
    FS_BUFFER *buffer;
    int32_t status;

    /* We will just peek the packet and see if it related to us. */
    buffer = fs_buffer_get(fd, FS_BUFFER_RX, FS_BUFFER_ACTIVE);

    /* If we do have a buffer. */
    if (buffer)
    {
        /* Handle modem initialization. */
        status = modem_chat_process(fd, buffer);

        /* If this was our buffer. If not this will lie on the file descriptor
         * so if someone else is exacting it, can receive this buffer. */
        if (status == SUCCESS)
        {
            /* We are in LCP state now. */
            ppp->state = PPP_STATE_LCP;
            ppp_lcp_state_initialize(ppp);
        }

        /* Free the received buffer. */
        fs_buffer_add(fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
    }

} /* ppp_process_modem_chat */

/*
 * ppp_configuration_process
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * @buffer: Buffer needed to process.
 * @proto: PPP protocol data.
 * This function will be called to process LCP configuration packets.
 */
void ppp_configuration_process(void *fd, PPP *ppp, FS_BUFFER_CHAIN *buffer, PPP_PROTO *proto)
{
    PPP_PKT rx_packet, tx_packet;
    PPP_PKT_OPT option;
    FS_BUFFER *tx_buffer = NULL;
    int32_t status;

    /* Clear the packet structure in which we will be parsing the data. */
    memset(&rx_packet, 0, sizeof(PPP_PKT));

    /* Parse the configuration header. */
    status = ppp_packet_configuration_header_parse(buffer, &rx_packet);

    if (status == SUCCESS)
    {
        /* If this is a configuration request then we will send a ACK, NAK
         * or Reject in the reply for this packet. */
        if ( (rx_packet.code == PPP_CONFIG_REQ) ||
             (rx_packet.code == PPP_TREM_REQ) )
        {
            /* Pull a buffer from the free list to use when we are building
             * the response. */
            tx_buffer = fs_buffer_get(fd, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);

            if (tx_buffer != NULL)
            {
                /* Clear the TX packet to initialize a reply for this
                 * packet. */
                memset(&tx_packet, 0, sizeof(PPP_PKT));

                /* Leave head room on the buffer so that it can be sent. */
                /* Add extra room for LCP. */
                fs_buffer_add_head(tx_buffer, (ppp_get_buffer_head_room(ppp) + 4));

                if (rx_packet.code == PPP_CONFIG_REQ)
                {
                    /* We will be sending an ACK until we see a requirement
                     * for not to. */
                    tx_packet.code = PPP_CONFIG_ACK;
                }

                else if (rx_packet.code == PPP_TREM_REQ)
                {
                    /* Send a terminate ACK in response. */
                    tx_packet.code = PPP_TREM_ACK;
                }

                /* Use the same ID in reply too. */
                tx_packet.id = rx_packet.id;
            }
            else
            {
                /* We don't have buffers to process this request. */
                status = PPP_NO_BUFFERS;
            }
        }

        /* Don't process data in terminate request. */
        if (rx_packet.code != PPP_TREM_REQ)
        {
            /* Parse all the in-line packet options. */
            while (status == SUCCESS)
            {
                /* Parse next packet option. */
                status = ppp_packet_configuration_option_parse(buffer, &option);

                /* If we have successfully parsed the option. */
                if (status == SUCCESS)
                {
                    /* Check if this option is not supported. */
                    if (proto->negotiable(ppp, &option) == FALSE)
                    {
                        /* If we have not rejected a option yet. */
                        if (tx_packet.code != PPP_CONFIG_REJECT)
                        {
                            /* This option is not negotiable as we don't
                             * support it. */
                            tx_packet.code = PPP_CONFIG_REJECT;

                            /* Remove any data already on the buffer. */
                            fs_buffer_one_pull(tx_buffer, NULL, tx_buffer->length, 0);
                        }

                        /* Add this option in the transmit buffer. */
                        status = ppp_packet_configuration_option_add(tx_buffer, &option);
                    }
                    else
                    {
                        /* Validate the option length. */
                        if (proto->length_valid (ppp, &option))
                        {
                            /* Process the option data. */
                            status = proto->process(ppp, &option, &rx_packet);

                            /* If we have not accepted this option and we are not in NAK state. */
                            /* If we are rejecting an option don't send a NAK send a REJECT. */
                            if ((status == PPP_VALUE_NOT_VALID) &&
                                (tx_packet.code != PPP_CONFIG_NAK) &&
                                (tx_packet.code != PPP_CONFIG_REJECT))
                            {
                                /* The option value is not supported. */
                                tx_packet.code = PPP_CONFIG_NAK;

                                /* Remove any data already on the buffer. */
                                fs_buffer_one_pull(tx_buffer, NULL, tx_buffer->length, 0);
                            }

                            /* If we have not accepted this option and we are not in the REJECT state. */
                            if ((status == PPP_NOT_SUPPORTED) &&
                                (tx_packet.code != PPP_CONFIG_REJECT))
                            {
                                /* This option is not supported. */
                                tx_packet.code = PPP_CONFIG_REJECT;

                                /* Remove any data already on the buffer. */
                                fs_buffer_one_pull(tx_buffer, NULL, tx_buffer->length, 0);
                            }

                            /* If a value is not valid and we are rejecting this
                             * packet don't add the option. */
                            if (((status == PPP_VALUE_NOT_VALID) &&
                                 (tx_packet.code == PPP_CONFIG_REJECT)) ||

                                /* If we are already sending a NAK or reject don't
                                 * add options which are supported. */
                                ((status == SUCCESS) &&
                                 (tx_packet.code != PPP_CONFIG_ACK)))
                            {
                                /* Just reset the status here. */
                                status = SUCCESS;
                            }
                            else
                            {
                                /* Add this option in the transmit buffer. */
                                status = ppp_packet_configuration_option_add(tx_buffer, &option);
                            }
                        }
                        else
                        {
                            /* Invalid option length was given. */
                            status = PPP_INVALID_HEADER;
                        }
                    }
                }

                /* If there are no more options to parse. */
                if (status == PPP_NO_NEXT_OPTION)
                {
                    status = SUCCESS;
                    break;
                }
            }
        }

        /* If options in the packets were successfully parsed. */
        if (status == SUCCESS)
        {
            /* If we actually need to send a reply. */
            if (tx_packet.code != PPP_CONFIG_NONE)
            {
                /* Required options needed to sent are already pushed
                 * in the buffer. */

                /* Push the PPP header on the buffer. */
                status = ppp_packet_configuration_header_add(tx_buffer, &tx_packet);

                if (status == SUCCESS)
                {
                    /* Add PPP protocol. */
                    status = ppp_packet_protocol_add(tx_buffer, proto->protocol, PPP_IS_PFC_VALID(ppp));
                }

                if (status == SUCCESS)
                {
                    /* Add the HDLC header. */
                    status = ppp_hdlc_header_add(tx_buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp), (proto->protocol == PPP_PROTO_LCP));
                }

                if (status == SUCCESS)
                {
                    /* Add this buffer to the TX list. */
                    fs_buffer_add(fd, tx_buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);
                }
            }
        }

        if (status != SUCCESS)
        {
            /* If we have allocated a TX buffer. */
            if (tx_buffer != NULL)
            {
                /* Free this buffer. */
                fs_buffer_add(fd, tx_buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
                tx_buffer = NULL;
            }
        }
    }

    /* If we did not get any error. */
    if (status == SUCCESS)
    {
        if (proto->update != NULL)
        {
            /* Update the internal state. */
            status = proto->update(fd, ppp, &rx_packet, &tx_packet);
        }
    }

} /* ppp_configuration_process */

/*
 * ppp_process_frame
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * This function will be parse the PPP frames and if is a PPP packet will be be
 * processed by related protocol otherwise packet will be forwarded to upper
 * layers for processing.
 */
void ppp_process_frame(void *fd, PPP *ppp)
{
    FS_BUFFER *buffer, *new_buffer;
    PPP_PROTO *proto;
    int32_t status = SUCCESS;
    uint16_t protocol;
    uint8_t num_flags, this_flag = 0;
    char *flag_ptr[2];

    /* Get the buffer from the receive list. */
    buffer = fs_buffer_get(fd, FS_BUFFER_RX, FS_BUFFER_ACTIVE);

    /* If a buffer was removed from the file descriptor. */
    if (buffer)
    {
        /* If we are not waiting for a partial packet we need two flag to parse
         * next packet otherwise we need 1 flag. */
        num_flags = (ppp->rx_buffer.list.head == NULL) + 1;

        /* Start from the buffer head. */
        flag_ptr[this_flag] = memchr(buffer->buffer, PPP_FLAG, buffer->length);

        /* While we have a flag. */
        while (flag_ptr[this_flag] != NULL)
        {
            /* Increment number of flags we have encountered. */
            this_flag++;

            /* If we have required amount of flags. */
            if (this_flag == num_flags)
            {
                break;
            }

            /* Search in this buffer and see if we have another flag. */
            flag_ptr[this_flag] = memchr(flag_ptr[this_flag - 1] + 1, PPP_FLAG,
                                         (uint32_t)((buffer->buffer + buffer->length) - (flag_ptr[this_flag - 1] + 1)));
        }

        /* Check if we have required amount of flags. */
        if (this_flag == num_flags)
        {
            this_flag = 0;

            /* If we were not waiting for a buffer. */
            if (num_flags == 2)
            {
                /* First check if we have some data before the first flag. */
                if (flag_ptr[this_flag] != buffer->buffer)
                {
                    /* Pull the junk data before the flag. */
                    fs_buffer_one_pull(buffer, NULL, (uint32_t)(flag_ptr[this_flag] - buffer->buffer), 0);
                }

                this_flag ++;
            }

            /* Check if there is still some data after the last flag. */
            if (flag_ptr[this_flag] != &buffer->buffer[buffer->length - 1])
            {
                /* Divide this buffer into two buffers. */
                fs_buffer_divide(fd, buffer, &new_buffer, flag_ptr[this_flag] + 1,
                                 (uint32_t)((buffer->buffer + buffer->length) - (flag_ptr[this_flag] + 1)));

                if (new_buffer != NULL)
                {
                    /* Silently add new buffer on the receive list of the
                     * file descriptor we have received the data. */
                    fs_buffer_add(fd, new_buffer, FS_BUFFER_RX, 0);
                }
            }

            /* Add this complete or partial received buffer on the receive buffer. */
            fs_buffer_chain_push(&ppp->rx_buffer, buffer, 0);
        }

        /* We will wait to receive required amount of flags so that we can
         * process a frame. */
        else
        {
            /* If we required 2 flags and we did not even get a single flag. */
            if ((num_flags == 2) && (this_flag == 0))
            {
                /* There is only junk in the buffer, so free it. */
                fs_buffer_add(fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
            }
            else
            {
                /* Add this partial received buffer on the receive buffer. */
                fs_buffer_chain_push(&ppp->rx_buffer, buffer, 0);
            }

            /* We don't have a buffer to continue. */
            status = PPP_PARTIAL_READ;
        }
    }

    /* If we actually have a buffer to process. */
    if (status == SUCCESS)
    {
        /* Save the file descriptor on which this buffer chain was build. */
        ppp->rx_buffer.fd = fd;

        /* Verify and skim the HDLC headers. */
        status = ppp_hdlc_header_parse(&ppp->rx_buffer, PPP_IS_ACFC_VALID(ppp));

        if (status == SUCCESS)
        {
            /* Pick the protocol field. */
            status = ppp_packet_protocol_parse(&ppp->rx_buffer, &protocol, PPP_IS_PFC_VALID(ppp));
        }

        /* If protocol was successfully parsed. */
        if (status == SUCCESS)
        {
            switch (protocol)
            {
            case (PPP_PROTO_LCP):

                /* Process LCP configuration. */
                proto = &ppp_proto_lcp;

                break;
            case (PPP_PROTO_IPCP):

                /* Process IPCP configuration. */
                proto = &ppp_proto_ipcp;

                break;
            default:

                /* Either protocol is not supported or an invalid header was given. */
                status = PPP_NOT_SUPPORTED;

                break;
            }
        }

        if (status == SUCCESS)
        {
            /* Process this configuration packet. */
            ppp_configuration_process(fd, ppp, &ppp->rx_buffer, proto);
        }

        if (ppp->rx_buffer.total_length > 80)
        {
            fs_buffer_chain_pull(&ppp->rx_buffer, NULL, ppp->rx_buffer.total_length, 0);
        }

        /* Free the received buffer. */
        fs_buffer_chain_add(&ppp->rx_buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
    }

} /* ppp_process_frame */

/*
 * ppp_rx_watcher
 * @fd: File descriptor for which this was called.
 * @ppp: PPP private data.
 * This function will be called when even there is some data available to read
 * from a file descriptor registered with a PPP instance.
 */
void ppp_rx_watcher(void *fd, void *priv_data)
{
    PPP *ppp = (PPP *)priv_data;

#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Acquire global data lock for PPP. */
    if (semaphore_obtain(&((PPP *)ppp)->lock, MAX_WAIT) == SUCCESS)
#endif
    {
        /* Process the received buffer according to the PPP state. */
        switch (ppp->state)
        {
        /* If physical medium is connected. */
        case PPP_STATE_CONNECTED:

            /* Link layer is connected, and we are expecting some modem
             * initialization before formally start handling PPP packets. */
            ppp_process_modem_chat(fd, ppp);

            /* Break out of this switch. */
            break;

        /* If we are processing LCP, IPCP frames or we have our connection
         * established. */
        case PPP_STATE_LCP:
        case PPP_STATE_IPCP:
        case PPP_STATE_NETWORK:

            /* Try to parse the packet and see if PPP needs to process it. */
            ppp_process_frame(fd, ppp);

            /* Break out of this switch. */
            break;

        /* If physical medium not connected. */
        case PPP_STATE_DISCONNECTED:
        default:
            /* Nothing to do here. */

            /* Just break out of this switch. */
            break;
        }

#ifdef CONFIG_SEMAPHORE
        /* Release the global data lock. */
        semaphore_release(&((PPP *)ppp)->lock);
#else
        /* Enable scheduling. */
        scheduler_unlock();
#endif
    }

} /* ppp_rx_watcher */

/*
 * ppp_tx_watcher
 * @fd: File descriptor for which this was called.
 * @ppp: PPP private data.
 * This function will be called when even there is some space available and we
 * can push new data on the file descriptor.
 */
void ppp_tx_watcher(void *fd, void *ppp)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);
    UNUSED_PARAM(fd);

} /* ppp_tx_watcher */

#endif /* CONFIG_PPP */
