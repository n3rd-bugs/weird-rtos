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
#include <fs.h>
#include <ppp.h>
#include <net.h>
#include <ppp_packet.h>

/* PPP global data. */
PPP_DATA ppp_data;

/*
 * ppp_init
 * This function will initialize global PPP data.
 */
void ppp_init()
{
    /* Clear the global data. */
    memset(&ppp_data, 0, sizeof(PPP_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create the PPP global data semaphore. */
    semaphore_create(&ppp_data.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

} /* ppp_init */

/*
 * ppp_register_fd
 * @ppp: PPP instance data.
 * @fd: File descriptor to hook with this PPP instance.
 * @dedicated: If true the registered FD will be considered as dedicated and
 *  packets not parsed correctly will be dropped.
 * This function will register a file descriptor with PPP.
 */
void ppp_register_fd(PPP *ppp, FD fd, uint8_t dedicated)
{
    /* Will only work with buffered file descriptors. */
    OS_ASSERT((((FS *)fd)->flags & FS_BUFFERED) == 0);

    /* Clear the PPP instance. */
    memset(ppp, 0, sizeof(PPP));

    /* Assign the file descriptor with which this PPP instance is registered. */
    ppp->fd = fd;

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

    if (dedicated == TRUE)
    {
        /* Set the dedicated file descriptor flag. */
        ppp->flags |= PPP_DEDICATED_FD;
    }

#ifdef CONFIG_SEMAPHORE
    /* Create the PPP instance semaphore. */
    semaphore_create(&ppp->lock, 1, 1, (SEMAPHORE_PRIORITY | SEMAPHORE_IRQ));

    /* Obtain the global data semaphore. */
    OS_ASSERT(semaphore_obtain(&ppp_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Add this device on the global instance list. */
    sll_append(&ppp_data.ppp, ppp, OFFSETOF(PPP, next));

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&ppp_data.lock);
#endif

    /* Register networking device for this PPP instance. */
    net_register_fd(&ppp->net_device, fd, &net_ppp_transmit);

} /* ppp_register_fd */

/*
 * ppp_get_instance_fd
 * @fd: File descriptor for which PPP instance is required.
 * @return: If not null the PPP instance associated with this file descriptor
 *  will be returned.
 * This function will return the associated PPP instance for the given file
 * descriptor.
 */
PPP *ppp_get_instance_fd(FD fd)
{
    PPP *ret_instance;
#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data semaphore. */
    OS_ASSERT(semaphore_obtain(&ppp_data.lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif

    /* Pick the instance list head. */
    ret_instance = ppp_data.ppp.head;

    /* Search the instance list for the required PPP instance. */
    while ((ret_instance) && (ret_instance->fd != fd))
    {
        /* Get the next instance. */
        ret_instance = ret_instance->next;
    }

#ifndef CONFIG_SEMAPHORE
    /* Enable scheduling. */
    scheduler_unlock();
#else
    /* Release the global semaphore. */
    semaphore_release(&ppp_data.lock);
#endif

    /* Return the required device. */
    return (ret_instance);

} /* ppp_get_instance_fd */

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
 * ppp_process_modem_chat
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * This function will be called to parse the packets that we will be receiving
 * during link initialization or modem chat phase.
 */
void ppp_process_modem_chat(void *fd, PPP *ppp)
{
    FS_BUFFER_ONE *buffer;
    int32_t status;

    /* Peek a packet from the receive list. */
    buffer = fs_buffer_one_get(fd, FS_BUFFER_RX, FS_BUFFER_INPLACE);

    /* If we do have a buffer. */
    if (buffer)
    {
        /* Handle modem initialization. */
        status = modem_chat_process(fd, buffer);

        /* If this was our buffer. If not buffer will lie on the file descriptor
         * so that if someone else is exacting it, can receive this buffer. */
        if ( (ppp->flags & PPP_DEDICATED_FD) || (status == SUCCESS) || (status != MODEM_CHAT_IGNORE))
        {
            /* Remove the buffer from the receive list and free it. */
            OS_ASSERT(fs_buffer_one_get(fd, FS_BUFFER_RX, 0) != buffer);
            fs_buffer_add(fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
        }

        /* If modem initialization was completed successfully. */
        if (status == SUCCESS)
        {
            /* We are in LCP state now. */
            ppp->state = PPP_STATE_LCP;
            ppp_lcp_state_initialize(ppp);
        }
    }

} /* ppp_process_modem_chat */

/*
 * ppp_configuration_process
 * @ppp: PPP private data.
 * @buffer: Buffer needed to process.
 * @proto: PPP protocol data.
 * This function will be called to process PPP configuration packets.
 */
void ppp_configuration_process(PPP *ppp, FS_BUFFER *buffer, PPP_PROTO *proto)
{
    PPP_CONF_PKT rx_packet, tx_packet;
    PPP_CONF_OPT option;
    FS_BUFFER *tx_buffer = fs_buffer_get(buffer->fd, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
    int32_t status;

    /* Should never happen. */
    OS_ASSERT(tx_buffer == NULL);

    /* Clear the packet structure and transmit buffers. */
    memset(&rx_packet, 0, sizeof(PPP_CONF_PKT));

    /* Parse the configuration header. */
    status = ppp_packet_configuration_header_parse(buffer, &rx_packet);

    /* If configuration header was successfully parsed. */
    if (status == SUCCESS)
    {
        /* If this is a configuration request then we will send a ACK, NAK
         * or Reject in the reply for this packet. */
        if ( (rx_packet.code == PPP_CONFIG_REQ) ||
             (rx_packet.code == PPP_TREM_REQ) )
        {
            /* Clear the transmit packet so that a reply can be initialized. */
            memset(&tx_packet, 0, sizeof(PPP_CONF_PKT));

            /* If this is a configuration request. */
            if (rx_packet.code == PPP_CONFIG_REQ)
            {
                /* We will be sending an ACK until we see a requirement
                 * for not to. */
                tx_packet.code = PPP_CONFIG_ACK;
            }

            /* If this is a terminate request. */
            else if (rx_packet.code == PPP_TREM_REQ)
            {
                /* Send a terminate ACK in response. */
                tx_packet.code = PPP_TREM_ACK;
            }

            /* Use the same ID in reply too. */
            tx_packet.id = rx_packet.id;
        }

        /* Don't process data if we don't have a configuration request or
         * configuration acknowledgment. */
        if ((rx_packet.code == PPP_CONFIG_REQ) || (rx_packet.code == PPP_CONFIG_ACK))
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
                            fs_buffer_pull(tx_buffer, NULL, tx_buffer->total_length, 0);
                        }

                        /* Add this option in the transmit buffer. */
                        status = ppp_packet_configuration_option_add(tx_buffer, &option);
                    }
                    else
                    {
                        /* Validate the option length. */
                        if (proto->length_valid(ppp, &option))
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
                                fs_buffer_pull(tx_buffer, NULL, tx_buffer->total_length, 0);
                            }

                            /* If we have not accepted this option and we are not in the REJECT state. */
                            if ((status == PPP_NOT_SUPPORTED) &&
                                (tx_packet.code != PPP_CONFIG_REJECT))
                            {
                                /* This option is not supported. */
                                tx_packet.code = PPP_CONFIG_REJECT;

                                /* Remove any data already on the buffer. */
                                fs_buffer_pull(tx_buffer, NULL, tx_buffer->total_length, 0);
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

        /* If it is not a terminate request. */
        else if (rx_packet.code != PPP_TREM_REQ)
        {
            /* This PPP code is not supported. */
            status = PPP_NOT_SUPPORTED;
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

                /* If PPP configuration header was successfully added. */
                if (status == SUCCESS)
                {
                    /* Send a PPP configuration packet in reply. */
                    status = ppp_transmit_buffer_instance(ppp, &tx_buffer, proto->protocol);
                }
            }
        }
    }

    /* If we did not get any error. */
    if (status == SUCCESS)
    {
        /* If an update function is registered with the protocol. */
        if (proto->update != NULL)
        {
            /* Update the internal state. */
            status = proto->update(buffer->fd, ppp, &rx_packet, &tx_packet);
        }
    }

    /* Free the buffer list allocated before and any buffers still left on it. */
    fs_buffer_add(tx_buffer->fd, tx_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

} /* ppp_configuration_process */

/*
 * ppp_process_frame
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * This function will be parse the PPP frames, if required a appropriate
 * configuration protocol will be invoked to process this packet otherwise will
 * be forwarded to the upper layers for processing.
 */
void ppp_process_frame(void *fd, PPP *ppp)
{
    FS_BUFFER_ONE *buffer, *new_buffer;
    PPP_PROTO *proto;
    int32_t status = SUCCESS;
    uint16_t protocol;
    uint8_t num_flags, this_flag = 0;
    uint8_t *flag_ptr[2];

    /* Get the buffer from the receive list. */
    buffer = fs_buffer_one_get(fd, FS_BUFFER_RX, FS_BUFFER_ACTIVE);

    /* If a buffer was removed from the file descriptor. */
    if (buffer)
    {
        if (ppp->rx_buffer == NULL)
        {
            /* Get a buffer list. */
            ppp->rx_buffer = fs_buffer_get(fd, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

            /* Should never happen. */
            OS_ASSERT(ppp->rx_buffer == NULL);
        }

        /* If we are not waiting for a partial packet we need two flag to parse
         * next packet otherwise we need 1 flag. */
        num_flags = (ppp->rx_buffer->list.head == NULL) + 1;

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
                /* Stop processing any more flags. */
                break;
            }

            /* Search in this buffer and see if we have another flag. */
            flag_ptr[this_flag] = memchr(flag_ptr[this_flag - 1] + 1, PPP_FLAG, (uint32_t)((buffer->buffer + buffer->length) - (flag_ptr[this_flag - 1] + 1)));
        }

        /* Check if we have required amount of flags. */
        if (this_flag == num_flags)
        {
            /* Reset the flag count. */
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
                fs_buffer_one_divide(fd, buffer, &new_buffer, flag_ptr[this_flag] + 1, (uint32_t)((buffer->buffer + buffer->length) - (flag_ptr[this_flag] + 1)));

                if (new_buffer != NULL)
                {
                    /* Silently add new buffer on the receive list of the
                     * file descriptor we have received the data. */
                    fs_buffer_add(fd, new_buffer, FS_BUFFER_RX, 0);
                }
            }

            /* Add this complete or partial received buffer on the receive buffer. */
            fs_buffer_add_one(ppp->rx_buffer, buffer, 0);
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
                fs_buffer_add_one(ppp->rx_buffer, buffer, 0);
            }

            /* We don't have a buffer to continue. */
            status = PPP_PARTIAL_READ;
        }

        /* If we have a complete buffer to process a frame. */
        if (status == SUCCESS)
        {
            /* Verify and skim the HDLC headers. */
            status = ppp_hdlc_header_parse(ppp->rx_buffer, PPP_IS_ACFC_VALID(ppp));

            /* TODO: Remove this. */
            OS_ASSERT(status != SUCCESS);

            /* If HDLC verification was successful. */
            if (status == SUCCESS)
            {
                /* Parse and pick the protocol field. */
                status = ppp_packet_protocol_parse(ppp->rx_buffer, &protocol, PPP_IS_PFC_VALID(ppp));
            }

            /* If protocol was successfully parsed. */
            if (status == SUCCESS)
            {
                /* Try to pick the an appropriate protocol to parse this packet. */
                switch (protocol)
                {
                /* PPP LCP packets. */
                case (PPP_PROTO_LCP):

                    /* Process LCP configuration. */
                    proto = &ppp_proto_lcp;

                    break;

                /* PPP IPCP packets. */
                case (PPP_PROTO_IPCP):

                    /* Process IPCP configuration. */
                    proto = &ppp_proto_ipcp;

                    break;

                case (PPP_PROTO_IPV4):

                    /* Send this buffer to the networking stack. */
                    net_device_buffer_receive(ppp->rx_buffer, NET_PROTO_IPV4);
                    ppp->rx_buffer = NULL;

                    /* This buffer will now be handled by networking stack. */
                    status = PPP_BUFFER_FORWARDED;

                    break;

                /* Not supported protocol. */
                default:

                    /* Either protocol is not supported or an invalid header was given. */
                    status = PPP_NOT_SUPPORTED;

                    break;
                }
            }

            /* If we have picked a protocol that is needed to be invoked to parse
             * this packet. */
            if (status == SUCCESS)
            {
                /* Process this configuration packet. */
                ppp_configuration_process(ppp, ppp->rx_buffer, proto);
            }

            /* If buffer was forwarded to the networking stack. */
            if (status != PPP_BUFFER_FORWARDED)
            {
                /* Free the received buffer. */
                fs_buffer_add(ppp->rx_buffer->fd, ppp->rx_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
                ppp->rx_buffer = NULL;
            }
        }
    }

} /* ppp_process_frame */

/*
 * net_ppp_transmit
 * @buffer: File system buffer needed to be transmitted.
 * @return: If buffer was successfully transmitted, otherwise PPP_INVALID_FD
 *  will be returned if the PPP instance was not resolved. PPP_INVALID_PROTO
 *  will be returned if a valid protocol was not resolved.
 * This function will transmit a PPP networking buffer.
 */
int32_t net_ppp_transmit(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    PPP *ppp = ppp_get_instance_fd(buffer->fd);
    uint16_t protocol = 0;
    uint8_t net_proto;

    if (ppp != NULL)
    {
#ifndef CONFIG_SEMAPHORE
        /* Lock the scheduler. */
        scheduler_lock();
#else
        /* Acquire data lock for PPP. */
        OS_ASSERT(semaphore_obtain(&((PPP *)ppp)->lock, MAX_WAIT) != SUCCESS)
#endif
        /* Skim the protocol from the buffer. */
        OS_ASSERT(fs_buffer_pull(buffer, &net_proto, sizeof(uint8_t), 0) != SUCCESS);

        switch (net_proto)
        {
        /* IPv4 protocol. */
        case NET_PROTO_IPV4:

            /* Pick the PPP IPv4 protocol. */
            protocol = PPP_PROTO_IPV4;
            break;
        default:

            /* Unknown protocol. */
            status = PPP_INVALID_PROTO;
            break;
        }

        if (status == SUCCESS)
        {
            /* Transmit this PPP buffer. */
            status = ppp_transmit_buffer_instance(ppp, &buffer, protocol);

            /* Free this buffer in any case. */
            fs_buffer_add(buffer->fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

#ifndef CONFIG_SEMAPHORE
        /* Enable scheduling. */
        scheduler_unlock();
#else
        /* Release the PPP instance lock. */
        semaphore_release(&((PPP *)ppp)->lock);
#endif
    }
    else
    {
        /* Return an error to the caller. */
        status = PPP_INVALID_FD;
    }

    /* Return status to the caller. */
    return (status);

} /* net_ppp_transmit */

/*
 * ppp_transmit_buffer_instance
 * @ppp: PPP private data.
 * @buffer: Buffer needed to be sent.
 * @proto: PPP protocol needed to be added in the header.
 * This function will add required fields on a given buffer and send it on the
 * file descriptor attached to the given buffer. Caller is responsible for
 * freeing the given buffer if this function returns an error.
 */
int32_t ppp_transmit_buffer_instance(PPP *ppp, FS_BUFFER **buffer, uint16_t proto)
{
    int32_t status;

    /* Add PPP protocol. */
    status = ppp_packet_protocol_add(*buffer, proto, PPP_IS_PFC_VALID(ppp));

    /* If PPP protocol was successfully added. */
    if (status == SUCCESS)
    {
        /* Add the HDLC header. */
        status = ppp_hdlc_header_add(buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp), (proto == PPP_PROTO_LCP));
    }

    /* If HDLC header was successfully added. */
    if (status == SUCCESS)
    {
        /* Add this buffer to the transmit list of the provided file descriptor. */
        fs_buffer_add_list(*buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_transmit_buffer_instance */

/*
 * ppp_rx_watcher
 * @fd: File descriptor for which this was called.
 * @ppp: PPP private data.
 * This function will be called when even there is some data available to read
 * from a file descriptor registered with a PPP instance.
 */
void ppp_rx_watcher(void *fd, void *priv_data)
{
    /* Pick up the PPP instance structure. */
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
