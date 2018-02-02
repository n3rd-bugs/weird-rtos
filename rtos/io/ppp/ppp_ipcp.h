/*
 * ppp_ipcp.h
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
#ifndef _PPP_IPCP_H_
#define _PPP_IPCP_H_

#include <kernel.h>

#ifdef CONFIG_PPP

/* PPP IP configurations. */
#ifdef CMAKE_BUILD
#include <ppp_ipcp_config.h>
#else
#define PPP_LOCAL_IP_ADDRESS    {192, 168, 0, 1}
#define PPP_REMOTE_IP_ADDRESS   {192, 168, 0, 2}
#endif /* CMAKE_BUILD */

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
