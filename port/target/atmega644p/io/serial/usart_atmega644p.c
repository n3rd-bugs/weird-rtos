/*
 * usart_atmega644p.c
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
#include <os.h>
#include <avr/io.h>
#include <stdio.h>
#define BAUD        BAUD_RATE
#define BAUD_TOL    10
#include <util/setbaud.h>
#include <fs.h>
#include <console.h>

#ifdef FS_CONSOLE
/* Debug file descriptor. */
static FD debug_fd = NULL;

/* Console data. */
static CONSOLE usart_1 =
{
    .fs =
    {
        /* Name of this port. */
        .name = "uart1",

        /* Console manipulation APIs. */
        .write = &usart_atmega644p_puts,
    }
};
#endif

/*
 * usart_atmega644p_puts
 * @priv_data: For now it is unused.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * This function prints a string on the USART.
 */
int32_t usart_atmega644p_puts(void *priv_data, uint8_t *buf, int32_t nbytes)
{
    int32_t to_print = nbytes;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);

    /* While we have some data to be printed. */
    while(nbytes > 0)
    {
        /* Wait for last byte to be sent. */
        loop_until_bit_is_set(UCSR0A, UDRE0);

        /* Add this byte. */
        UDR0 = (*buf);

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* usart_atmega644p_puts */

/*
 * uart_atmega644p_printf
 * @format: Formated string to be printed on USART.
 * This function prints a formated string on the USART.
 */
int32_t uart_atmega644p_printf(char *format, ...)
{
    int32_t n = 0;
    uint8_t buf[100];
    va_list vl;

    /* Arguments start from the format. */
    va_start(vl, format);

    /* Process the given string and save the result in a temporary buffer. */
    n = vsnprintf((char *)buf, 100, format, vl);

#ifdef FS_CONSOLE
    /* Assert if debug FD is not yet initialized. */
    OS_ASSERT(debug_fd == NULL);

    /* Use the debug FD. */
    n = fs_write(debug_fd, buf, n);
#else
    /* Print the result on the UART. */
    n = usart_atmega644p_puts(NULL, buf, n);
#endif

    /* Destroy the argument list. */
    va_end(vl);

    /* Return number of bytes printed on UART. */
    return (n);

} /* uart_atmega644p_printf */

/*
 * usart_atmega644p_init
 * This function initializes the AVR serial console over USART.
 */
void usart_atmega644p_init()
{
    /* Set the configured baud-rate. */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    /* Enable RX and TX. */
    UCSR0B = (1 << RXEN0)  | (1 << TXEN0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);

#ifdef FS_CONSOLE

    /* Register serial port with console. */
    console_register(&usart_1);

    /* There is always some space available to send data. */
    usart_1.fs.flags |= FS_SPACE_AVAILABLE;

    /* Set debug file descriptor. */
    debug_fd = fs_open("\\console\\uart1", 0);

#endif /* FS_CONSOLE */

} /* usart_atmega644p_init */
