/*
 * ppp.h
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
#ifndef _PPP_H_
#define _PPP_H_

#include <kernel.h>

#ifdef IO_PPP
#ifndef CONFIG_NET
#error "PPP requires networking stack."
#endif
#include <ppp_fcs.h>
#include <ppp_packet.h>
#include <ppp_hdlc.h>
#include <net.h>
#include <ppp_config.h>

/* Status codes. */
#define PPP_INVALID_HEADER          -900
#define PPP_NOT_SUPPORTED           -901
#define PPP_VALUE_NOT_VALID         -902
#define PPP_PARTIAL_READ            -903
#define PPP_BUFFER_FORWARDED        -904
#define PPP_NO_NEXT_OPTION          -905
#define PPP_NO_BUFFERS              -906
#define PPP_NO_SPACE                -907
#define PPP_INVALID_PROTO           -908
#define PPP_INVALID_FD              -909
#define PPP_INTERNAL_ERROR          -910

/* PPP instance states. */
#define PPP_STATE_INIT              1
#define PPP_STATE_LCP               2
#define PPP_STATE_IPCP              3
#define PPP_STATE_NETWORK           4

/* PPP configuration flags. */
#define PPP_FLAG_ACFC               0x1
#define PPP_FLAG_PFC                0x2
#define PPP_DEDICATED_FD            0x4

/* ACFC and PFC helper macros. */
#define PPP_IS_ACFC_VALID(ppp)      (((ppp)->flags & PPP_FLAG_ACFC) != 0)
#define PPP_IS_PFC_VALID(ppp)       (((ppp)->flags & PPP_FLAG_PFC) != 0)

/* LCP code definitions. */
#define PPP_CONFIG_NONE             0
#define PPP_CONFIG_REQ              1
#define PPP_CONFIG_ACK              2
#define PPP_CONFIG_NAK              3
#define PPP_CONFIG_REJECT           4
#define PPP_TREM_REQ                5
#define PPP_TREM_ACK                6
#define PPP_CODE_REJECT             7
#define PPP_PROTO_REJECT            8
#define PPP_ECHO_REQ                9
#define PPP_ECHO_REP                10
#define PPP_DIS_REQ                 11

/* PPP protocol definitions. */
#define PPP_ADDRESS                 (0xFF)
#define PPP_CONTROL                 (0x3)

/* PPP protocol definitions. */
#define PPP_PROTO_IPV4              (0x21)
#define PPP_PROTO_IPCP              (0x8021)
#define PPP_PROTO_LCP               (0xC021)
#define PPP_AUTH_PAP                (0xC023)
#define PPP_AUTH_CHAP               (0xC223)

/* PPP instance structure. */
typedef struct _ppp PPP;
struct _ppp
{
    /* PPP instance list member. */
    PPP             *next;

    /* File descriptor registered with PPP device. */
    FD              fd;

    /* Networking device structure. */
    NET_DEV         net_device;

    /* PPP instance state. */
    uint32_t        state;

    /* PPP configuration flags. */
    uint32_t        flags;

    /* ACCM definitions. */
    uint32_t        tx_accm[8];
    uint32_t        rx_accm;

    /* MRU value. */
    uint32_t        mru;

    /* Configured IP addresses. */
    uint32_t        remote_ip_address;
    uint32_t        local_ip_address;

    /* Receive buffer chain. */
    FS_BUFFER_LIST  *rx_buffer;

    union _ppp_state_data
    {
        /* ID used in last LCP request. */
        uint8_t         lcp_id;

        /* ID used in last IPCP request. */
        uint8_t         ipcp_id;
    } state_data;

    /* Structure padding. */
    uint8_t         pad[3];
};

/* PPP global data structure. */
typedef struct _ppp_data
{
    /* PPP instance list. */
    struct _ppp_list
    {
        PPP     *head;
        PPP     *tail;
    } ppp;
} PPP_DATA;

/* PPP protocol definition. */
typedef struct _ppp_proto
{
    /* PPP configuration protocol callbacks. */
    uint8_t (*negotiable)(PPP *, PPP_CONF_OPT *);
    uint8_t (*length_valid)(PPP *, PPP_CONF_OPT *);
    int32_t (*process)(PPP *, PPP_CONF_OPT *, PPP_CONF_PKT *);
    int32_t (*update)(void *, PPP *, PPP_CONF_PKT *, PPP_CONF_PKT *);

    /* PPP protocol definition. */
    uint16_t protocol;

    /* Structure padding. */
    uint8_t pad[2];
} PPP_PROTO;

/* Function prototypes. */
void ppp_init(void);
void ppp_register_fd(PPP *, FD, uint8_t);
PPP *ppp_get_instance_fd(FD);
void ppp_connection_established(void *, void *);
void ppp_connection_terminated(void *, void *);
void ppp_rx_watcher(void *, void *);

/* PPP internal APIs. */
void ppp_process_modem_chat(void *, PPP *);
void ppp_configuration_process(PPP *, FS_BUFFER_LIST *, PPP_PROTO *);
void ppp_process_frame(void *, PPP *);
int32_t net_ppp_transmit(FS_BUFFER_LIST *, uint8_t);
void net_ppp_receive(void *, int32_t);
int32_t ppp_transmit_buffer_instance(PPP *, FS_BUFFER_LIST *, uint16_t, uint8_t);

/* Include PPP supported configuration protocol definitions. */
#include <ppp_lcp.h>
#include <ppp_ipcp.h>

#ifdef PPP_MODEM_CHAT
#include <modem_chat.h>
#endif

#endif /* IO_PPP */
#endif /* _PPP_H_ */
