#include <avr/io.h>
#include <stdio.h>
#define BAUD    115200
#include <util/setbaud.h>

int __serial_putc(char c, FILE *fd);

FILE serial_fd = FDEV_SETUP_STREAM(__serial_putc, NULL, _FDEV_SETUP_WRITE);

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
