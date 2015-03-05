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
#ifndef FS_CONSOLE
#error "Console FS is needed for using USB CDC device."
#endif

/* CDC console device structure. */
typedef struct _cdc_console
{
    /* Console data for file system. */
    CONSOLE     console;

    /* Intermediate buffer used to maintain OUT buffer to receive incoming
     * data in an endpoint. */
    char        rx_buffer[CDC_DATA_MAX_PACKET_SIZE];
    char        tx_buffer[CDC_DATA_MAX_PACKET_SIZE];
    char        cmd_buffer[CDC_CMD_PACKET_SZE];

    /* USB device. */
    void        *usb_device;

    /* Buffer data lengths. */
    uint32_t    rx_len;
    uint32_t    tx_len;
    uint32_t    cmd_len;

} CDC_CONSOLE;

/* Function prototypes. */
void usb_cdc_console_register(CDC_CONSOLE *);
void usb_cdc_console_unregister(CDC_CONSOLE *);

/* Device driver APIs. */
void usb_cdc_fun_console_handle_rx(CDC_CONSOLE *, uint32_t);
void usb_cdc_fun_console_handle_tx_complete(CDC_CONSOLE *);
uint32_t usb_cdc_fun_console_handle_tx(CDC_CONSOLE *);
void usb_cdc_fun_console_handle_ctrl(CDC_CONSOLE *, uint32_t, char *, int32_t);

#endif /* USB_CDC_CONSOLE */
#endif /* _USB_CDC_CONSOLE_H_ */
