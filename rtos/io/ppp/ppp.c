/*
 * ppp.c
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

#ifdef IO_PPP
#include <sll.h>
#include <string.h>
#include <fs.h>
#include <ppp.h>
#include <net.h>
#include <ppp_packet.h>
#include <ppp_target.h>

/* PPP global data. */
static PPP_DATA ppp_data;

/*
 * ppp_init
 * This function will initialize global PPP data.
 */
void ppp_init(void)
{
#ifdef PPP_TGT_INIT
    /* Initialize PPP target. */
    PPP_TGT_INIT();
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
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();
    FS *fs = (FS *)fd;

    /* Will only work with buffered file descriptors. */
    ASSERT((fs->flags & FS_BUFFERED) == 0);

    /* Clear the PPP instance. */
    memset(ppp, 0, sizeof(PPP));

    /* Assign the file descriptor with which this PPP instance is registered. */
    ppp->fd = fd;

    /* Assume that we are already connected. */
    ppp->state = PPP_STATE_INIT;
    ppp->state_data.lcp_id = 0;

    if (dedicated == TRUE)
    {
        /* Set the dedicated file descriptor flag. */
        ppp->flags |= PPP_DEDICATED_FD;
    }

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Add this device on the global instance list. */
    sll_append(&ppp_data.ppp, ppp, OFFSETOF(PPP, next));

    /* Restore the interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Register networking device for this PPP instance. */
    net_register_fd(&ppp->net_device, fd, &net_ppp_transmit, &net_ppp_receive);

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
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Pick the instance list head. */
    ret_instance = ppp_data.ppp.head;

    /* Search the instance list for the required PPP instance. */
    while ((ret_instance) && (ret_instance->fd != fd))
    {
        /* Get the next instance. */
        ret_instance = ret_instance->next;
    }

    /* Restore the interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Return the required device. */
    return (ret_instance);

} /* ppp_get_instance_fd */

/*
 * ppp_process_modem_chat
 * @fd: File descriptor on which this packet was received.
 * @ppp: PPP private data.
 * This function will be called to parse the packets that we will be receiving
 * during link initialization or modem chat phase.
 */
void ppp_process_modem_chat(void *fd, PPP *ppp)
{
    int32_t status;

    /* We should have a RX buffer here. */
    ASSERT(ppp->rx_buffer == NULL);

    /* Handle modem initialization. */
    status = modem_chat_process(fd, ppp->rx_buffer);

    /* If this was our buffer. If not buffer will lie on the file descriptor
     * so that if someone else is exacting it, can receive this buffer. */
    if (((ppp->flags & PPP_DEDICATED_FD) && (status != MODEM_CHAT_INCOMPLETE)) || (status == SUCCESS))
    {
        /* Free the RX buffer. */
        fs_buffer_add(fd, ppp->rx_buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
        ppp->rx_buffer = NULL;
    }

    /* If modem initialization was completed successfully. */
    if (status == SUCCESS)
    {
        /* We are in LCP state now. */
        ppp->state = PPP_STATE_LCP;
        ppp_lcp_state_initialize(ppp);
    }

} /* ppp_process_modem_chat */

/*
 * ppp_configuration_process
 * @ppp: PPP private data.
 * @buffer: Buffer needed to process.
 * @proto: PPP protocol data.
 * This function will be called to process PPP configuration packets.
 */
void ppp_configuration_process(PPP *ppp, FS_BUFFER_LIST *buffer, PPP_PROTO *proto)
{
    PPP_CONF_PKT rx_packet, tx_packet;
    PPP_CONF_OPT option;
    FS_BUFFER_LIST *tx_buffer = fs_buffer_get(buffer->fd, FS_LIST_FREE, 0);
    int32_t status;

    /* Should never happen. */
    ASSERT(tx_buffer == NULL);

    /* Clear RX/TX packets. */
    memset(&rx_packet, 0, sizeof(PPP_CONF_PKT));
    memset(&tx_packet, 0, sizeof(PPP_CONF_PKT));

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
                            fs_buffer_list_pull(tx_buffer, NULL, tx_buffer->total_length, 0);
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
                                fs_buffer_list_pull(tx_buffer, NULL, tx_buffer->total_length, 0);
                            }

                            /* If we have not accepted this option and we are not in the REJECT state. */
                            if ((status == PPP_NOT_SUPPORTED) &&
                                (tx_packet.code != PPP_CONFIG_REJECT))
                            {
                                /* This option is not supported. */
                                tx_packet.code = PPP_CONFIG_REJECT;

                                /* Remove any data already on the buffer. */
                                fs_buffer_list_pull(tx_buffer, NULL, tx_buffer->total_length, 0);
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
                    status = ppp_transmit_buffer_instance(ppp, tx_buffer, proto->protocol, 0);

                    if (status == SUCCESS)
                    {
                        /* TX buffer is no longer required. */
                        tx_buffer = NULL;
                    }
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

    /* If we still have TX buffer. */
    if (tx_buffer != NULL)
    {
        /* Free the TX buffer. */
        fs_buffer_add(tx_buffer->fd, tx_buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
    }

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
    FS_BUFFER_LIST *new_buffer;
    FS_BUFFER *buffer_one, *tmp_buffer_one, *new_buffer_one = NULL;
    PPP_PROTO *proto;
    int32_t status = PPP_PARTIAL_READ;
    uint32_t this_length;
    uint16_t protocol;
    uint8_t num_flags = 0, *flag_ptr;

    /* We should have a RX buffer here. */
    ASSERT(ppp->rx_buffer == NULL);

    /* If we have lost the sequence. */
    if ((ppp->rx_buffer->list.head) && (ppp->rx_buffer->list.head->buffer[0] != PPP_FLAG))
    {
        /* Free the RX buffer. */
        fs_buffer_add(ppp->fd, ppp->rx_buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
        ppp->rx_buffer = NULL;
    }
    else
    {
        /* Check if we have a flag in this buffer. */
        for (buffer_one = ppp->rx_buffer->list.head; buffer_one != NULL; buffer_one = buffer_one->next)
        {
            /* Check if we have a flag in this buffer. */
            flag_ptr = memchr(buffer_one->buffer, PPP_FLAG, buffer_one->length);

            /* If this is the end of a frame. */
            while (flag_ptr != NULL)
            {
                /* We have a PPP flag. */
                num_flags ++;

                /* Check if we have two flags in this buffer. */
                if (num_flags == 2)
                {
                    /* Calculate the number of bytes still valid in this buffer. */
                    this_length = (uint32_t)((flag_ptr + 1) - buffer_one->buffer);

                    /* We have successfully read a complete buffer. */
                    status = SUCCESS;

                    /* If this is not at the end on this buffer. */
                    if ((buffer_one->length > this_length) || (buffer_one->next != NULL))
                    {
                        /* Get a buffer list. */
                        new_buffer = fs_buffer_get(fd, FS_LIST_FREE, 0);

                        /* If we have some data left in the current buffer. */
                        if ((buffer_one->length > this_length))
                        {
                            /* If buffer was successfully allocated. */
                            if (new_buffer)
                            {
                                /* Divide this buffer into two buffers. */
                                if (fs_buffer_divide(fd, buffer_one, &new_buffer_one, 0, this_length) == SUCCESS)
                                {
                                    /* Add this one buffer on the new buffer. */
                                    fs_buffer_list_append(new_buffer, new_buffer_one, FS_BUFFER_HEAD);
                                }
                                else
                                {
                                    /* Free the allocated buffer. */
                                    fs_buffer_add(fd, new_buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
                                    new_buffer = NULL;

                                    /* Pull and discard extra data from this one buffer. */
                                    fs_buffer_pull(buffer_one, NULL, buffer_one->length - this_length, FS_BUFFER_TAIL);

                                    /* Reset the status as we will handle this condition. */
                                    status = SUCCESS;
                                }
                            }
                            else
                            {
                                /* Pull and discard extra data from this one buffer. */
                                fs_buffer_pull(buffer_one, NULL, buffer_one->length - this_length, FS_BUFFER_TAIL);
                            }
                        }

                        /* Update the buffer list. */
                        tmp_buffer_one = buffer_one->next;
                        buffer_one->next = NULL;
                        ppp->rx_buffer->list.tail = buffer_one;
                        ppp->rx_buffer->total_length -= (buffer_one->length - this_length);

                        /* Now add remaining one buffers in the buffer list. */
                        for (buffer_one = tmp_buffer_one; buffer_one != NULL; )
                        {
                            /* Update the buffer list. */
                            ppp->rx_buffer->total_length -= buffer_one->length;

                            /* Save the next buffer. */
                            tmp_buffer_one = buffer_one->next;

                            /* If we have new buffer. */
                            if (new_buffer)
                            {
                                /* Add this buffer on the new buffer. */
                                fs_buffer_list_append(new_buffer, buffer_one, 0);
                            }
                            else
                            {
                                /* Free this one buffer. */
                                fs_buffer_add(fd, buffer_one, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
                            }

                            /* Pick the next buffer. */
                            buffer_one = tmp_buffer_one;
                        }

                        /* If we have a buffer. */
                        if (new_buffer)
                        {
                            /* Add new buffer on the receive list. */
                            fs_buffer_add(fd, new_buffer, FS_BUFFER_RX, FS_BUFFER_ACTIVE);
                        }
                    }
                }

                /* If we have a buffer. */
                if (buffer_one)
                {
                    /* If we have at-least one bytes after PPP flag. */
                    if ((buffer_one->buffer + buffer_one->length) > flag_ptr)
                    {
                        /* Check if we have an other flag in this buffer. */
                        flag_ptr = memchr((flag_ptr + 1), PPP_FLAG, buffer_one->length - (uint32_t)((flag_ptr + 1) - buffer_one->buffer));
                    }
                }
                else
                {
                    /* We don't have a flag after this. */
                    flag_ptr = NULL;
                }
            }

            if (status == SUCCESS)
            {
                /* Break out of this loop. */
                break;
            }
        }
    }

    /* If we have a complete buffer to process a frame. */
    if (status == SUCCESS)
    {
        /* Verify and skim the HDLC headers. */
        status = ppp_hdlc_header_parse(ppp->rx_buffer, PPP_IS_ACFC_VALID(ppp));

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
            case PPP_PROTO_LCP:

                /* Process LCP configuration. */
                proto = &ppp_proto_lcp;

                break;

            /* PPP IPCP packets. */
            case PPP_PROTO_IPCP:

                /* Process IPCP configuration. */
                proto = &ppp_proto_ipcp;

                break;

            case PPP_PROTO_IPV4:

                /* Pass this buffer to the networking stack. */
                status = net_device_buffer_receive(ppp->rx_buffer, NET_PROTO_IPV4, 0);

                /* If buffer was successfully passed. */
                if (status == SUCCESS)
                {
                    /* Buffer is now forwarded to the stack. */
                    ppp->rx_buffer = NULL;

                    /* This buffer will now be handled by networking stack. */
                    status = PPP_BUFFER_FORWARDED;
                }

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
            fs_buffer_add(ppp->rx_buffer->fd, ppp->rx_buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
            ppp->rx_buffer = NULL;
        }
    }

} /* ppp_process_frame */

/*
 * net_ppp_transmit
 * @buffer: File system buffer needed to be transmitted.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: If buffer was successfully transmitted, otherwise PPP_INVALID_FD
 *  will be returned if the PPP instance was not resolved. PPP_INVALID_PROTO
 *  will be returned if a valid protocol was not resolved.
 * This function will transmit a PPP networking buffer.
 */
int32_t net_ppp_transmit(FS_BUFFER_LIST *buffer, uint8_t flags)
{
    int32_t status = SUCCESS;
    PPP *ppp;
    uint16_t protocol = 0;
    uint8_t net_proto;
    FS_BUFFER_LIST *buffer_copy;

    /* Resolve required PPP buffer instance. */
    ppp = ppp_get_instance_fd(buffer->fd);

    /* If we have a PPP instance. */
    if (ppp != NULL)
    {
        /* Skim the protocol from the buffer. */
        ASSERT(fs_buffer_list_pull(buffer, &net_proto, sizeof(uint8_t), 0) != SUCCESS);

        /* Process this frame according to the protocol. */
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

        /* If we have a free callback registered for this buffer. */
        if ((status == SUCCESS) && (buffer->free != NULL))
        {
            /* Allocate a buffer to make a copy of this buffer. */
            buffer_copy = fs_buffer_get(buffer->fd, FS_LIST_FREE, 0);

            /* If we have a buffer to make a copy of this frame. */
            if (buffer_copy)
            {
                /* Make a copy of this buffer. */
                status = fs_buffer_list_move_data(buffer_copy, buffer, FS_BUFFER_COPY);

                if (status == SUCCESS)
                {
                    /* Free the buffer. */
                    fs_buffer_add(buffer->fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);

                    /* Use the buffer copy from now on. */
                    buffer = buffer_copy;
                }
                else
                {
                    /* Free the buffer copy. */
                    fs_buffer_add(buffer_copy->fd, buffer_copy, FS_LIST_FREE, FS_BUFFER_ACTIVE);

                    /* We will free the original buffer later. */
                }
            }
            else
            {
                /* We don't have buffers to send this frame. */
                status = FS_BUFFER_NO_SPACE;
            }
        }

        if (status == SUCCESS)
        {
            /* Transmit this PPP buffer. */
            status = ppp_transmit_buffer_instance(ppp, buffer, protocol, flags);
        }
    }
    else
    {
        /* Return an error to the caller. */
        status = PPP_INVALID_FD;
    }

    if (status != SUCCESS)
    {
        /* Free the buffer. */
        fs_buffer_add(buffer->fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
    }

    /* Buffer will always be consumed. */
    status = NET_BUFFER_CONSUMED;

    /* Return status to the caller. */
    return (status);

} /* net_ppp_transmit */

/*
 * net_ppp_receive
 * @data: File descriptor on which data was received.
 * @status: Resumption status.
 * Function that will be called to receive packets on PPP device.
 */
void net_ppp_receive(void *data, int32_t status)
{
    PPP *ppp = ppp_get_instance_fd((FD)data);
    FS_BUFFER_LIST *buffer;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(status);

    /* Get lock for the PPP. */
    ASSERT(fd_get_lock(ppp->fd) != SUCCESS);

    /* Get the buffer from the receive list. */
    buffer = fs_buffer_get(ppp->fd, FS_BUFFER_RX, 0);

    /* If we have a buffer and we can process it. */
    if (buffer)
    {
        /* If we don't have a RX buffer. */
        if (ppp->rx_buffer == NULL)
        {
            /* Save this as PPP RX buffer. */
            ppp->rx_buffer = buffer;
            buffer = NULL;
        }
        else
        {
            /* Add the received data to the current RX buffer. */
            /* We will copy the data as, moving buffers will cause poor
             * performance. */
            if (fs_buffer_list_move_data(ppp->rx_buffer, buffer, FS_BUFFER_COPY) != SUCCESS)
            {
                /* Free the receive buffer. */
                fs_buffer_add(ppp->fd, ppp->rx_buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
                ppp->rx_buffer = NULL;
            }
        }
    }

    /* If we have a buffer to process. */
    if (ppp->rx_buffer)
    {
        /* Process the received buffer according to the PPP state. */
        switch (ppp->state)
        {
        /* If we are at initialization state. */
        case PPP_STATE_INIT:

            /* Link layer is connected, and we are expecting some modem
             * initialization before formally start handling PPP packets. */
            ppp_process_modem_chat(ppp->fd, ppp);

            /* Break out of this switch. */
            break;

        /* If we are processing LCP, IPCP frames or we have our connection
         * established. */
        case PPP_STATE_LCP:
        case PPP_STATE_IPCP:
        case PPP_STATE_NETWORK:

            /* Try to parse the packet and see if PPP needs to process it. */
            ppp_process_frame(ppp->fd, ppp);

            /* Break out of this switch. */
            break;

        default:
            /* Nothing to do here. */

            /* Just break out of this switch. */
            break;
        }
    }

    /* If we have a buffer to free. */
    if (buffer)
    {
        /* Free the original buffer. */
        fs_buffer_add(ppp->fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
    }

    /* Release lock for PPP. */
    fd_release_lock(ppp->fd);

} /* net_ppp_receive */

/*
 * ppp_transmit_buffer_instance
 * @ppp: PPP private data.
 * @buffer: Buffer needed to be sent.
 * @proto: PPP protocol needed to be added in the header.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * This function will add required fields on a given buffer and send it on the
 * file descriptor attached to the given buffer. Caller is responsible for
 * freeing the given buffer if this function returns an error.
 */
int32_t ppp_transmit_buffer_instance(PPP *ppp, FS_BUFFER_LIST *buffer, uint16_t proto, uint8_t flags)
{
    int32_t status;
    FD fd = buffer->fd;

    /* Add PPP protocol. */
    status = ppp_packet_protocol_add(buffer, proto, PPP_IS_PFC_VALID(ppp), flags);

    /* If PPP protocol was successfully added. */
    if (status == SUCCESS)
    {
        /* Add the HDLC header. */
        status = ppp_hdlc_header_add(buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp), (proto == PPP_PROTO_LCP), flags);
    }

    /* If HDLC header was successfully added. */
    if (status == SUCCESS)
    {
        /* Release lock for file descriptor. */
        fd_release_lock(fd);

        /* Add a transmit buffer. */
        fs_write(fd, (uint8_t *)buffer, sizeof(buffer));

        /* Acquire file descriptor lock. */
        ASSERT(fd_get_lock(fd) != SUCCESS);
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_transmit_buffer_instance */

#endif /* IO_PPP */
