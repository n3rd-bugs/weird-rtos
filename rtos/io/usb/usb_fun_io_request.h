/*
 * usb_fun_io_request.h
 *
 * Copyright(c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 *(in any form) the author will not be liable for any legal charges.
 */
#ifndef _USB_FUN_IO_REQUEST_H_
#define _USB_FUN_IO_REQUEST_H_

#include <os.h>

#ifdef USB_FUNCTION

/* Function prototypes. */
uint32_t usb_fun_control_tx(USB_STM32F407_HANDLE *, uint8_t *, uint32_t );
uint32_t usb_fun_control_continue_tx(USB_STM32F407_HANDLE *, uint8_t *, uint32_t );
uint32_t usb_fun_control_rx(USB_STM32F407_HANDLE *, uint8_t *, uint32_t );
uint32_t usb_fun_control_continue_rx(USB_STM32F407_HANDLE *, uint8_t *, uint32_t );
uint32_t usb_fun_control_send_status(USB_STM32F407_HANDLE *);
uint32_t usb_fun_control_receive_status(USB_STM32F407_HANDLE *);
uint32_t usb_fun_get_rx_count(USB_STM32F407_HANDLE *, uint8_t);

#endif /* USB_FUNCTION */
#endif /* _USB_FUN_IO_REQUEST_H_ */
