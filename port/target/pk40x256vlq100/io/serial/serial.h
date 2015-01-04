/*
 * serial.h
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
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <os.h>

typedef struct _UART_CON
{
    /* For now it only has console file data. */
    CONSOLE     console;

#ifdef CONFIG_SEMAPHORE
    /* UART lock. */
    SEMAPHORE   lock;
#endif

} UART_CON;

/* Some configurations. */
#define BAUD_RATE       115200

/* Function prototypes. */
uint32_t serial_puts(void *priv_data, char *buf, uint32_t nbytes);
uint32_t serial_printf(char *format, ...);
void serial_init();

#ifdef printf
#undef printf
#endif
#define printf serial_printf

#endif /* _SERIAL_H_ */
