/*
 * net_tcp.h
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
#ifndef _NET_TCP_H_
#define _NET_TCP_H_

#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_TCP
#include <console.h>

/* TCP configuration. */
#ifndef CMAKE_BUILD
#define TCP_WND_SIZE                (1024)
#define TCP_WND_SCALE               (2)
#define TCP_RTO                     (750)
#define TCP_MAX_RTO                 (5000)
#define TCP_MSL                     (60000)
#define TCP_NUM_RTX                 (16)
#define TCP_MAX_CONG_WINDOW         (0xFFFF)
#endif /* CMAKE_BUILD */

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
#define TCP_SOCK_CLOSE_WAIT         (5)    /* Not used. */
#define TCP_SOCK_FIN_WAIT_1         (6)
#define TCP_SOCK_FIN_WAIT_2         (7)
#define TCP_SOCK_LAST_ACK           (8)
#define TCP_SOCK_TIME_WAIT          (9)
#define TCP_SOCK_CLOSING            (10)

/* TCP socket flags. */
#define TCP_FLAG_WND_SCALE          (0x01)
#define TCP_FLAG_MSS                (0x02)

/* TCP out-of-order parameter flags. */
#define TCP_FLAG_SEG_CONFLICT       (0x01)

/* TCP RTX data flags. */
#define TCP_RTX_IN_USE              (0x01)
#define TCP_RTX_BUFFER_RETURNED     (0x04)

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

/* TCP retransmission packet structure. */
typedef struct _tcp_timeout_suspend
{
    /* Networking condition data for this TCP socket. */
    CONDITION       condition;
    SUSPEND         suspend;

} TCP_TIMEOUT_SUSPEND;

/* TCP retransmission data. */
typedef struct _tcp_port TCP_PORT;
typedef struct _tcp_rtx_data TCP_RTX_DATA;
struct _tcp_rtx_data
{
    /* Associated port with this retransmission structure. */
    TCP_PORT        *port;

    /* Buffer need to be retransmitted. */
    FS_BUFFER_LIST  *buffer;

    /* List member. */
    TCP_RTX_DATA    *next;

    /* Sequence number associated with this packet. */
    uint32_t        seq_num;

    /* Segment length. */
    uint16_t        seg_len;

    /* Structure flags to maintain state. */
    uint8_t         flags;

    /* Structure padding. */
    uint8_t         pad[5];

};

/* TCP port structure. */
struct _tcp_port
{
    /* Console structure for this TCP port. */
    CONSOLE             console;

    /* TCP buffer lists. */
    struct _tcp_port_buffer_list
    {
        FS_BUFFER_LIST      *head;
        FS_BUFFER_LIST      *tail;
    } buffer_list;

    /* Structure to maintain TCP RX buffer data. */
    struct _tcp_port_rx_buffer
    {
        /* TCP out of order buffer lists. */
        struct _tcp_port_oorx_list
        {
            FS_BUFFER_LIST      *head;
            FS_BUFFER_LIST      *tail;
        } oorx_list;

        /* TCP accumulated received buffer. */
        FS_BUFFER_LIST      *buffer;
    } rx_buffer;

    /* TCP socket address. */
    SOCKET_ADDRESS      socket_address;

    /* These variables maintain the socket state and configurations. */

    /* TCP timeout data. */
    TCP_TIMEOUT_SUSPEND timeout_suspend;

    /* Retransmission lists. */
    TCP_RTX_DATA        rtx[TCP_NUM_RTX];

    /* Tick at which a TCP event is needed to be processed. */
    uint32_t            event_timeout;

    /* Tick at which a TCP retransmission is needed to be performed. */
    uint32_t            rtx_timeout;

    /* Current retransmission time. */
    uint32_t            rtx_time;

    /* TCP port list member. */
    TCP_PORT            *next;

    /* Receive sequence numbers. */
    uint32_t            rcv_nxt;
    uint32_t            rcv_wnd;

    /* Send sequence numbers. */
    uint32_t            snd_nxt;
    uint32_t            snd_una;
    uint32_t            snd_wnd;

    /* Maximum segment size. */
    uint16_t            mss;

    /* TCP window configuration. */
    uint8_t             rcv_wnd_scale;
    uint8_t             snd_wnd_scale;

    /* TCP socket state. */
    uint8_t             state;

    /* TCP socket flags. */
    uint8_t             flags;

    /* ACK counter. */
    uint8_t             nacks;

    /* Flags to specify if the event or/and the retransmission timers are enabled. */
    uint8_t             event_timeout_enable;
    uint8_t             rtx_timeout_enable;

    /* Structure padding. */
    uint8_t             pad[3];
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
    SEMAPHORE       lock;
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
void tcp_initialize(void);
void tcp_register(TCP_PORT *, char *, SOCKET_ADDRESS *);
void tcp_unregister(TCP_PORT *);
int32_t net_process_tcp(FS_BUFFER_LIST *, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t tcp_header_add(FS_BUFFER_LIST *, SOCKET_ADDRESS *, uint32_t, uint32_t, uint16_t, uint16_t, uint32_t, uint8_t);
int32_t tcp_listen(TCP_PORT *);
int32_t tcp_connect(TCP_PORT *);
int32_t tcp_accept(TCP_PORT *, TCP_PORT *);
void tcp_close(TCP_PORT *);

#endif /* NET_TCP */

#endif /* CONFIG_NET */
#endif /* _NET_TCP_H_ */
