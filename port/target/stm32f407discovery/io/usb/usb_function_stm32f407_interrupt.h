/*
 * usb_function_stm32f407_interrupt.h
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
#ifndef _USB_FUNCTION_STM32F407_INTERRUPT_H_
#define _USB_FUNCTION_STM32F407_INTERRUPT_H_

#include <os.h>

#ifdef USB_FUNCTION

/* Helper macro definitions. */
#define CLEAR_IN_EP_INTR(num, interrupt)    {                                                                       \
                                                diepint.d32 = 0;                                                    \
                                                diepint.b.interrupt = 1;                                            \
                                                OS_WRITE_REG32(&pdev->regs.INEP_REGS[num]->DIEPINT, diepint.d32);   \
                                            }

#define CLEAR_OUT_EP_INTR(num, interrupt)   {                                                                       \
                                                doepint.d32 = 0;                                                    \
                                                doepint.b.interrupt = 1;                                            \
                                                OS_WRITE_REG32(&pdev->regs.OUTEP_REGS[num]->DOEPINT, doepint.d32);  \
                                            }

/* USB function interrupt callbacks. */
typedef struct _usb_fun_interrupt_cb
{
    uint32_t (*data_out_stage)(USB_STM32F407_HANDLE *, uint8_t);
    uint32_t (*data_in_stage)  (USB_STM32F407_HANDLE * , uint8_t);
    uint32_t (*setup_stage) (USB_STM32F407_HANDLE *);
    uint32_t (*sof) (USB_STM32F407_HANDLE *);
    uint32_t (*reset) (USB_STM32F407_HANDLE *);
    uint32_t (*suspend) (USB_STM32F407_HANDLE *);
    uint32_t (*resume) (USB_STM32F407_HANDLE *);
    uint32_t (*iso_in_incomplete) (USB_STM32F407_HANDLE *);
    uint32_t (*iso_out_incomplete) (USB_STM32F407_HANDLE *);

    uint32_t (*connected) (USB_STM32F407_HANDLE *);
    uint32_t (*disconnected) (USB_STM32F407_HANDLE *);

} USB_FUN_INTERRUPT_CB;

extern USB_FUN_INTERRUPT_CB *usb_function_interrupt_cb;

/* Function prototypes. */
uint32_t usb_function_stm32f407_interrupt_handler(USB_STM32F407_HANDLE *);

#endif /* USB_FUNCTION */

#endif /* _USB_FUNCTION_STM32F407_INTERRUPT_H_ */
