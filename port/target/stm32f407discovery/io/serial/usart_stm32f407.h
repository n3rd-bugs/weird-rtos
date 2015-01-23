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
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _USART_STM32F407_H_
#define _USART_STM32F407_H_

#include <os.h>

/* Hook-up the STDIO printf function. */
#ifdef printf
#undef printf
#endif
#define printf usart_stm32f407_printf

/* Used by console file system to initialize DEBUG console. */
#define DEBUG_CONSOLE_INIT  usart_stm32f407_init

/* Some configurations. */
#define BAUD_RATE       115200

/* Function prototypes. */
uint32_t usart_stm32f407_puts(void *priv_data, char *buf, uint32_t nbytes);
uint32_t usart_stm32f407_printf(char *format, ...);
void usart_stm32f407_init();

#endif /* _USART_STM32F407_H_ */
