/*
 * usart_stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _USART_STM32F407_H_
#define _USART_STM32F407_H_

#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <stdarg.h>

/* Some configurations. */
#define BAUD_RATE                       19200
//#define SERIAL_INTERRUPT_MODE

/* Serial buffer configuration. */
#define SERIAL_MAX_BUFFER_SIZE          128
#define SERIAL_NUM_BUFFERS              128
#define SERIAL_NUM_BUFFER_LIST          64
#define SERIAL_THRESHOLD_BUFFER         6
#define SERIAL_THRESHOLD_BUFFER_LIST    2

/* Function prototypes. */
void serial_stm32f407_init();
#ifdef SERIAL_INTERRUPT_MODE
ISR_FUN usart1_interrupt();
#endif /* SERIAL_INTERRUPT_MODE */

#endif /* CONFIG_SERIAL */
#endif /* _USART_STM32F407_H_ */
