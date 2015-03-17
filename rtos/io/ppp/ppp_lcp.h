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

/* PPP option DB configuration. */
#define LCP_OPT_DB_NUM_OPTIONS      (9)
#define LCP_OPT_RANDOM              (-1)
#define LCP_OPT_NO_VALUE            (-2)

/* Supported option definition. */
/* Each bit specifies one option type starting from 0 at LSb-0. */
#define PPP_LCP_OPTION_MASK         (0x000001A6)

/* PPP LCP option database structure. */
typedef struct _ppp_lcp_opt
{
    const uint8_t   *value;     /* Option value that will be send. */
    uint8_t         length;     /* Valid option length. */
    uint8_t         do_send;    /* If this option is needed to be sent. */
    uint8_t         pad[2];
} PPP_LCP_OPT;

/* Exported variables. */
extern PPP_PROTO ppp_proto_lcp;

/* Function prototypes. */
void ppp_lcp_state_initialize(PPP *);
int32_t ppp_lcp_configuration_add(FS_BUFFER *);
uint8_t ppp_lcp_option_negotiable(PPP *, PPP_PKT_OPT *);
int32_t ppp_lcp_option_pocess(PPP *, PPP_PKT_OPT *, PPP_PKT *);
uint8_t ppp_lcp_option_length_valid(PPP *, PPP_PKT_OPT *);
int32_t ppp_lcp_update(void *, PPP *, PPP_PKT *, PPP_PKT *);

#endif /* CONFIG_PPP */

#endif /* _PPP_LCP_H_ */
