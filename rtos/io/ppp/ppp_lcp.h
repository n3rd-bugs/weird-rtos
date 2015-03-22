/*
 * ppp_lcp.h
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
#ifndef _PPP_LCP_H_
#define _PPP_LCP_H_

#include <os.h>

#ifdef CONFIG_PPP

/* LCP option type definitions. */
#define PPP_LCP_OPT_MRU                 1
#define PPP_LCP_OPT_ACCM                2
#define PPP_LCP_OPT_AUTH_PROTO          3
#define PPP_LCP_OPT_QUAL_PROTO          4
#define PPP_LCP_OPT_MAGIC               5
#define PPP_LCP_OPT_PFC                 7
#define PPP_LCP_OPT_ACFC                8

/* PPP option DB configuration. */
#define LCP_OPT_DB_NUM_OPTIONS          (9)
#define LCP_OPT_DB_NUM_OPTIONS_VALID    (5)
#define LCP_OPT_RANDOM                  (-1)
#define LCP_OPT_NO_VALUE                (-2)

/* Supported option definition. */
/* Each bit specifies one option type starting from 0 at LSb-0. */
#define PPP_LCP_OPTION_NEG_MASK     (0x000001A6)
#define PPP_LCP_OPTION_SEND_MASK    (0x000001A4)

/* Exported variables. */
extern PPP_PROTO ppp_proto_lcp;

/* Function prototypes. */
void ppp_lcp_state_initialize(PPP *);
int32_t ppp_lcp_configuration_add(FS_BUFFER *);
uint8_t ppp_lcp_option_negotiable(PPP *, PPP_CONF_OPT *);
int32_t ppp_lcp_option_pocess(PPP *, PPP_CONF_OPT *, PPP_CONF_PKT *);
uint8_t ppp_lcp_option_length_valid(PPP *, PPP_CONF_OPT *);
int32_t ppp_lcp_update(void *, PPP *, PPP_CONF_PKT *, PPP_CONF_PKT *);

#endif /* CONFIG_PPP */

#endif /* _PPP_LCP_H_ */
