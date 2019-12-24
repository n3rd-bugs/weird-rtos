/*
 * usart_stm32f030.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _USART_STM32F030_H_
#define _USART_STM32F030_H_

#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <serial.h>
#include <stdarg.h>

#ifndef CMAKE_BUILD
/* Some configurations. */
#define SERIAL_BAUD_RATE                115200
//#define SERIAL_INTERRUPT_MODE

/* Serial buffer configuration. */
#define SERIAL_MAX_BUFFER_SIZE          16
#define SERIAL_NUM_BUFFERS              4
#define SERIAL_NUM_BUFFER_LIST          4
#define SERIAL_THRESHOLD_BUFFER         0
#define SERIAL_THRESHOLD_BUFFER_LIST    0
#endif /* CMAKE_BUILD */

/* USART flags. */
#define STM32_USART_HW_FCTRL            0x01

/* STM32 USRAT device. */
typedef struct _stm32_usart
{
    /* Serial device. */
    SERIAL          serial;

    /* Hardware register. */
    USART_TypeDef   *reg;

    /* Device number. */
    uint32_t         device_num;

    /* Baud rate. */
    uint32_t         baud_rate;

    /* USART flags */
    uint32_t         flags;

} STM32_USART;

/* Function prototypes. */
void serial_stm32f030_init(void);
int32_t usart_stm32f030_register(STM32_USART *, const char *, uint8_t, uint32_t, FS_BUFFER_DATA *, uint8_t, uint8_t);
ISR_FUN usart1_interrupt(void);
ISR_FUN usart2_interrupt(void);

#endif /* CONFIG_SERIAL */
#endif /* _USART_STM32F030_H_ */