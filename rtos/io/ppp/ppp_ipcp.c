/*
 * ppp_ipcp.c
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

/* PPP IPCP protocol definition. */
PPP_PROTO ppp_proto_ipcp =
{
    .negotiable     = &ppp_ipcp_option_negotiable,
    .length_valid   = &ppp_ipcp_option_length_valid,
    .process        = &ppp_ipcp_option_pocess,
    .update         = &ppp_ipcp_update,
    .protocol       = PPP_PROTO_IPCP,
};

/*
 * ppp_ipcp_option_negotiable
 * @ppp: PPP private data.
 * @option: Option needed to be checked.
 * @return: Returns true if the given option is supported, otherwise false will
 *  be returned.
 * This function will return if a given option type for IPCP is negotiable or
 * not.
 */
uint8_t ppp_ipcp_option_negotiable(PPP *ppp, PPP_PKT_OPT *option)
{
    uint8_t negotiable = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* For now only IP address option is negotiable. */
    if (option->type == PPP_IPCP_OPT_IP)
    {
        negotiable = TRUE;
    }

    /* Return if this option is negotiable. */
    return (negotiable);

} /* ppp_ipcp_option_negotiable */

/*
 * ppp_ipcp_option_pocess
 * @ppp: PPP private data.
 * @option: Option needed to be process.
 * @rx_packet: Parsed PPP packet.
 * @return: Return success if option was successfully parsed,
 *  PPP_VALUE_NOT_VALID will be returned if a option value is not valid and a
 *  valid option value is returned in the option,
 *  PPP_NOT_SUPPORTED will be returned if option type is not supported.
 * This function will process the data for a given option.
 */
int32_t ppp_ipcp_option_pocess(PPP *ppp, PPP_PKT_OPT *option, PPP_PKT *rx_packet)
{
    int32_t status = PPP_NOT_SUPPORTED;
    uint8_t ip[4] = PPP_REMOTE_IP_ADDRESS;

    /* If we have a IP option. */
    if (option->type == PPP_IPCP_OPT_IP)
    {
        /* If this is a configuration request. */
        if (rx_packet->code == PPP_CONFIG_REQ)
        {
            /* If remote has a different IP address. */
            if (memcmp(option->data, ip, (uint32_t)(option->length - 2)))
            {
                /* Whatever IP address was given by the other end, overwrite it
                 * with our configured IP address. */
                memcpy(option->data, ip, (uint32_t)(option->length - 2));

                /* Tell the other end to use this IP address. */
                status = PPP_VALUE_NOT_VALID;
            }

            /* Remote has same IP address. */
            else
            {
                /* Copy the configured remote IP address in the PPP structure. */
                fs_memcpy_r((char *)&ppp->remote_ip_address, (char *)option->data, (uint32_t)(option->length - 2));

                /* Return success. */
                status = SUCCESS;
            }
        }

        /* If this is a ACK for our configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Copy the configured local IP address in the PPP structure. */
            fs_memcpy_r((char *)&ppp->local_ip_address, (char *)option->data, (uint32_t)(option->length - 2));

            /* Return success. */
            status = SUCCESS;
        }
    }

    /* Always return success. */
    return (status);

} /* ppp_ipcp_option_pocess */

/*
 * ppp_ipcp_option_length_valid
 * @ppp: PPP private data.
 * @option: Option for which valid length is needed.
 * @return: Returns true of option length is valid.
 * This function will provide valid length of a given option type.
 */
uint8_t ppp_ipcp_option_length_valid(PPP *ppp, PPP_PKT_OPT *option)
{
    uint8_t valid = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* We only support IP option for now. */
    if ( (option->type == PPP_IPCP_OPT_IP) && (option->length == 6) )
    {
        /* Option length is valid. */
        valid = TRUE;
    }

    /* Return if the given option is valid. */
    return (valid);

} /* ppp_ipcp_option_length_valid */

/*
 * ppp_ipcp_update
 * @fd: File descriptor on which a packet was received.
 * @ppp: PPP private data.
 * @rx_packet: Parsed PPP packet header that was received.
 * @tx_packet: Parsed PPP packet that was sent in the reply.
 * @return: Returns success if operation was successful.
 * This function will be called when a IPCP configuration packet is processed
 * and a rely has already sent and internal state is needed to be updated.
 */
int32_t ppp_ipcp_update(void *fd, PPP *ppp, PPP_PKT *rx_packet, PPP_PKT *tx_packet)
{
    int32_t status = SUCCESS;
    PPP_PKT_OPT option;
    FS_BUFFER *tx_buffer = NULL;
    uint8_t ip[4] = PPP_LOCAL_IP_ADDRESS;

    /* If we have not received an ACK for our configuration. */
    if (ppp->local_ip_address == 0)
    {
        /* Pull a buffer from the free list to use when we are building
         * the response. */
        tx_buffer = fs_buffer_get(fd, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);

        /* If we have a transmit buffer. */
        if (tx_buffer != NULL)
        {
            /* Clear the TX packet to initialize a reply for this
             * packet. */
            memset(tx_packet, 0, sizeof(PPP_PKT));

            /* Leave head room on the buffer so that it can be sent. */
            /* Add extra room for LCP. */
            fs_buffer_add_head(tx_buffer, (ppp_get_buffer_head_room(ppp) + 4));

            /* We will be sending an ACK until we see a requirement
             * for not to. */
            tx_packet->code = PPP_CONFIG_REQ;
            tx_packet->id = ++(ppp->state_data.ipcp_id);

            /* Add IP configuration option in the transmit buffer. */
            option.data = ip;
            option.length = 6;
            option.type = PPP_IPCP_OPT_IP;
            status = ppp_packet_configuration_option_add(&option, tx_buffer);

            if (status == SUCCESS)
            {
                /* Push the PPP header on the buffer. */
                status = ppp_packet_configuration_header_add(tx_packet, tx_buffer);

                if (status == SUCCESS)
                {
                    /* Add PPP protocol. */
                    status = ppp_packet_protocol_add(tx_buffer, PPP_PROTO_IPCP, PPP_IS_PFC_VALID(ppp));
                }

                if (status == SUCCESS)
                {
                    /* Add the HDLC header. */
                    status = ppp_hdlc_header_add(fd, tx_buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp), FALSE);
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

    /* IPCP configuration is being ACKed. */
    if (rx_packet->code == PPP_CONFIG_ACK)
    {
        /* We are now in network phase. */
        ppp->state = PPP_STATE_NETWORK;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_ipcp_update */

#endif /* CONFIG_PPP */
