/*
 * ppp_lcp.c
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

/* PPP LCP protocol definition. */
PPP_PROTO ppp_proto_lcp =
{
    .negotiable     = &ppp_lcp_option_negotiable,
    .length_valid   = &ppp_lcp_option_length_valid,
    .process        = &ppp_lcp_option_pocess,
    .update         = &ppp_lcp_update,
    .protocol       = PPP_PROTO_LCP,
};

/* PPP LCP option database. */
const uint32_t ppp_lcp_accm = 0x00000000;
const int8_t ppp_lcp_opt_index[LCP_OPT_DB_NUM_OPTIONS] =
{
    -1,     /* 0: Invalid option. */
    0,      /* 1: MRU. */
    1,      /* 2: ACCM. */
    -1,     /* 3: Invalid option. */
    -1,     /* 4: Invalid option. */
    2,      /* 5: MAGIC. */
    -1,     /* 6: Invalid option. */
    3,      /* 8: ACFC. */
    4,      /* 7: PFC. */
};

const uint8_t ppp_lcp_option_valid_lengths[LCP_OPT_DB_NUM_OPTIONS_VALID] =
{
    0x04,   /* 1: MRU. */
    0x06,   /* 2: ACCM. */
    0x06,   /* 5: MAGIC. */
    0x02,   /* 8: ACFC. */
    0x02,   /* 7: PFC. */
};

const uint8_t *ppp_lcp_option_values[LCP_OPT_DB_NUM_OPTIONS_VALID] =
{
    NULL,                           /* 1: MRU. */
    (uint8_t *)&ppp_lcp_accm,       /* 2: ACCM. */
    (uint8_t *)LCP_OPT_RANDOM,      /* 5: MAGIC. */
    (uint8_t *)LCP_OPT_NO_VALUE,    /* 8: ACFC. */
    (uint8_t *)LCP_OPT_NO_VALUE,    /* 7: PFC. */
};

/*
 * ppp_lcp_state_initialize
 * @ppp: PPP instance needed to be updated.
 * This function will initialize LCP state for a PPP instance.
 */
void ppp_lcp_state_initialize(PPP *ppp)
{
    /* Initialize PPP connection state. */
    ppp->lcp_flags = 0;
    ppp->rx_accm = (0xFFFFFFFF);
    ppp->tx_accm[0] = (0xFFFFFFFF);
    ppp->tx_accm[1] = (0x0);
    ppp->tx_accm[2] = (0x0);
    ppp->tx_accm[3] = (0x60000000);

} /* ppp_lcp_configuration_add */

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
    int32_t random, status = SUCCESS;
    uint8_t i, opt_len;
    uint8_t *db_value;

    for (i = 0; (status == SUCCESS) && (i < LCP_OPT_DB_NUM_OPTIONS); i++)
    {
        /* If we need to send this option. */
        if (((1 << i) & PPP_LCP_OPTION_SEND_MASK))
        {
            /* Pick up the option value and lengths needed to be send. */
            db_value = (uint8_t *)ppp_lcp_option_values[ppp_lcp_opt_index[i]];
            opt_len = ppp_lcp_option_valid_lengths[ppp_lcp_opt_index[i]];

            /* Check if this option takes a random value. */
            if (db_value == (const uint8_t *)(LCP_OPT_RANDOM))
            {
                /* Generate and put a random value. */
                random = (int32_t)current_system_tick();
                fs_memcpy_r((char *)option.data, (char *)&random, (uint32_t)(opt_len - 2));
            }

            /* Check if we have specified a value for this option. */
            else if (db_value != NULL)
            {
                /* Copy the given value in the option buffer. */
                fs_memcpy_r((char *)option.data, (char *)db_value, (uint32_t)(opt_len - 2));
            }

            else if (db_value != (const uint8_t *)(LCP_OPT_NO_VALUE))
            {
                /* Should not happen return an error. */
                status = PPP_INTERNAL_ERROR;
            }

            if (status == SUCCESS)
            {
                /* Initialize the option needed to be send. */
                option.type = i;
                option.length = opt_len;

                /* Add this option in the transmit buffer. */
                status = ppp_packet_configuration_option_add(buffer, &option);
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_lcp_configuration_add */

/*
 * ppp_lcp_option_negotiable
 * @ppp: PPP private data.
 * @option: Option needed to be checked.
 * @return: Returns true if the given option is supported, otherwise false will
 *  be returned.
 * This function will return if a given option type for LCP is negotiable or
 * not.
 */
uint8_t ppp_lcp_option_negotiable(PPP *ppp, PPP_PKT_OPT *option)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* For now all the options in LCP can be mapped on a 32-bit integer so a
     * option mask is used here to check if do support a given option. */
    return (((1 << option->type) & PPP_LCP_OPTION_NEG_MASK) ? TRUE: FALSE);

} /* ppp_lcp_option_negotiable */

/*
 * ppp_lcp_option_pocess
 * @ppp: PPP private data.
 * @option: Option needed to be process.
 * @rx_packet: Parsed PPP packet.
 * @return: Always return success.
 * This function will process the data for a given option.
 */
int32_t ppp_lcp_option_pocess(PPP *ppp, PPP_PKT_OPT *option, PPP_PKT *rx_packet)
{
    /* Process the option data. */
    switch (option->type)
    {
    /* Maximum receive unit. */
    case (PPP_LCP_OPT_MRU):

        /* If we have received an ACK for this configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Save the received MRU value. */
            fs_memcpy_r((char *)&(ppp->mru), (char *)option->data, (uint32_t)(option->length - 2));
        }

        /* Break out of this switch. */
        break;

    /* Asynchronous control character map. */
    case (PPP_LCP_OPT_ACCM):

        /* If we are accepting the configuration. */
        if (rx_packet->code == PPP_CONFIG_REQ)
        {
            /* Pull the ACCM in PPP configuration as receive ACCM. */
            fs_memcpy_r((char *)&(ppp->rx_accm), (char *)option->data, (uint32_t)(option->length - 2));
        }

        /* If we have received an ACK for this configuration. */
        else if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Pull the ACCM in PPP configuration as transmit ACCM. */
            fs_memcpy_r((char *)&(ppp->tx_accm[0]), (char *)option->data, (uint32_t)(option->length - 2));
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
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Set the flag in PPP structure that we might
             * receive compressed protocol field. */
            ppp->lcp_flags |= PPP_FLAG_PFC;
        }

        break;

    /* Address and control field compression. */
    case (PPP_LCP_OPT_ACFC):

        /* If we have received an ACK for this configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Set the flag in PPP structure that we might
             * receive compressed address and control fields. */
            ppp->lcp_flags |= PPP_FALG_ACFC;
        }

        break;

    default:
        /* Nothing to do here. */
        break;
    }

    /* Always return success. */
    return (SUCCESS);

} /* ppp_lcp_option_pocess */

/*
 * ppp_lcp_option_length_valid
 * @ppp: PPP private data.
 * @option: Option for which valid length is needed.
 * @return: Returns true of option length is valid.
 * This function will provide valid length of a given option type.
 */
uint8_t ppp_lcp_option_length_valid(PPP *ppp, PPP_PKT_OPT *option)
{
    uint8_t valid = FALSE;
    uint8_t opt_len;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);
    if ((option->type < LCP_OPT_DB_NUM_OPTIONS) &&
        (ppp_lcp_opt_index[option->type] != -1))
    {
        /* All supported LCP options have a static length so we will just check our
         * static database if the given option has valid length. */
        opt_len = ppp_lcp_option_valid_lengths[ppp_lcp_opt_index[option->type]];

        /* If option length is valid. */
        if (option->length == opt_len)
        {
            /* Option length is valid. */
            valid = TRUE;
        }
    }

    /* Return if the given option is valid. */
    return (valid);

} /* ppp_lcp_option_length_valid */

/*
 * ppp_lcp_update
 * @fd: File descriptor on which a packet was received.
 * @ppp: PPP private data.
 * @rx_packet: Parsed PPP packet header that was received.
 * @tx_packet: Parsed PPP packet that was sent in the reply.
 * @return: Returns success if operation was successful.
 * This function will be called when a LCP configuration packet is processed
 * and a rely has already sent and internal state is needed to be updated.
 */
int32_t ppp_lcp_update(void *fd, PPP *ppp, PPP_PKT *rx_packet, PPP_PKT *tx_packet)
{
    int32_t status = SUCCESS;
    FS_BUFFER *tx_buffer = NULL;

    /* Check if we have received a request and we have sent an ACK in
     * response, send our own configuration. */
    if ( (rx_packet->code == PPP_CONFIG_REQ) &&
         (tx_packet->code == PPP_CONFIG_ACK) )
    {
        /* Pull a buffer from the free list to use when we are building
         * the response. */
        tx_buffer = fs_buffer_get(fd, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);

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
            tx_packet->id = ++(ppp->state_data.lcp_id);

            /* Add configuration options we need to send. */
            status = ppp_lcp_configuration_add(tx_buffer);

            if (status == SUCCESS)
            {
                /* Push the PPP header on the buffer. */
                status = ppp_packet_configuration_header_add(tx_buffer, tx_packet);

                if (status == SUCCESS)
                {
                    /* Add PPP protocol. */
                    status = ppp_packet_protocol_add(tx_buffer, PPP_PROTO_LCP, PPP_IS_PFC_VALID(ppp));
                }

                if (status == SUCCESS)
                {
                    /* Add the HDLC header. */
                    status = ppp_hdlc_header_add(tx_buffer, ppp->tx_accm, PPP_IS_ACFC_VALID(ppp), TRUE);
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
    else if (rx_packet->code == PPP_CONFIG_ACK)
    {
        /* We have now received an ACK so our link is now established. */
        /* Now start network configuration. */
        ppp->state = PPP_STATE_IPCP;
        ppp->state_data.ipcp_id = 0;

        /* Clear the assigned IP addresses. */
        ppp->local_ip_address = ppp->remote_ip_address = 0;
    }

    /* Check if connection was terminated. */
    else if (rx_packet->code == PPP_TREM_REQ)
    {
        /* This puts us back to LCP configuration stage. */
        ppp->state = PPP_STATE_LCP;
        ppp_lcp_state_initialize(ppp);

        /* Move to the connected state. */
        ppp->state = PPP_STATE_CONNECTED;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_lcp_update */

#endif /* CONFIG_PPP */