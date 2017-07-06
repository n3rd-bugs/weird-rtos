/*
 * ppp_ipcp.h
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
#ifndef _PPP_IPCP_H_
#define _PPP_IPCP_H_

#include <kernel.h>

#ifdef CONFIG_PPP

/* PPP IP configurations. */
#define PPP_LOCAL_IP_ADDRESS    {192, 168, 0, 1}
#define PPP_REMOTE_IP_ADDRESS   {192, 168, 0, 2}

/* IPCP option type definitions. */
#define PPP_IPCP_OPT_IPS        1
#define PPP_IPCP_OPT_COMP       2
#define PPP_IPCP_OPT_IP         3

/* Exported variables. */
extern PPP_PROTO ppp_proto_ipcp;

/* Function prototypes. */
uint8_t ppp_ipcp_option_negotiable(PPP *, PPP_CONF_OPT *);
int32_t ppp_ipcp_option_pocess(PPP *, PPP_CONF_OPT *, PPP_CONF_PKT *);
uint8_t ppp_ipcp_option_length_valid(PPP *, PPP_CONF_OPT *);
int32_t ppp_ipcp_update(void *, PPP *, PPP_CONF_PKT *, PPP_CONF_PKT *);

#endif /* CONFIG_PPP */

#endif /* _PPP_IPCP_H_ */
