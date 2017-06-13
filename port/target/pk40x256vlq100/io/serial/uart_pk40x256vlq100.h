/*
 * uart_pk40x256vlq100.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _UART_PK40X256VLQ100_H_
#define _UART_PK40X256VLQ100_H_

#include <os.h>

/* Hook-up the STDIO printf function. */
#ifdef printf
#undef printf
#endif
#define printf uart_pk40x256vlq100_printf
#ifdef vprintf
#undef vprintf
#endif
#define vprintf uart_pk40x256vlq100_vprintf

/* Used by console file system to initialize DEBUG console. */
#define DEBUG_CONSOLE_INIT  uart_pk40x256vlq100_init

/* Some configurations. */
#define BAUD_RATE           115200
#define PRINTF_BUFFER_SIZE  64

/* Function prototypes. */
uint32_t uart_pk40x256vlq100_puts(void *, uint8_t *, uint32_t);
uint32_t uart_pk40x256vlq100_printf(char *, ...);
int32_t uart_pk40x256vlq100_vprintf(const char *, va_list);
void uart_pk40x256vlq100_init();

#endif /* _UART_PK40X256VLQ100_H_ */
