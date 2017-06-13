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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <os.h>

#ifdef CONFIG_PPP
#include <string.h>
#include <fs.h>
#include <ppp.h>
#include <ppp_packet.h>
#include <net.h>
#include <net_ipv4.h>
#include <net_route.h>

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
uint8_t ppp_ipcp_option_negotiable(PPP *ppp, PPP_CONF_OPT *option)
{
    uint8_t negotiable = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* For now only IP address option is negotiable. */
    if (option->type == PPP_IPCP_OPT_IP)
    {
        /* This is negotiable. */
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
 *  valid option value is returned in the option, PPP_NOT_SUPPORTED will be
 *  returned if option type is not supported.
 * This function will process the data for a given option.
 */
int32_t ppp_ipcp_option_pocess(PPP *ppp, PPP_CONF_OPT *option, PPP_CONF_PKT *rx_packet)
{
    int32_t status = PPP_NOT_SUPPORTED;

    /* If we have a IP option. */
    if (option->type == PPP_IPCP_OPT_IP)
    {
        /* If this is a configuration request. */
        if (rx_packet->code == PPP_CONFIG_REQ)
        {
            /* If remote has a different IP address. */
            if (memcmp(option->data, (uint8_t [])PPP_REMOTE_IP_ADDRESS, (uint32_t)(option->length - 2)))
            {
                /* Whatever IP address was given by the other end, overwrite it
                 * with our configured IP address. */
                memcpy(option->data, (uint8_t [])PPP_REMOTE_IP_ADDRESS, (uint32_t)(option->length - 2));

                /* Tell the other end to use this IP address. */
                status = PPP_VALUE_NOT_VALID;
            }

            /* Remote has same IP address. */
            else
            {
#ifdef OS_LITTLE_ENDIAN
                /* Copy the configured remote IP address in the PPP structure. */
                fs_memcpy_r(&ppp->remote_ip_address, option->data, (uint32_t)(option->length - 2));
#else
                /* Copy the configured remote IP address in the PPP structure. */
                memcpy(&ppp->remote_ip_address, option->data, (uint32_t)(option->length - 2));
#endif

                /* Return success. */
                status = SUCCESS;
            }
        }

        /* If this is a ACK for our configuration. */
        if (rx_packet->code == PPP_CONFIG_ACK)
        {
#ifdef OS_LITTLE_ENDIAN
            /* Copy the configured local IP address in the PPP structure. */
            fs_memcpy_r(&ppp->local_ip_address, option->data, (uint32_t)(option->length - 2));
#else
            /* Copy the configured local IP address in the PPP structure. */
            memcpy(&ppp->local_ip_address, option->data, (uint32_t)(option->length - 2));
#endif

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
uint8_t ppp_ipcp_option_length_valid(PPP *ppp, PPP_CONF_OPT *option)
{
    uint8_t valid = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);

    /* We only support IP option for now. */
    if ((option->type == PPP_IPCP_OPT_IP) && (option->length == 6))
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
int32_t ppp_ipcp_update(void *fd, PPP *ppp, PPP_CONF_PKT *rx_packet, PPP_CONF_PKT *tx_packet)
{
    int32_t status = SUCCESS;
    PPP_CONF_OPT option;
    FS_BUFFER *tx_buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);

    /* Should never happen. */
    OS_ASSERT(tx_buffer == NULL);

    /* If we are sending an NAK it means we have the required configuration
     * options and we can now send our configuration in reply. */
    if (tx_packet->code == PPP_CONFIG_NAK)
    {
        /* If we have not received an ACK for our configuration. */
        if (ppp->local_ip_address == 0)
        {
            /* Clear the transmit packet and buffer chain structures. */
            memset(tx_packet, 0, sizeof(PPP_CONF_PKT));

            /* We have successfully ACKed a configuration request we will send our
             * configuration now.. */
            tx_packet->code = PPP_CONFIG_REQ;
            tx_packet->id = ++(ppp->state_data.ipcp_id);

            /* Add IP configuration option in the transmit buffer. */
            memcpy(option.data, (uint8_t [])PPP_LOCAL_IP_ADDRESS, 4);
            option.type = PPP_IPCP_OPT_IP;
            option.length = 6;
            status = ppp_packet_configuration_option_add(tx_buffer, &option);

            /* If configuration option was successfully added. */
            if (status == SUCCESS)
            {
                /* Push the PPP header on the buffer. */
                status = ppp_packet_configuration_header_add(tx_buffer, tx_packet);

                /* If configuration header was successfully added. */
                if (status == SUCCESS)
                {
                    /* Send this buffer. */
                    status = ppp_transmit_buffer_instance(ppp, tx_buffer, PPP_PROTO_IPCP, 0);
                }
            }
        }
    }

    /* IPCP configuration is being ACKed. */
    if (rx_packet->code == PPP_CONFIG_ACK)
    {
        /* We are now in network phase. */
        ppp->state = PPP_STATE_NETWORK;

        /* Release lock for this file descriptor. */
        fd_release_lock(fd);

        /* Set the IPv4 address for this device. */
        OS_ASSERT(ipv4_set_device_address(fd, ppp->local_ip_address, IPV4_SUBNET_LL) != SUCCESS);

        /* Add a route for remote address. */
        route_add(fd, ppp->local_ip_address, ppp->remote_ip_address, ppp->remote_ip_address, IPV4_SUBNET_LL, 0);

        /* Acquire lock for this file descriptor. */
        OS_ASSERT(fd_get_lock(fd) != SUCCESS);

        /* Set link-up for the associated networking device. */
        net_device_link_up(fd);
    }

    /* Free the buffer list allocated before and any buffers still left on it. */
    fs_buffer_add(tx_buffer->fd, tx_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

    /* Return status to the caller. */
    return (status);

} /* ppp_ipcp_update */

#endif /* CONFIG_PPP */
