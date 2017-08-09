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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _USART_ATMEGA664P_H_
#define _USART_ATMEGA664P_H_

#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <stdarg.h>

#ifndef CMAKE_BUILD
/* Some configurations. */
#define BAUD_RATE                       115200
//#define SERIAL_INTERRUPT_MODE

/* Serial buffer configuration. */
#define SERIAL_MAX_BUFFER_SIZE          16
#define SERIAL_NUM_BUFFERS              4
#define SERIAL_NUM_BUFFER_LIST          4
#define SERIAL_THRESHOLD_BUFFER         0
#define SERIAL_THRESHOLD_BUFFER_LIST    0
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void serial_atmega644p_init(void);

#endif /* CONFIG_SERIAL */
#endif /* _USART_ATMEGA664P_H_ */
