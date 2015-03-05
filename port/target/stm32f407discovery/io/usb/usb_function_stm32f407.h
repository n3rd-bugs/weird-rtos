/*
 * usb_function_stm32f407.h
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
#ifndef _USB_FUNCTION_STM32F407_H_
#define _USB_FUNCTION_STM32F407_H_

#include <os.h>

#ifdef USB_FUNCTION
#include <usb_function_stm32f407_interrupt.h>

/* ST32F407 USB function configuration. */
#define STM32F407_USB_CDC_ACM
//#define STM32F407_USB_CDC_DEBUG
#ifdef CONFIG_PPP
#define STM32F407_USB_CDC_ACM_PPP
#endif

/* Function prototypes. */
void usb_function_stm32f407_init();
uint32_t usb_function_stm32f407_core_initialize_device(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_initialize_speed(USB_STM32F407_HANDLE *, uint8_t);
enum USB_STM32F407_SPEED usb_function_stm32f407_get_device_speed(USB_STM32F407_HANDLE *);
uint32_t usb_function_stm32f407_ep0_deactivate(USB_STM32F407_HANDLE *);
uint32_t usb_function_stm32f407_ep_activate(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
uint32_t usb_function_stm32f407_ep_deactivate(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
uint32_t usb_function_stm32f407_ep_start_transfer(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
uint32_t usb_function_stm32f407_ep0_start_transfer(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
uint32_t usb_function_stm32f407_ep_set_stall(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
uint32_t usb_function_stm32f407_ep_clear_stall(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
uint32_t usb_function_stm32f407_ep_get_device_all_out_interrupt(USB_STM32F407_HANDLE *);
uint32_t usb_function_stm32f407_ep_get_device_out_interrupt(USB_STM32F407_HANDLE *, uint8_t);
uint32_t usb_function_stm32f407_ep_get_device_all_in_interrupt(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_ep_set_status (USB_STM32F407_HANDLE *, USB_ENDPOINT *, uint32_t);
uint32_t usb_function_stm32f407_ep_get_status(USB_STM32F407_HANDLE *, USB_ENDPOINT *);
void usb_function_stm32f407_ep0_start_out(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_activate_remote_wakeup(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_ungate_clock(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_stop_device(USB_STM32F407_HANDLE *);
uint32_t usb_function_stm32f407_enable_interrupts(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_connect(USB_STM32F407_HANDLE *);
void usb_function_stm32f407_disconnect(USB_STM32F407_HANDLE *);

#endif /* USB_FUNCTION */
#endif /* _USB_FUNCTION_STM32F407_H_ */
