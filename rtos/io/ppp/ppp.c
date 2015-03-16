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
#include <string.h>

const uint32_t ppp_lcp_accm = 0x00000000;
const uint16_t ppp_lcp_mru = 1500;
const PPP_LCP_OPT ppp_lcp_options[PPP_OPT_DB_NUM_OPTIONS] =
{
    /*  Value,                              Length,     Send,   Padding,    */
    {   NULL,                               0xFF,       FALSE,  {0, 0},     },  /* 0: Invalid option. */
    {   NULL,                               0x04,       FALSE,  {0, 0},    },  /* 1: MRU. */
    {   (const uint8_t*)&ppp_lcp_accm,      0x06,       TRUE,   {0, 0},    },  /* 2: ACCM. */
    {   NULL,                               0xFF,       FALSE,  {0, 0},    },  /* 3: Invalid option. */
    {   NULL,                               0xFF,       FALSE,  {0, 0},    },  /* 4: Invalid option. */
    {   (const uint8_t*)(LCP_OPT_RANDOM),   0x06,       TRUE,   {0, 0},    },  /* 5: MAGIC. */
    {   NULL,                               0xFF,       FALSE,  {0, 0},    },  /* 6: Invalid option. */
    {   (const uint8_t*)(LCP_OPT_NO_VALUE), 0x02,       TRUE,   {0, 0},    },  /* 7: PFC. */
    {   (const uint8_t*)(LCP_OPT_NO_VALUE), 0x02,       TRUE,   {0, 0},    },  /* 8: ACFC. */
};

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
    fs_set_data_watcher(fd, &ppp->data_watcher);

    /* Initialize connection watcher. */
    ppp->connection_watcher.data = ppp;
    ppp->connection_watcher.connected = &ppp_connection_established;
    ppp->connection_watcher.disconnected = &ppp_connection_terminated;

    /* Register connection watcher. */
    fs_set_connection_watcher(fd, &ppp->connection_watcher);

    /* Assume that we are already connected. */
    ppp->state = PPP_STATE_CONNECTED;
    ppp->state_data.lcp_id = 0;

    /* Initialize state. */
    ppp->rx_accm = (0xFFFFFFFF);
    ppp->tx_accm[0] = (0xFFFFFFFF);
    ppp->tx_accm[1] = (0x0);
    ppp->tx_accm[2] = (0x0);
    ppp->tx_accm[3] = (0x60000000);

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
    ((PPP *)ppp)->rx_accm = (0xFFFFFFFF);
    ((PPP *)ppp)->tx_accm[0] = (0xFFFFFFFF);
    ((PPP *)ppp)->tx_accm[1] = (0x0);
    ((PPP *)ppp)->tx_accm[2] = (0x0);
    ((PPP *)ppp)->tx_accm[3] = (0x60000000);

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
            /* Free the received buffer. */
            fs_buffer_add(fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);

            /* We are in LCP state now. */
            ppp->state = PPP_STATE_LCP;
        }

        /* If we are not ignoring the received buffer. */
        else if (status != MODEM_CHAT_IGNORE)
        {
            /* Free the received buffer. */
            fs_buffer_add(fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
        }
    }

} /* ppp_process_modem_chat */

/*
 * ppp_lcp_configuration_add
 * @buffer: Buffer needed to process.
 * @return: A success status will be returned if all required options were
 *  added in the buffer. PPP_INTERNAL_ERROR will be returned if an internal
 *  error has accrued.
 * This function will add our LCP configuration options so that a configuration
 * request can be sent.
 */
int32_t ppp_lcp_configuration_add(FS_BUFFER *buffer)
{
    PPP_PKT_OPT option;
    int32_t status, random;
    uint8_t i;
    uint8_t opt_value[4];

    for (i = 0; (status == SUCCESS) && (i < PPP_OPT_DB_NUM_OPTIONS); i++)
    {
        /* If we need to send this option. */
        if (ppp_lcp_options[i].do_send == TRUE)
        {
            /* Check if this option takes a random value. */
            if (ppp_lcp_options[i].value == (const uint8_t *)(LCP_OPT_RANDOM))
            {
                /* Generate and put a random value. */
                random = (int32_t)current_system_tick();
                fs_memcpy_r((char *)opt_value, (char *)&random, (uint32_t)(ppp_lcp_options[i].length - 2));
                option.data = opt_value;
            }

            /* Check if this option does not take a value. */
            else if (ppp_lcp_options[i].value == (const uint8_t *)(LCP_OPT_NO_VALUE))
            {
                option.data = NULL;
            }

            /* Check if we have specified a value for this option. */
            else if (ppp_lcp_options[i].value != NULL)
            {
                /* Copy the given value in the option buffer. */
                fs_memcpy_r((char *)opt_value, (char *)ppp_lcp_options[i].value, (uint32_t)(ppp_lcp_options[i].length - 2));
                option.data = opt_value;
            }

            else
            {
                /* Should not happen return an error. */
                status = PPP_INTERNAL_ERROR;
            }

            if (status == SUCCESS)
            {
                /* Initialize the option needed to be send. */
                option.type = i;
                option.length = ppp_lcp_options[i].length;

                /* Add this option in the transmit buffer. */
                status = ppp_packet_configuration_option_add(&option, buffer);
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_lcp_configuration_add */

/*
 * ppp_lcp_configuration_process
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * @buffer: Buffer needed to process.
 * This function will be called to process LCP configuration packets.
 */
void ppp_lcp_configuration_process(void *fd, PPP *ppp, FS_BUFFER *buffer)
{
    PPP_PKT rx_packet, tx_packet;
    PPP_PKT_OPT option;
    FS_BUFFER *tx_buffer = NULL;
    int32_t status;

    /* Clear the packet structure in which we will be parsing the data. */
    memset(&rx_packet, 0, sizeof(PPP_PKT));

    /* Parse the configuration header. */
    status = ppp_packet_configuration_header_parse(&rx_packet, buffer);

    if (status == SUCCESS)
    {
        /* If this is a configuration request then we will send a ACK, NAK
         * or Reject in the reply for this packet. */
        if (rx_packet.code == PPP_LCP_CONFIG_REQ)
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

                /* We will be sending an ACK until we see a requirement
                 * for not to. */
                tx_packet.code = PPP_LCP_CONFIG_ACK;
                tx_packet.id = rx_packet.id;
            }
            else
            {
                /* We don't have buffers to process this request. */
                status = PPP_NO_BUFFERS;
            }
        }

        /* Parse all the in-line packet options. */
        while (status == SUCCESS)
        {
            /* Parse next packet option. */
            status = ppp_packet_configuration_option_parse(&option, buffer);

            /* If we have successfully parsed the option. */
            if (status == SUCCESS)
            {
                /* Check if this option is supported. */
                if (!((1 << option.type) & PPP_LCP_OPTION_MASK))
                {
                    /* If we have not rejected a option yet. */
                    if (tx_packet.code != PPP_LCP_CONFIG_REJECT)
                    {
                        /* This option is not negotiable as we don't
                         * support it. */
                        tx_packet.code = PPP_LCP_CONFIG_REJECT;

                        /* Remove any data already on the buffer. */
                        fs_buffer_pull(tx_buffer, NULL, tx_buffer->length, 0);
                    }

                    /* Add this option in the transmit buffer. */
                    status = ppp_packet_configuration_option_add(&option, tx_buffer);
                }
                else
                {
                    /* Validate the option length. */
                    if ( (option.type < PPP_OPT_DB_NUM_OPTIONS) &&
                         (option.length == ppp_lcp_options[option.type].length) )
                    {
                        /* Process the option data. */
                        switch (option.type)
                        {
                        /* Maximum receive unit. */
                        case (PPP_LCP_OPT_MRU):

                            /* If we have received an ACK for this configuration. */
                            if (rx_packet.code == PPP_LCP_CONFIG_ACK)
                            {
                                /* Save the received MRU value. */
                                fs_memcpy_r((char *)&(ppp->mru), (char *)option.data, (uint32_t)(option.length - 2));
                            }

                            /* Break out of this switch. */
                            break;

                        /* Asynchronous control character map. */
                        case (PPP_LCP_OPT_ACCM):

                            /* If we are accepting the configuration. */
                            if (rx_packet.code == PPP_LCP_CONFIG_REQ)
                            {
                                /* Pull the ACCM in PPP configuration as receive ACCM. */
                                fs_memcpy_r((char *)&(ppp->rx_accm), (char *)option.data, (uint32_t)(option.length - 2));
                            }

                            /* If we have received an ACK for this configuration. */
                            else if (rx_packet.code == PPP_LCP_CONFIG_ACK)
                            {
                                /* Pull the ACCM in PPP configuration as transmit ACCM. */
                                fs_memcpy_r((char *)&(ppp->tx_accm[0]), (char *)option.data, (uint32_t)(option.length - 2));
                            }

                            break;

                        /* Magic number. */
                        case (PPP_LCP_OPT_MAGIC):

                            /* Keep the magic number as it is. */
                            /* TODO: If this not configuration request
                             * validate this. */
                            break;

                        /* Protocol field compression. */
                        case (PPP_LCP_OPT_PFC):

                            /* If we have received an ACK for this configuration. */
                            if (rx_packet.code == PPP_LCP_CONFIG_ACK)
                            {
                                /* Set the flag in PPP structure that we might
                                 * receive compressed protocol field. */
                                ppp->flags |= PPP_FLAG_PFC;
                            }

                            break;

                        /* Address and control field compression. */
                        case (PPP_LCP_OPT_ACFC):

                            /* If we have received an ACK for this configuration. */
                            if (rx_packet.code == PPP_LCP_CONFIG_ACK)
                            {
                                /* Set the flag in PPP structure that we might
                                 * receive compressed address and control fields. */
                                ppp->flags |= PPP_FALG_ACFC;
                            }

                            break;

                        default:
                            /* Nothing to do here. */
                            break;
                        }

                        /* If we have not accepted this option and we are not in NAK state. */
                        if ((status == PPP_VALUE_NOT_SUPPORTED) && (tx_packet.code != PPP_LCP_CONFIG_NAK))
                        {
                            /* The option value is not supported. */
                            tx_packet.code = PPP_LCP_CONFIG_NAK;

                            /* Remove any data already on the buffer. */
                            fs_buffer_pull(tx_buffer, NULL, tx_buffer->length, 0);
                        }

                        /* If we have not accepted this option and we are not in the REJECT state. */
                        if ((status == PPP_NOT_SUPPORTED) && (tx_packet.code != PPP_LCP_CONFIG_REJECT))
                        {
                            /* This option is not supported. */
                            tx_packet.code = PPP_LCP_CONFIG_REJECT;

                            /* Remove any data already on the buffer. */
                            fs_buffer_pull(tx_buffer, NULL, tx_buffer->length, 0);
                        }

                        /* Add this option in the transmit buffer. */
                        status = ppp_packet_configuration_option_add(&option, tx_buffer);
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

        /* If options in the packets were successfully parsed. */
        if (status == SUCCESS)
        {
            /* If we actually need to send a reply. */
            if (tx_packet.code != PPP_LCP_CONFIG_NONE)
            {
                /* Required options needed to sent are already pushed
                 * in the buffer. */

                /* Push the PPP header on the buffer. */
                status = ppp_packet_configuration_header_add(&tx_packet, tx_buffer);

                if (status == SUCCESS)
                {
                    /* Add PPP protocol. */
                    status = ppp_packet_protocol_add(tx_buffer, PPP_PROTO_LCP, PPP_IS_PFC_VALID(ppp));
                }

                if (status == SUCCESS)
                {
                    /* Add the HDLC header. */
                    status = hdlc_header_add(tx_buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp));
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
        /* Check if we have received a request and we have sent an ACK in
         * response, send our own configuration. */
        if ( (rx_packet.code == PPP_LCP_CONFIG_REQ) &&
             (tx_packet.code == PPP_LCP_CONFIG_ACK) )
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

                /* We will be sending an ACK until we see a requirement
                 * for not to. */
                tx_packet.code = PPP_LCP_CONFIG_REQ;
                tx_packet.id = ++(ppp->state_data.lcp_id);

                /* Add configuration options we need to send. */
                status = ppp_lcp_configuration_add(tx_buffer);

                if (status == SUCCESS)
                {
                    /* Push the PPP header on the buffer. */
                    status = ppp_packet_configuration_header_add(&tx_packet, tx_buffer);

                    if (status == SUCCESS)
                    {
                        /* Add PPP protocol. */
                        status = ppp_packet_protocol_add(tx_buffer, PPP_PROTO_LCP, PPP_IS_PFC_VALID(ppp));
                    }

                    if (status == SUCCESS)
                    {
                        /* Add the HDLC header. */
                        status = hdlc_header_add(tx_buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp));
                    }

                    if (status == SUCCESS)
                    {
                        /* Add this buffer to the TX list. */
                        fs_buffer_add(fd, tx_buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);
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
            else
            {
                /* We don't have buffers to process this request. */
                status = PPP_NO_BUFFERS;
            }
        }

        /* Check if we have received an ACK. */
        else if (rx_packet.code == PPP_LCP_CONFIG_REQ)
        {
            /* We have now received an ACK so our link is now established. */
            /* Now start network configuration. */
            ppp->state = PPP_STATE_NCP;
            ppp->state_data.ncp_id = 0;
        }
    }

} /* ppp_lcp_configuration_process */

/*
 * ppp_ncp_configuration_process
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * @buffer: Buffer needed to process.
 * This function will be called to process NCP configuration packets.
 */
void ppp_ncp_configuration_process(void *fd, PPP *ppp, FS_BUFFER *buffer)
{
    ;
} /* ppp_ncp_configuration_process */

/*
 * ppp_process_configuration
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * This function will be called with the packets that we will receive during
 * configuration phase.
 */
void ppp_process_configuration(void *fd, PPP *ppp)
{
    FS_BUFFER *buffer;
    int32_t status;
    uint16_t protocol;
    static uint8_t num_received = 0;

    if (num_received == 2)
    {
        num_received = 2;
    }
    num_received ++;

    /* Get the buffer from the receive list. */
    buffer = fs_buffer_get(fd, FS_BUFFER_RX, FS_BUFFER_ACTIVE);

    /* If we actually have a buffer to process. */
    if (buffer)
    {
        /* Verify and skim the HDLC headers. */
        status = hdlc_header_parse(buffer, PPP_IS_ACFC_VALID(ppp));

        if (status == SUCCESS)
        {
            /* Pick the protocol field. */
            status = ppp_packet_protocol_parse(buffer, &protocol, PPP_IS_PFC_VALID(ppp));
        }

        /* If this is a LCP configuration. */
        if ((status == SUCCESS) && (protocol == PPP_PROTO_LCP))
        {
            /* Process LCP configuration. */
            ppp_lcp_configuration_process(fd, ppp, buffer);
        }

        /* If this is a NCP configuration. */
        else if (status == SUCCESS)
        {
            /* Process NCP configuration. */
            ppp_ncp_configuration_process(fd, ppp, buffer);
        }

        else
        {
            /* Either protocol is not supported or an invalid header was given. */
            status = PPP_NOT_SUPPORTED;
        }

        /* Free the received buffer. */
        fs_buffer_add(fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
    }
} /* ppp_process_configuration */

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

        switch (ppp->state)
        {
        /* If physical medium is connected. */
        case PPP_STATE_CONNECTED:

            /* Link layer is connected, and we are expecting some modem
             * initialization before formally start handling PPP packets. */
            ppp_process_modem_chat(fd, ppp);

            /* Break out of this switch. */
            break;

        /* If we are processing LCP or NCP configuration packets. */
        case PPP_STATE_LCP:
        case PPP_STATE_NCP:

            /* Process Link-layer Configuration Packets. */
            ppp_process_configuration(fd, ppp);

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
