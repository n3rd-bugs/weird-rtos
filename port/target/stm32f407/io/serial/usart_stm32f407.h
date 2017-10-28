/*
 * usart_stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _USART_STM32F407_H_
#define _USART_STM32F407_H_

#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <stdarg.h>

#ifndef CMAKE_BUILD
/* Some configurations. */
#define BAUD_RATE                       19200
//#define SERIAL_INTERRUPT_MODE

/* Serial buffer configuration. */
#define SERIAL_MAX_BUFFER_SIZE          128
#define SERIAL_NUM_BUFFERS              128
#define SERIAL_NUM_BUFFER_LIST          64
#define SERIAL_THRESHOLD_BUFFER         6
#define SERIAL_THRESHOLD_BUFFER_LIST    2
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void serial_stm32f407_init(void);
#ifdef SERIAL_INTERRUPT_MODE
ISR_FUN usart1_interrupt(void);
#endif /* SERIAL_INTERRUPT_MODE */

#endif /* CONFIG_SERIAL */
#endif /* _USART_STM32F407_H_ */
