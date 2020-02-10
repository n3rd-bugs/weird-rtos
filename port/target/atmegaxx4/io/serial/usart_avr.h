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
#ifdef IO_SERIAL
#include <serial.h>
#include <stdarg.h>
#include <usart_avr_config.h>

/* Macro to detect if USART1 is available on the platform. */
#define USART1_AVAILABLE                (defined(UDR1))

/* AVR USART data. */
typedef struct _avr_usart
{
    /* Serial data for this USRAT. */
    SERIAL      serial;

    /* Hardware tick at which line was asserted. */
    uint64_t    last_asserted;

    /* Baud rate for this serial device. */
    uint32_t    baudrate;

    /* AVR USART number. */
    uint8_t     num;

    /* Flag to specify if HW flow control is enabled. */
    uint8_t     hw_flow_enabled;

    /* Structure padding. */
    uint8_t     pad[2];

} AVR_USART;

/* Function prototypes. */
void serial_avr_init(void);
void usart_avr_register(AVR_USART *, const char *, uint8_t, uint32_t, FS_BUFFER_DATA *, uint8_t, uint8_t);

#endif /* IO_SERIAL */
#endif /* _USART_AVR_H_ */
