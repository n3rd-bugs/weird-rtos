/*
 * usb_fun_request.h
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
#ifndef _USB_FUN_REQUEST_H_
#define _USB_FUN_REQUEST_H_

#include <os.h>
#ifdef USB_FUNCTION

/* USB function request callback. */
typedef struct _usb_fun_req_cb
{
    uint32_t (*set_cfg)(USB_STM32F407_HANDLE *, uint8_t);
    uint32_t (*clear_cfg)(USB_STM32F407_HANDLE *, uint8_t);
} USB_FUN_REQ_CB;

/* Function prototypes. */
uint32_t usb_fun_std_interface_request(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
uint32_t usb_fun_std_dev_request(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
uint32_t usb_fun_std_endpoint_request(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
void usb_fun_parse_setup_request(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
void usb_fun_control_error(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
void usb_fun_get_string(uint8_t *, uint8_t *, uint16_t *);

#endif /* USB_FUNCTION */
#endif /* _USB_FUN_REQUEST_H_ */
