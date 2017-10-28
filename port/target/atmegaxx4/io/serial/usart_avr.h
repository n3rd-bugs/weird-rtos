/*
 * usart_avr.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _USART_AVR_H_
#define _USART_AVR_H_

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
void serial_avr_init(void);

#endif /* CONFIG_SERIAL */
#endif /* _USART_AVR_H_ */
