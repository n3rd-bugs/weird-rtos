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
#include <serial.h>
#include <stdarg.h>

#ifdef CMAKE_BUILD
#include <usart_avr_config.h>
#else
/* Some configurations. */
#define SERIAL_BAUD_RATE                (115200)
//#define SERIAL_INTERRUPT_MODE
//#define SERIAL_ENABLE_HW_FLOW

/* Serial buffer configuration. */
#define SERIAL_MAX_BUFFER_SIZE          (16)
#define SERIAL_NUM_BUFFERS              (4)
#define SERIAL_NUM_BUFFER_LIST          (4)
#define SERIAL_THRESHOLD_BUFFER         (0)
#define SERIAL_THRESHOLD_BUFFER_LIST    (0)

/* USART HW configurations. */
#define USART_HW_TOGGLE_DELAY           (0)
#define USART_HW_RTS_RECOVER_DELAY      (20000)

/* RTS/CTS signal pins for USART0. */
#define USART0_HW_RTS                   (5)
#define USART0_HW_RTS_PORT              (PORTD)
#define USART0_HW_RTS_DDR               (DDRD)
#define USART0_HW_RTS_PIN               (PIND)
#define USART0_HW_RTS_RESET             (6)
#define USART0_HW_RTS_RESET_PORT        (PORTD)
#define USART0_HW_RTS_RESET_DDR         (DDRD)
#define USART0_HW_RTS_RESET_PIN         (PIND)
#define USART0_HW_CTS                   (7)
#define USART0_HW_CTS_PORT              (PORTD)
#define USART0_HW_CTS_DDR               (DDRD)
#define USART0_HW_CTS_PIN               (PIND)

/* RTS/CTS signal pins for USART1. */
#define USART1_HW_RTS                   (5)
#define USART1_HW_RTS_PORT              (PORTD)
#define USART1_HW_RTS_DDR               (DDRD)
#define USART1_HW_RTS_PIN               (PIND)
#define USART1_HW_RTS_RESET             (6)
#define USART1_HW_RTS_RESET_PORT        (PORTD)
#define USART1_HW_RTS_RESET_DDR         (DDRD)
#define USART1_HW_RTS_RESET_PIN         (PIND)
#define USART1_HW_CTS                   (7)
#define USART1_HW_CTS_PORT              (PORTD)
#define USART1_HW_CTS_DDR               (DDRD)
#define USART1_HW_CTS_PIN               (PIND)
#endif /* CMAKE_BUILD */

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

#endif /* CONFIG_SERIAL */
#endif /* _USART_AVR_H_ */
