/*
 * usb_cdc_console.h
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
#ifndef _USB_CDC_CONSOLE_H_
#define _USB_CDC_CONSOLE_H_

#include <os.h>

#define USB_FUN_CDC_AUTO_CONSOLE

#ifdef USB_CDC_CONSOLE
#ifndef FS_CONSOLE
#error "Console FS is needed for using USB CDC device."
#endif

/* Number of bytes to be used to maintain intermediated circular buffer. */
#define USB_CDC_CONSOLE_RX_PIPE_SIZE    512
#define USB_CDC_CONSOLE_TX_PIPE_SIZE    512

/* CDC console device structure. */
typedef struct _cdc_console
{
    /* Console data for file system. */
    CONSOLE     console;

    /* Pipe used for used to maintain data for this console. */
    PIPE        rx_pipe;
    PIPE        tx_pipe;

    /* Intermediate buffer used to maintain OUT buffer to receive incoming
     * data in an endpoint. */
    char        rx_buffer[CDC_DATA_MAX_PACKET_SIZE];
    char        tx_buffer[CDC_DATA_MAX_PACKET_SIZE];
    char        cmd_buffer[CDC_CMD_PACKET_SZE];

    /* PIPE buffers used to maintain data steam. */
    char        rx_pipe_buffer[USB_CDC_CONSOLE_RX_PIPE_SIZE];
    char        tx_pipe_buffer[USB_CDC_CONSOLE_TX_PIPE_SIZE];

    /* Command data. */
    uint32_t    cmd;
    uint32_t    cmd_len;

    /* Alternate set. */
    uint32_t    altset;

} CDC_CONSOLE;

/* Function prototypes. */
void usb_cdc_console_register(CDC_CONSOLE *);
void usb_cdc_console_unregister(CDC_CONSOLE *);

/* Device driver APIs. */
void usb_cdc_fun_console_handle_rx(CDC_CONSOLE *, char *, int32_t);
int32_t usb_cdc_fun_console_handle_tx(CDC_CONSOLE *, char *, int32_t);
void usb_cdc_fun_console_handle_ctrl(CDC_CONSOLE *, uint32_t, char *, int32_t);

#endif /* USB_CDC_CONSOLE */
#endif /* _USB_CDC_CONSOLE_H_ */
