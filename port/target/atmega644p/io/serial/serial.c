/*
 * serial.c
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
#include <avr/io.h>
#include <stdio.h>
#define BAUD    115200
#include <util/setbaud.h>

/*
 * __serial_putc
 * @c: The character that is needed to be sent on the port.
 * @fd: This file descriptor.
 * @return: This is always 0.
 * This function sends a single character on the serial port.
 */
int __serial_putc(char c, FILE *fd)
{
    /* Send "\r\n" for a new line. */
    if (c == '\n')
    {
        /* First send "\r". */
        __serial_putc('\r', fd);
    }

    /* Wait for last byte to be sent. */
    loop_until_bit_is_set(UCSR0A, UDRE0);

    /* Add this byte. */
    UDR0 = c;

    /* Return success. */
    return (0);

} /* __serial_putc */

/*
 * serial_init
 * This function initializes the AVR serial port so that user can send important
 * information on the serial port.
 */
void serial_init()
{
    /* Set the configured boud-rate. */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSRA |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    /* Enable RX and TX. */
    UCSR0B = (1 << RXEN0)  | (1 << TXEN0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);

} /* serial_init */

/* This is the file descriptor that will be used to stream out the data on the
 * serial port.  */
FILE serial_fd = FDEV_SETUP_STREAM(__serial_putc, NULL, _FDEV_SETUP_WRITE);
