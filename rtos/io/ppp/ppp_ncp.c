/*
 * ppp_ncp.c
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

/* PPP NCP protocol definition. */
PPP_PROTO ppp_proto_ncp =
{
    .negotiable     = &ppp_ncp_option_negotiable,
    .length_valid   = &ppp_ncp_option_length_valid,
    .process        = &ppp_ncp_option_pocess,
    .update         = &ppp_ncp_update,
    .protocol       = PPP_PROTO_NCP,
};

/*
 * ppp_ncp_option_negotiable
 * @ppp: PPP private data.
 * @option: Option needed to be checked.
 * @return: Returns true if the given option is supported, otherwise false will
 *  be returned.
 * This function will return if a given option type for NCP is negotiable or
 * not.
 */
uint8_t ppp_ncp_option_negotiable(PPP *ppp, PPP_PKT_OPT *option)
{
    uint8_t negotiable = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);
    UNUSED_PARAM(option);

    /* Return if this option is negotiable. */
    return (negotiable);

} /* ppp_ncp_option_negotiable */

/*
 * ppp_ncp_option_pocess
 * @ppp: PPP private data.
 * @option: Option needed to be process.
 * @rx_packet: Parsed PPP packet.
 * @return: Always return success.
 * This function will process the data for a given option.
 */
int32_t ppp_ncp_option_pocess(PPP *ppp, PPP_PKT_OPT *option, PPP_PKT *rx_packet)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);
    UNUSED_PARAM(option);
    UNUSED_PARAM(rx_packet);

    /* Always return success. */
    return (SUCCESS);

} /* ppp_ncp_option_pocess */

/*
 * ppp_ncp_option_length_valid
 * @ppp: PPP private data.
 * @option: Option for which valid length is needed.
 * @return: Returns true of option length is valid.
 * This function will provide valid length of a given option type.
 */
uint8_t ppp_ncp_option_length_valid(PPP *ppp, PPP_PKT_OPT *option)
{
    uint8_t valid = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(ppp);
    UNUSED_PARAM(option);

    /* Return if the given option is valid. */
    return (valid);

} /* ppp_ncp_option_length_valid */

/*
 * ppp_ncp_update
 * @fd: File descriptor on which a packet was received.
 * @ppp: PPP private data.
 * @rx_packet: Parsed PPP packet header that was received.
 * @tx_packet: Parsed PPP packet that was sent in the reply.
 * @return: Returns success if operation was successful.
 * This function will be called when a NCP configuration packet is processed
 * and a rely has already sent and internal state is needed to be updated.
 */
int32_t ppp_ncp_update(void *fd, PPP *ppp, PPP_PKT *rx_packet, PPP_PKT *tx_packet)
{
    int32_t status = SUCCESS;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(ppp);
    UNUSED_PARAM(rx_packet);
    UNUSED_PARAM(tx_packet);

    /* Return status to the caller. */
    return (status);

} /* ppp_ncp_update */

#endif /* CONFIG_PPP */
