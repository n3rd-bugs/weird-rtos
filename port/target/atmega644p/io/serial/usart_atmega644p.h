/*
 * usart_atmega644p.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _USART_ATMEGA664P_H_
#define _USART_ATMEGA664P_H_

#include <stdio.h>

/* Hook-up the STDIO printf function. */
#ifdef printf
#undef printf
#endif
#define printf uart_atmega644p_printf

/* Used by console file system to initialize DEBUG console. */
#define DEBUG_CONSOLE_INIT  usart_atmega644p_init

/* Some configurations. */
#define BAUD_RATE       115200

/* Function prototypes. */
int32_t usart_atmega644p_puts(void *, char *, int32_t);
int32_t uart_atmega644p_printf(char *, ...);
void usart_atmega644p_init();

#endif /* _USART_ATMEGA664P_H_ */
