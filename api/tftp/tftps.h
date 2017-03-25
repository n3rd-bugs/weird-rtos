/*
 * tftps.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _TFTPS_H_
#define _TFTPS_H_
#include <os.h>

#ifdef CONFIG_TFTPS
#ifndef CONFIG_NET
#error "Networking stack required for TFTP server."
#endif
#include <net.h>
#ifndef NET_UDP
#error "UDP stack required for TFTP server."
#endif
#include <net_udp.h>

/* TFTP error code definitions. */
#define TFTP_NOT_SUPPORTED      -22000
#define TFTP_LONG_FILENAME      -22001
#define TFTP_ERROR_FS           -22002
#define TFTP_ERROR_EXHAUSTED    -22003
#define TFTP_UNKNOWN_TID        -22004
#define TFTP_OUTOFBOUND_BLOCK   -22005
#define TFTP_FRAME_DROP         -22006

/* TFTP definitions. */
#define TFTP_OP_READ_REQ        (0x0001)
#define TFTP_OP_WRITE_REQ       (0x0002)
#define TFTP_OP_DATA            (0x0003)
#define TFTP_OP_ACK             (0x0004)
#define TFTP_OP_ERR             (0x0005)

#define TFTP_ERROR_GEN          (0x0000)
#define TFTP_ERROR_TID          (0x0005)

#define TFTP_ERRMSG_NOT_SUPPORTED   "opcode not supported"
#define TFTP_ERRMSG_FILENAME        "filename too long"
#define TFTP_ERRMSG_FS              "error opening specified file"
#define TFTP_ERRMSG_EXHAUSTED       "connections exhausted"
#define TFTP_ERRMSG_TID             "transaction ID is not known for this request"
#define TFTP_ERRMSG_BLOCK           "an out of bound block we received"

#define TFTP_BUFFER_SIZE        (32)
#define TFTP_BLOCK_SIZE         (512)
#define TFTP_CLI_TIMEOUT        (OS_TICKS_PER_SEC)

/* TFTP server structure. */
typedef struct _tftp_server
{
    /* Associated UDP port. */
    UDP_PORT    port;

    /* Timeout for current connection. */
    uint64_t    timeout;

    /* Condition data for processing requests on for this server. */
    SUSPEND     port_suspend;
    CONDITION   *port_condition;
    FS_PARAM    port_fs_param;

    /* Opened file descriptor. */
    FD          fd;

    /* Client address. */
    SOCKET_ADDRESS  client_address;

    /* Current block number. */
    uint16_t    block_num;

    /* Size of block we just sent. */
    uint16_t    tx_block_len;

    /* Flag to specify that we have just sent the last frame. */
    uint8_t     last_block;

} TFTP_SERVER;

/* Function prototypes. */
void tftp_server_init(TFTP_SERVER *, SOCKET_ADDRESS *, char *);

#endif /* CONFIG_TFTPS */
#endif /* _TFTPS_H_ */
