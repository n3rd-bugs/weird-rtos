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
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#ifndef _USART_STM32F407_H_
#define _USART_STM32F407_H_

#include <os.h>
#include <stdarg.h>

/* Hook-up the STDIO printf function. */
#ifdef printf
#undef printf
#endif
#define printf  usart_stm32f407_printf
#ifdef vprintf
#undef vprintf
#endif
#define vprintf usart_stm32f407_vprintf

/* Used by console file system to initialize DEBUG console. */
#define DEBUG_CONSOLE_INIT  usart_stm32f407_init

/* Some configurations. */
#define BAUD_RATE           115200
#define PRINTF_BUFFER_SIZE  64

/* Function prototypes. */
int32_t usart_stm32f407_puts(void *, uint8_t *, int32_t);
int32_t usart_stm32f407_printf(char *, ...);
int32_t usart_stm32f407_vprintf(const char *, va_list);
void usart_stm32f407_init();

#endif /* _USART_STM32F407_H_ */
