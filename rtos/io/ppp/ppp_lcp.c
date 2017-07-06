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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>

#ifdef CONFIG_PPP
#include <string.h>
#include <fs.h>
#include <ppp.h>
#include <ppp_packet.h>

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
/* To be negotiated ACCM value. */
const uint32_t ppp_lcp_accm = 0x00000000;

/* PPP option index look-up table. */
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

/* Option valid value look-up table. */
const uint8_t *ppp_lcp_option_values[LCP_OPT_DB_NUM_OPTIONS_VALID] =
{
    NULL,                           /* 1: MRU. */
    (uint8_t *)&ppp_lcp_accm,       /* 2: ACCM. */
    (uint8_t *)LCP_OPT_RANDOM,      /* 5: MAGIC. */
    (uint8_t *)LCP_OPT_NO_VALUE,    /* 8: ACFC. */
    (uint8_t *)LCP_OPT_NO_VALUE,    /* 7: PFC. */
};

/* Option valid length look-up table. */
const uint8_t ppp_lcp_option_valid_lengths[LCP_OPT_DB_NUM_OPTIONS_VALID] =
{
    0x04,   /* 1: MRU. */
    0x06,   /* 2: ACCM. */
    0x06,   /* 5: MAGIC. */
    0x02,   /* 8: ACFC. */
    0x02,   /* 7: PFC. */
};

/*
 * ppp_lcp_state_initialize
 * @ppp: PPP instance needed to be updated.
 * This function will initialize LCP state for a PPP instance.
 */
void ppp_lcp_state_initialize(PPP *ppp)
{
    /* Initialize PPP connection state. */
    ppp->flags &= (uint32_t)~(PPP_FLAG_ACFC | PPP_FLAG_PFC);
    ppp->rx_accm = (0xFFFFFFFF);
    ppp->tx_accm[0] = (0xFFFFFFFF);
    ppp->tx_accm[1] = (0x0);
    ppp->tx_accm[2] = (0x0);
    ppp->tx_accm[3] = (0x60000000);
    ppp->mru = 1500;

} /* ppp_lcp_state_initialize */

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
    PPP_CONF_OPT option;
    int32_t random, status = SUCCESS;
    uint8_t opt_type, opt_len, *db_value;

    /* Check all the possible options and see if we need to send them in our
     * configuration packet. */
    for (opt_type = 0; (status == SUCCESS) && (opt_type < LCP_OPT_DB_NUM_OPTIONS); opt_type++)
    {
        /* If we need to send this option. */
        if (((1 << opt_type) & PPP_LCP_OPTION_SEND_MASK))
        {
            /* Pick up the option value and lengths needed to be send. */
            db_value = (uint8_t *)ppp_lcp_option_values[ppp_lcp_opt_index[opt_type]];
            opt_len = ppp_lcp_option_valid_lengths[ppp_lcp_opt_index[opt_type]];

            /* Check if this option takes a random value. */
            if (db_value == (const uint8_t *)(LCP_OPT_RANDOM))
            {
                /* Generate and put a random value. */
                random = (int32_t)current_system_tick();
                fs_memcpy_r(option.data, &random, (uint32_t)(opt_len - 2));
            }

            /* Check if we have specified a value for this option. */
            else if (db_value != NULL)
            {
                /* Copy the given value in the option buffer. */
#ifdef LITTLE_ENDIAN
                fs_memcpy_r(option.data, db_value, (uint32_t)(opt_len - 2));
#else
                memcpy(option.data, db_value, (uint32_t)(opt_len - 2));
#endif
            }

            else if (db_value != (const uint8_t *)(LCP_OPT_NO_VALUE))
            {
                /* Should not happen return an error. */
                status = PPP_INTERNAL_ERROR;
            }

            /* If option was successfully picked from the option database. */
            if (status == SUCCESS)
            {
                /* Initialize the option needed to be send. */
                option.type = opt_type;
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
uint8_t ppp_lcp_option_negotiable(PPP *ppp, PPP_CONF_OPT *option)
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
int32_t ppp_lcp_option_pocess(PPP *ppp, PPP_CONF_OPT *option, PPP_CONF_PKT *rx_packet)
{
    /* Process the option data. */
    switch (option->type)
    {
    /* Maximum receive unit. */
    case PPP_LCP_OPT_MRU:

        /* If we have received an ACK for this configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Save the received MRU value. */
#ifdef LITTLE_ENDIAN
            fs_memcpy_r(&(ppp->mru), option->data, (uint32_t)(option->length - 2));
#else
            memcpy(&(ppp->mru), option->data, (uint32_t)(option->length - 2));
#endif
        }

        /* Break out of this switch. */
        break;

    /* Asynchronous control character map. */
    case PPP_LCP_OPT_ACCM:

        /* If we are accepting the configuration. */
        if (rx_packet->code == PPP_CONFIG_REQ)
        {
            /* Pull the ACCM in PPP configuration as receive ACCM. */
#ifdef LITTLE_ENDIAN
            fs_memcpy_r(&(ppp->rx_accm), option->data, (uint32_t)(option->length - 2));
#else
            memcpy(&(ppp->rx_accm), option->data, (uint32_t)(option->length - 2));
#endif
        }

        /* If we have received an ACK for this configuration. */
        else if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Pull the ACCM in PPP configuration as transmit ACCM. */
#ifdef LITTLE_ENDIAN
            fs_memcpy_r(&(ppp->tx_accm[0]), option->data, (uint32_t)(option->length - 2));
#else
            memcpy(&(ppp->tx_accm[0]), option->data, (uint32_t)(option->length - 2));
#endif
        }

        break;

    /* Magic number. */
    case PPP_LCP_OPT_MAGIC:

        /* Keep the magic number as it is. */
        /* TODO: If this not configuration request validate this. */
        break;

    /* Protocol field compression. */
    case PPP_LCP_OPT_PFC:

        /* If we have received an ACK for this configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Set the flag in PPP structure that we might
             * receive compressed protocol field. */
            ppp->flags |= PPP_FLAG_PFC;
        }

        break;

    /* Address and control field compression. */
    case PPP_LCP_OPT_ACFC:

        /* If we have received an ACK for this configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
            /* Set the flag in PPP structure that we might
             * receive compressed address and control fields. */
            ppp->flags |= PPP_FLAG_ACFC;
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
uint8_t ppp_lcp_option_length_valid(PPP *ppp, PPP_CONF_OPT *option)
{
    uint8_t valid = FALSE;
    uint8_t opt_len;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* Check if we have a value for this option in our database. */
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
int32_t ppp_lcp_update(void *fd, PPP *ppp, PPP_CONF_PKT *rx_packet, PPP_CONF_PKT *tx_packet)
{
    int32_t status = SUCCESS;
    FS_BUFFER *tx_buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);

    /* Should never happen. */
    ASSERT(tx_buffer == NULL);

    /* Check if we have received a request and we have sent an ACK in
     * response, send our own configuration. */
    if ( (rx_packet->code == PPP_CONFIG_REQ) &&
         (tx_packet->code == PPP_CONFIG_ACK) )
    {
        /* Clear the transmit packet and buffer chain structures. */
        memset(tx_packet, 0, sizeof(PPP_CONF_PKT));

        /* We have successfully ACKed a configuration request we will send our
         * configuration now.. */
        tx_packet->code = PPP_CONFIG_REQ;
        tx_packet->id = ++(ppp->state_data.lcp_id);

        /* Add configuration options we need to send. */
        status = ppp_lcp_configuration_add(tx_buffer);

        /* If LCP configuration options were successfully added. */
        if (status == SUCCESS)
        {
            /* Push the PPP header on the buffer. */
            status = ppp_packet_configuration_header_add(tx_buffer, tx_packet);

            /* If PPP configuration header was successfully added. */
            if (status == SUCCESS)
            {
                /* Send this buffer. */
                status = ppp_transmit_buffer_instance(ppp, tx_buffer, PPP_PROTO_LCP, 0);

                if (status == SUCCESS)
                {
                    /* TX buffer is no longer required. */
                    tx_buffer = NULL;
                }
            }
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

        /* Set the MTU for this networking device. */
        net_device_set_mtu(fd, ppp->mru);
    }

    /* Check if connection was terminated. */
    else if (rx_packet->code == PPP_TREM_REQ)
    {
        /* Set link-down for the associated networking device. */
        net_device_link_down(fd);

        /* This puts us back to LCP configuration stage. */
        ppp->state = PPP_STATE_LCP;
        ppp_lcp_state_initialize(ppp);

        /* Move to the connected state. */
        ppp->state = PPP_STATE_INIT;
    }

    /* If we still have TX buffer. */
    if (tx_buffer != NULL)
    {
        /* Free the TX buffer. */
        fs_buffer_add(tx_buffer->fd, tx_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_lcp_update */

#endif /* CONFIG_PPP */
