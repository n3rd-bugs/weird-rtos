/*
 * usb_target.h
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

#ifndef _USB_TARGET_H_
#define _USB_TARGET_H_

#include <os.h>

#ifdef CONFIG_USB
#include <usb_stm32f407.h>

/* Hook-up USB OS stack. */
#define USB_TGT_INIT    usb_stm32f407_init

/* Week link the STM32F407 USB handle. */
typedef struct _usb_stm32f407_handle USB_STM32F407_HANDLE;

#endif /* CONFIG_USB */

#endif /* _USB_TARGET_H_ */
