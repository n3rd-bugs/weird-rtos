/*
 * usb.h
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

#ifndef _USB_H_
#define _USB_H_

#include <os.h>

#ifndef CONFIG_FS
#error "File system is needed to manage underlying USB devices."
#endif

#ifdef CONFIG_USB

/* USB configuration. */
#define USB_FUNCTION

/* USB descriptor definitions. */
#define USB_DEVICE_DESC_TYPE            0x01
#define USB_CFG_DESC_TYPE               0x02
#define USB_STR_DESC_TYPE               0x03
#define USB_IFACE_DESC_TYPE             0x04
#define USB_EP_DESC_TYPE                0x05

#define USB_MAX_FIFO                    (15)
#define USB_STRING_DESC_SIZE            (64)
#define USB_DEVICE_DESC_SIZE            (18)
#define USB_LANGID_DESC_SIZE            (4)
#define USB_EP_DESC_SIZE                (9)


/* Type definitions. */
typedef struct usb_setup_req
{
    uint8_t   bmRequest;
    uint8_t   bRequest;
    uint16_t  wValue;
    uint16_t  wIndex;
    uint16_t  wLength;
} USB_SETUP_REQ;

/* USB endpoint definition. */
typedef struct _usb_endpoint
{
    uint8_t     *xfer_buff;
    uint32_t    dma_addr;
    uint32_t    xfer_len;
    uint32_t    xfer_count;
    uint32_t    rem_data_len;
    uint32_t    total_data_len;
    uint32_t    ctl_data_len;
    uint32_t    maxpacket;
    uint8_t     num;
    uint8_t     is_in;
    uint8_t     is_stall;
    uint8_t     type;
    uint8_t     data_pid_start;
    uint8_t     even_odd_frame;
    uint8_t     tx_fifo_num;
    uint8_t     pad[1];
} USB_ENDPOINT;

#ifdef USB_FUNCTION

/* USB function callback. */
typedef struct _usb_fun_cb USB_FUN_CB;

/* USB device structure. */
typedef struct _usb_device
{
    USB_ENDPOINT    in_ep[USB_MAX_FIFO];
    USB_ENDPOINT    out_ep[USB_MAX_FIFO];
    uint32_t        cmd;
    uint32_t        altset;
    uint8_t         str_desc_buffer[USB_STRING_DESC_SIZE];
    uint8_t         setup_packet[8*3];
    USB_FUN_CB      *class_cb;
    uint8_t         *config_desc;
    uint8_t         remote_wakeup;
    uint8_t         config;
    uint8_t         state;
    uint8_t         status;
    uint8_t         address;
    uint8_t         rx_enable;
    uint8_t         pad[2];
} USB_DEVICE;

#endif /* USB_FUNCTION */

/* Function prototypes. */
void usb_init();

/* Include USB target configurations. */
#include <usb_target.h>

#ifdef USB_FUNCTION
#include <usb_function.h>
#endif
#endif /* CONFIG_USB */

#endif /* _USB_H_ */
