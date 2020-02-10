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
#ifdef IO_SERIAL
#include <stdarg.h>
#include <usart_stm32_config.h>

/* Function prototypes. */
void serial_stm32f407_init(void);
#ifdef SERIAL_INTERRUPT_MODE
ISR_FUN usart1_interrupt(void);
#endif /* SERIAL_INTERRUPT_MODE */

#endif /* IO_SERIAL */
#endif /* _USART_STM32F407_H_ */
