/*
 * ppp.h
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
#ifndef _PPP_H_
#define _PPP_H_

#include <os.h>

#ifdef CONFIG_PPP
#include <ppp_fcs.h>
#include <ppp_packet.h>

/* PPP configuration. */
#define PPP_MODEM_CHAT
#define PPP_HDLC

/* Status codes. */
#define PPP_NOT_CONNECTED           -1000
#define PPP_INVALID_STATE           -1001
#define PPP_DONT_CONSUME            -1002
#define PPP_INVALID_HEADER          -1003
#define PPP_NOT_SUPPORTED           -1004
#define PPP_VALUE_NOT_SUPPORTED     -1005
#define PPP_NO_REPLY                -1006
#define PPP_NO_NEXT_OPTION          -1007
#define PPP_NO_BUFFERS              -1008
#define PPP_NO_SPACE                -1009
#define PPP_PHASE_SUCCESS           -1010
#define PPP_INTERNAL_ERROR          -1011

/* PPP instance states. */
#define PPP_STATE_CONNECTED         1
#define PPP_STATE_LCP               2
#define PPP_STATE_NCP               3
#define PPP_STATE_DISCONNECTED      4

/* PPP configuration flags. */
#define PPP_FALG_ACFC               0x01
#define PPP_FLAG_PFC                0x02

/* ACFC and PFC helper macros. */
#define PPP_IS_ACFC_VALID(ppp)      (((ppp)->flags & PPP_LCP_OPT_ACFC) && ((ppp)->state != PPP_STATE_LCP))
#define PPP_IS_PFC_VALID(ppp)       (((ppp)->flags & PPP_FLAG_PFC) && ((ppp)->state != PPP_STATE_LCP))

/* LCP code definitions. */
#define PPP_LCP_CONFIG_NONE         0
#define PPP_LCP_CONFIG_REQ          1
#define PPP_LCP_CONFIG_ACK          2
#define PPP_LCP_CONFIG_NAK          3
#define PPP_LCP_CONFIG_REJECT       4
#define PPP_LCP_TREM_REQ            5
#define PPP_LCP_TREM_ACK            6
#define PPP_LCP_CODE_REJECT         7
#define PPP_LCP_PROTO_REJECT        8
#define PPP_LCP_ECHO_REQ            9
#define PPP_LCP_ECHO_REP            10
#define PPP_LCP_DIS_REQ             11

/* LCP option definitions. */
#define PPP_LCP_OPT_MRU             1
#define PPP_LCP_OPT_ACCM            2
#define PPP_LCP_OPT_AUTH_PROTO      3
#define PPP_LCP_OPT_QUAL_PROTO      4
#define PPP_LCP_OPT_MAGIC           5
#define PPP_LCP_OPT_PFC             7
#define PPP_LCP_OPT_ACFC            8

/* NCP option definitions. */
#define PPP_NCP_OPT_ADDRESSES       1
#define PPP_NCP_OPT_COMP            2
#define PPP_NCP_OPT_ADDRESS         3

/* PPP protocol definitions. */
#define PPP_FLAG                    (0x7E)
#define PPP_ADDRESS                 (0xFF)
#define PPP_CONTROL                 (0x03)

/* PPP packet type definitions. */
#define PPP_PROTO_LCP               (0xC021)
#define PPP_PROTO_NCP               (0x8021)

/* PPP authentication protocol definitions. */
#define PPP_AUTH_PAP                (0xC023)
#define PPP_AUTH_CHAP               (0xC223)

/* PPP option DB configuration. */
#define PPP_OPT_DB_NUM_OPTIONS      (9)
#define LCP_OPT_RANDOM              (-1)
#define LCP_OPT_NO_VALUE            (-2)

typedef struct _ppp_lcp_opt
{
    const uint8_t   *value;
    uint8_t         length;
    uint8_t         do_send;
    uint8_t         pad[2];
} PPP_LCP_OPT;

/* Supported option definition. */
/* Each bit specifies one option type starting from 0 at LSb-0. */
#define PPP_LCP_OPTION_MASK         (0x000001A6)

/* PPP instance data. */
typedef struct _ppp_data
{
#ifdef CONFIG_SEMAPHORE
    SEMAPHORE   lock;
#endif

    /* File system watchers. */
    FS_DATA_WATCHER         data_watcher;
    FS_CONNECTION_WATCHER   connection_watcher;

    uint8_t     magic_number[4];

    /* PPP instance state. */
    uint32_t    state;

    /* PPP configuration flags. */
    uint32_t    flags;

    /* ACCM definitions. */
    uint32_t    tx_accm[8];
    uint32_t    rx_accm;

    /* MRU value. */
    uint32_t    mru;

    union _ppp_state_data
    {
        /* ID used in last LCP request. */
        uint8_t     lcp_id;

        /* ID used in last NCP request. */
        uint8_t     ncp_id;
    } state_data;

    /* Structure padding. */
    uint8_t     pad[3];
} PPP;

/* Function prototypes. */
void ppp_register_fd(PPP *, FD fd);
void ppp_connection_established(void *, void *);
void ppp_connection_terminated(void *, void *);
void ppp_rx_watcher(void *, void *);
void ppp_tx_watcher(void *, void *);

/* PPP internal APIs. */
uint32_t ppp_get_buffer_head_room(PPP *);
void ppp_process_modem_chat(void *, PPP *);
int32_t ppp_lcp_configuration_add(FS_BUFFER *);
void ppp_process_configuration(void *, PPP *);
void ppp_lcp_configuration_process(void *, PPP *, FS_BUFFER *);
void ppp_ncp_configuration_process(void *, PPP *, FS_BUFFER *);

#ifdef PPP_MODEM_CHAT
#include <modem_chat.h>
#endif
#ifdef PPP_HDLC
#include <hdlc.h>
#endif

#endif /* CONFIG_PPP */

#endif /* _PPP_H_ */
