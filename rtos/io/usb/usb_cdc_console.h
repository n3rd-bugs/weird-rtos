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

#ifdef USB_CDC_CONSOLE
#include <fs.h>

#ifndef FS_CONSOLE
#error "Console FS is needed for using USB CDC device."
#endif

#include <console.h>

/* CDC console device structure. */
typedef struct _cdc_console
{
    /* Console data for file system. */
    CONSOLE     console;

    /* USB device. */
    void        *usb_device;

    /* Buffer used to manage data for this device. */
    char        buffer[CDC_DATA_MAX_PACKET_SIZE * CDC_NUM_BUFFERS];

    /* File system buffers. */
    FS_BUFFER_DATA  fs_buffer_data;
    FS_BUFFER_ONE   fs_buffer[CDC_NUM_BUFFERS];
    FS_BUFFER       fs_buffer_list[CDC_NUM_BUFFER_LISTS];

    /* These should only be accessed/modified in ISR context. */

    /* Current buffers being used. */
    FS_BUFFER_ONE   *rx_buffer;
    FS_BUFFER_ONE   *tx_buffer;

    /* Number of bytes valid in the receive buffer. */
    uint32_t    rx_valid;

    /* Command buffer. */
    char        cmd_buffer[CDC_CMD_PACKET_SIZE];

    /* Buffer data lengths. */
    uint32_t    cmd_len;

} CDC_CONSOLE;

/* Function prototypes. */
void usb_cdc_console_register(CDC_CONSOLE *, void *);
void usb_cdc_console_unregister(CDC_CONSOLE *);
void usb_cdc_console_handle_connect(CDC_CONSOLE *);
void usb_cdc_console_handle_disconnect(CDC_CONSOLE *);

/* Device driver APIs. */
void usb_cdc_fun_console_handle_sof(CDC_CONSOLE *);
void usb_cdc_fun_console_handle_rx(CDC_CONSOLE *, uint32_t);
void usb_cdc_fun_console_handle_tx_complete(CDC_CONSOLE *);
void usb_cdc_fun_console_handle_rx_start(CDC_CONSOLE *);
FS_BUFFER_ONE *usb_cdc_fun_console_handle_tx(CDC_CONSOLE *);
void usb_cdc_fun_console_handle_ctrl(CDC_CONSOLE *, uint32_t, char *, int32_t);

#endif /* USB_CDC_CONSOLE */
#endif /* _USB_CDC_CONSOLE_H_ */
