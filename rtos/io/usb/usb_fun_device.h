/*
 * usb_fun_device.h
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
#ifndef _USB_FUN_DEVICE_H_
#define _USB_FUN_DEVICE_H_

#include <os.h>

#ifdef USB_FUNCTION

/* USB function device status. */
#define USB_FUN_STATE_DEFAULT       1
#define USB_FUN_STATE_ADDRESSED     2
#define USB_FUN_STATE_CONFIGURED    3
#define USB_FUN_STATE_SUSPENDED     4

/* Function prototypes. */
void usb_fun_device_init(USB_STM32F407_HANDLE *);
uint32_t usb_fun_endpoint_open(USB_STM32F407_HANDLE *, uint8_t, uint16_t, uint8_t);
uint32_t usb_fun_endpoint_close(USB_STM32F407_HANDLE *, uint8_t);
uint32_t usb_fun_endpoint_prepare_rx(USB_STM32F407_HANDLE *, uint8_t, uint8_t *, uint32_t);
uint32_t usb_fun_endpoint_tx(USB_STM32F407_HANDLE *, uint8_t, uint8_t *, uint32_t);
uint32_t usb_fun_endpoint_stall(USB_STM32F407_HANDLE *, uint8_t);
uint32_t usb_fun_endpoint_clear_stall(USB_STM32F407_HANDLE *, uint8_t);
uint32_t usb_fun_endpoint_flush(USB_STM32F407_HANDLE *, uint8_t);
void usb_fun_endpoint_set_address(USB_STM32F407_HANDLE *, uint8_t);
uint32_t usb_fun_endpoint_get_status(USB_STM32F407_HANDLE *, uint8_t);
void usb_fun_endpoint_set_status(USB_STM32F407_HANDLE *, uint8_t, uint32_t);

#endif /* USB_FUNCTION */
#endif /* _USB_FUN_DEVICE_H_ */
