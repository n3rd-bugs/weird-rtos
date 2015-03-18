/*
 * usb_fun_cdc_acm.h
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

#ifndef _USB_FUN_CDC_ACM_H_
#define _USB_FUN_CDC_ACM_H_

#include <os.h>

#ifdef USB_FUNCTION_CDC_ACM

/* Enable CDC console. */
#define USB_CDC_CONSOLE

#define CDC_IN_EP                       0x81    /* EP1 for data IN. */
#define CDC_OUT_EP                      0x01    /* EP1 for data OUT. */
#define CDC_CMD_EP                      0x82    /* EP2 for CDC commands. */
#define CDC_DATA_MAX_PACKET_SIZE        64      /* Endpoint Packet size. */
#define CDC_NUM_BUFFERS                 16      /* Number of buffers we will use to transmit and receive data on this USB CDC device. */
#define CDC_CMD_PACKET_SIZE             8       /* Control Endpoint Packet size. */

/* CDC ACM descriptor configuration. */
#define USB_FUN_CDC_ACM_VID                 0x0483
#define USB_FUN_CDC_ACM_PID                 0x5740

#define USB_FUN_CDC_ACM_LANGID_STR          0x409   /* English (United States) */
#define USB_FUN_CDC_ACM_MFGR_STR            "N3RD Bugs"

#define USB_FUN_CDC_ACM_PDT_HS_STR          "Tiny-RTOS CDC Console (HS mode)"
#define USB_FUN_CDC_ACM_SN_HS_STR           "2K15-USB-CDC-HS-00000000"
#define USB_FUN_CDC_ACM_CFG_HS_STR          "None"
#define USB_FUN_CDC_ACM_IFACE_HS_STR        "System CDC Console"

#define USB_FUN_CDC_ACM_PDT_FS_STR          "Tiny-RTOS CDC Console (FS mode)"
#define USB_FUN_CDC_ACM_SN_FS_STR           "2K15-USB-CDC-FS-00000000"
#define USB_FUN_CDC_ACM_CFG_FS_STR          "None"
#define USB_FUN_CDC_ACM_IFACE_FS_STR        "System CDC Console"

/* CDC ACM core definitions. */
#define USB_CDC_CONFIG_DESC_SIZ         (67)
#define USB_CDC_DESC_SIZ                (67-9)

#define CDC_DESCRIPTOR_TYPE             0x21

#define DEVICE_CLASS_CDC                0x02
#define DEVICE_SUBCLASS_CDC             0x00

/* CDC ACM requests. */
#define SEND_ENCAPSULATED_COMMAND       0x00
#define GET_ENCAPSULATED_RESPONSE       0x01
#define SET_COMM_FEATURE                0x02
#define GET_COMM_FEATURE                0x03
#define CLEAR_COMM_FEATURE              0x04
#define SET_LINE_CODING                 0x20
#define GET_LINE_CODING                 0x21
#define SET_CONTROL_LINE_STATE          0x22
#define SEND_BREAK                      0x23
#define NO_CMD                          0xFF

/* USB CDC ACM device. */
extern USB_FUN_CB usb_fun_cdc_acm_cb;

#ifdef USB_CDC_CONSOLE
#include <usb_cdc_console.h>
#endif
#include <os_target.h>

/* USB CDC ACM function device descriptor. */
typedef struct _usb_fun_cdc_acm_dev
{
    USB_STM32F407_HANDLE    usb;
    CDC_CONSOLE             cdc_console;
} USB_FUN_CDC_ACM_DEV;

/* Function prototypes. */
void usb_fun_cdc_acm_data_out_enable(void *);

#endif /* USB_FUNCTION_CDC_ACM */
#endif /* _USB_FUN_CDC_ACM_H_ */
