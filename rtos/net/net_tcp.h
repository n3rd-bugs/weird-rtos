/*
 * net_tcp.h
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
#ifndef _NET_TCP_H_
#define _NET_TCP_H_

#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_TCP
#include <console.h>

/* TCP configuration. */
#define TCP_WND_SIZE                (1024)
#define TCP_WND_SCALE               (2)
#define TCP_RTO                     (1 * OS_TICKS_PER_SEC)
#define TCP_INIT_SS_THRESH          (512)
#define TCP_INIT_CWND               (128)

/* TCP header definitions. */
#define TCP_HRD_SIZE                (20)
#define TCP_HRD_SRC_PORT_OFFSET     (0)
#define TCP_HRD_DST_PORT_OFFSET     (2)
#define TCP_HRD_SEQ_NUM_OFFSET      (4)
#define TCP_HRD_ACK_NUM_OFFSET      (8)
#define TCP_HRD_FLAGS_OFFSET        (12)
#define TCP_HRD_WND_SIZE_OFFSET     (14)
#define TCP_HRD_CSUM_OFFSET         (16)
#define TCP_HRD_URG_OFFSET          (18)

/* TCP flag definitions. */
#define TCP_HDR_FLAG_MSK            (0x0FFF)
#define TCP_HDR_FLAG_FIN            (0x0001)
#define TCP_HDR_FLAG_SYN            (0x0002)
#define TCP_HDR_FLAG_RST            (0x0004)
#define TCP_HDR_FLAG_PSH            (0x0008)
#define TCP_HDR_FLAG_ACK            (0x0010)
#define TCP_HDR_FLAG_URG            (0x0020)
#define TCP_HDR_FLAG_ECE            (0x0040)
#define TCP_HDR_FLAG_CWR            (0x0080)
#define TCP_HDR_FLAG_NS             (0x0100)
#define TCP_HDR_HDR_LEN_MSK         (0xF000)
#define TCP_HDR_HDR_LEN_SHIFT       (12)

/* TCP option definitions. */
#define TCP_OPT_END                 (0)
#define TCP_OPT_NOP                 (1)
#define TCP_OPT_MSS                 (2)
#define TCP_OPT_WIND_SCALE          (3)
#define TCP_OPT_SACK_EN             (4)
#define TCP_OPT_TIME_STAMP          (5)

/* TCP socket states. */
#define TCP_SOCK_COLSED             (0)
#define TCP_SOCK_LISTEN             (1)
#define TCP_SOCK_SYN_SENT           (2)
#define TCP_SOCK_SYN_RCVD           (3)
#define TCP_SOCK_ESTAB              (4)
#define TCP_SOCK_CLOSE_WAIT         (5)
#define TCP_SOCK_FIN_WAIT_1         (6)
#define TCP_SOCK_FIN_WAIT_2         (7)
#define TCP_SOCK_LAST_ACK           (8)
#define TCP_SOCK_TIME_WAIT          (9)

/* TCP socket flags. */
#define TCP_FLAG_WND_SCALE          (0x01)
#define TCP_FLAG_MSS                (0x02)

/* TCP out-of-order parameter flags. */
#define TCP_FLAG_SEG_CONFLICT       (0x01)

/* TCP retransmission packet structure. */
typedef struct _tcp_rtx
{
    /* Networking condition data for this TCP socket. */
    CONDITION       condition;
    SUSPEND         suspend;

    /* Packet data, for now we only support retransmission of one segment at a
     * time. */
    /* Socket address on which this packet is needed to be sent. */
    SOCKET_ADDRESS  *socket_address;

    /* Sequence number to be sent. */
    uint32_t        seq_num;

    /* Sequence number to be ACKed. */
    uint32_t        ack_num;

    /* TCP flags. */
    uint16_t        flags;

    /* TCP window size to be sent. */
    uint16_t        wnd_size;

} TCP_RTX;

/* Parameter that will be used to process the out-of-order buffer list. */
typedef struct _tcp_oo_param
{
    /* Sequence data for the buffer segment we need to insert. */
    uint32_t    seg_seq;
    uint32_t    seg_len;

    /* Buffer operation flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];

} TCP_OO_PARAM;

/* TCP port structure. */
typedef struct _tcp_port TCP_PORT;
struct _tcp_port
{
    /* Console structure for this TCP port. */
    CONSOLE         console;

    /* TCP port list member. */
    TCP_PORT        *next;

    /* TCP buffer lists. */
    struct _tcp_port_buffer_list
    {
        FS_BUFFER       *head;
        FS_BUFFER       *tail;
    } buffer_list;

    /* Structure to maintain TCP RX buffer data. */
    struct _tcp_port_rx_buffer
    {
        /* TCP out of order buffer lists. */
        struct _tcp_port_oorx_list
        {
            FS_BUFFER       *head;
            FS_BUFFER       *tail;
        } oorx_list;

        /* TCP buffer list to be served to application. */
        FS_BUFFER       *buffer;
    } rx_buffer;

    /* TCP socket address. */
    SOCKET_ADDRESS  socket_address;

    /* These variables maintain the socket state and configurations. */

    /* Retransmission data. */
    TCP_RTX         rtx_data;

    /* Receive sequence numbers. */
    uint32_t        irs;
    uint32_t        rcv_nxt;
    uint32_t        rcv_wnd;

    /* Send sequence numbers. */
    uint32_t        iss;
    uint32_t        snd_nxt;
    uint32_t        snd_una;
    uint32_t        snd_wnd;

    /* Maximum segment size. */
    uint16_t        mss;

    /* Slow start threshold. */
    uint16_t        ssthresh;

    /* Congestion window size. */
    uint16_t        cwnd;

    /* TCP window configuration. */
    uint8_t         rcv_wnd_scale;
    uint8_t         snd_wnd_scale;

    /* TCP socket state. */
    uint8_t         state;

    /* TCP socket flags. */
    uint8_t         flags;

    /* Duplicate ACK counter. */
    uint8_t         dack;

    /* Exponential back off value. */
    uint8_t         expboff;
};

/* TCP global data. */
typedef struct _tcp_data
{
    /* Port list. */
    struct _tcp_data_port_list
    {
        TCP_PORT    *head;
        TCP_PORT    *tail;
    } port_list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock to protect global TCP data. */
    SEMAPHORE   lock;
#endif

} TCP_DATA;

/* TCP port search parameter. */
typedef struct _tcp_port_param
{
    /* Resolved TCP port. */
    TCP_PORT        *port;

    /* Socket search data. */
    SOCKET_ADDRESS  socket_address;
} TCP_PORT_PARAM;

/* Function prototypes. */
void tcp_initialize();
void tcp_register(TCP_PORT *, char *, SOCKET_ADDRESS *);
void tcp_unregister(TCP_PORT *);
int32_t net_process_tcp(FS_BUFFER *, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t tcp_header_add(FS_BUFFER *, SOCKET_ADDRESS *, uint32_t, uint32_t, uint16_t, uint16_t, uint32_t, uint8_t);
int32_t tcp_listen(TCP_PORT *);
int32_t tcp_accept(TCP_PORT *, TCP_PORT *);
void tcp_close(TCP_PORT *);

#endif /* NET_TCP */

#endif /* CONFIG_NET */
#endif /* _NET_TCP_H_ */
