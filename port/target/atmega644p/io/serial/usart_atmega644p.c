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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <stdio.h>
#include <serial.h>
#include <avr/io.h>
#define BAUD        BAUD_RATE
#define BAUD_TOL    10
#include <util/setbaud.h>
#include <avr/interrupt.h>

#ifdef SERIAL_INTERRUPT_MODE
#ifndef FS_CONSOLE
#error "Console is required for interrupt mode serial."
#endif /* FS_CONSOLE */
#endif /* SERIAL_INTERRUPT_MODE */

/* Internal function prototypes. */
static int32_t usart_atmega644p_init(void *);
#ifdef SERIAL_INTERRUPT_MODE
void usart0_handle_tx_interrupt();
void usart0_handle_rx_interrupt();
static void usart_atmega644p_enable_interrupt(void *);
#endif /* SERIAL_INTERRUPT_MODE */
static void usart_atmega644p_disable_interrupt(void *);
static int32_t usart_atmega644p_puts(void *, void *, uint8_t *, int32_t, uint32_t);
static int32_t usart_atmega644p_gets(void *, void *, uint8_t *, int32_t, uint32_t);

/* Target serial port. */
static SERIAL usart0;
#ifdef SERIAL_INTERRUPT_MODE
static FS_BUFFER_DATA usart0_buffer_data;
static uint8_t usart0_buffer_space[SERIAL_MAX_BUFFER_SIZE * SERIAL_NUM_BUFFERS];
static FS_BUFFER_ONE usart0_buffer_ones[SERIAL_NUM_BUFFERS];
static FS_BUFFER usart0_buffer_lists[SERIAL_NUM_BUFFER_LIST];
#endif /* SERIAL_INTERRUPT_MODE */

/*
 * serial_atmega644p_init
 * This will initialize serial interface(s) for this target.
 */
void serial_atmega644p_init()
{
    /* Initialize serial device data. */
    usart0.device.init = &usart_atmega644p_init;
    usart0.device.puts = &usart_atmega644p_puts;
    usart0.device.gets = &usart_atmega644p_gets;
#ifdef SERIAL_INTERRUPT_MODE
    usart0.device.int_lock = &usart_atmega644p_disable_interrupt;
    usart0.device.int_unlock = &usart_atmega644p_enable_interrupt;
#endif /* SERIAL_INTERRUPT_MODE */
    usart0.device.data = &usart0;

#ifdef SERIAL_INTERRUPT_MODE
    /* Register this serial device. */
    usart0_buffer_data.buffer_space = usart0_buffer_space;
    usart0_buffer_data.buffer_size = SERIAL_MAX_BUFFER_SIZE;
    usart0_buffer_data.buffer_ones = usart0_buffer_ones;
    usart0_buffer_data.num_buffer_ones = SERIAL_NUM_BUFFERS;
    usart0_buffer_data.buffer_lists = usart0_buffer_lists;
    usart0_buffer_data.num_buffer_lists = SERIAL_NUM_BUFFER_LIST;
    serial_register(&usart0, "usart0", &usart0_buffer_data, (SERIAL_DEBUG | SERIAL_INT));
#else
    serial_register(&usart0, "usart0", NULL, (SERIAL_DEBUG));
#endif /* SERIAL_INTERRUPT_MODE */

} /* serial_atmega644p_init */

/*
 * usart_atmega644p_init
 * This function initializes UART1 for ATmega644p.
 */
static int32_t usart_atmega644p_init(void *data)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(data);

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

    /* Always return success. */
    return (SUCCESS);

} /* usart_atmega644p_init */

#ifdef SERIAL_INTERRUPT_MODE
/*
 * ISR(USART0_TX_vect, ISR_NAKED)
 * This is USART0 TX interrupt.
 */
ISR(USART0_TX_vect, ISR_NAKED)
{
    ISR_ENTER();

    /* Handle TX interrupt. */
    usart0_handle_tx_interrupt();

    ISR_EXIT();

} /* ISR(USART0_TX_vect, ISR_NAKED) */

/*
 * usart0_handle_tx_interrupt
 * This function handles TX interrupt.
 */
void usart0_handle_tx_interrupt()
{
    FS_BUFFER *buffer;
    uint8_t chr;

    /* Get a buffer to be transmitted. */
    buffer = fs_buffer_get(&usart0, FS_BUFFER_TX, FS_BUFFER_INPLACE);

    /* If we have a buffer to transmit. */
    if (buffer != NULL)
    {
        /* Pull a byte from the buffer. */
        fs_buffer_pull(buffer, &chr, 1, 0);

        /* If there is nothing more to be sent from this buffer. */
        if (buffer->total_length == 0)
        {
            /* Actually remove this buffer. */
            buffer = fs_buffer_get(&usart0, FS_BUFFER_TX, 0);

            /* Free this buffer. */
            fs_buffer_add(&usart0, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Put a byte on USART to continue TX. */
        UDR0 = ((uint32_t)chr) & 0xFF;
    }
    else
    {
        /* Clear the in TX flag. */
        usart0.flags &= (uint32_t)(~SERIAL_IN_TX);
    }

} /* usart0_handle_tx_interrupt */

/*
 * ISR(USART0_RX_vect, ISR_NAKED)
 * This is USART0 RX interrupt.
 */
ISR(USART0_RX_vect, ISR_NAKED)
{
    ISR_ENTER();

    /* Handle RX interrupt. */
    usart0_handle_rx_interrupt();

    ISR_EXIT();

} /* ISR(USART0_RX_vect, ISR_NAKED) */

/*
 * usart0_handle_rx_interrupt
 * This function handles RX interrupt.
 */
void usart0_handle_rx_interrupt()
{
    FS_BUFFER *buffer;
    uint8_t chr;

    /* Get a RX buffer. */
    buffer = fs_buffer_get(&usart0, FS_BUFFER_RX, FS_BUFFER_INPLACE);

    /* If we don't have a buffer. */
    if (buffer == NULL)
    {
        /* Get a buffer. */
        buffer = fs_buffer_get(&usart0, FS_BUFFER_LIST, 0);

        /* If we do have a buffer. */
        if (buffer != NULL)
        {
            /* Add this buffer on the receive list. */
            fs_buffer_add(&usart0, buffer, FS_BUFFER_RX, 0);
        }
    }

    /* Read the incoming data and also clear the interrupt. */
    chr = (uint8_t)UDR0;

    /* If we do have a buffer. */
    if (buffer != NULL)
    {
        /* Append received byte on the buffer. */
        fs_buffer_push(buffer, &chr, 1, 0);

        /* Tell upper layers that some data is available to read. */
        fd_data_available(&usart0);
    }

} /* usart0_handle_rx_interrupt */

/*
 * usart_atmega644p_enable_interrupt.
 * This function will enable interrupts for the giver USART.
 */
static void usart_atmega644p_enable_interrupt(void *data)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(data);

    /* Enable the USART0 IRQ channel. */
    UCSR0B |= ((1 << RXCIE0) | (1 << TXCIE0));

} /* usart_atmega644p_enable_interrupt */
#endif /* SERIAL_INTERRUPT_MODE */

/*
 * usart_atmega644p_disable_interrupt.
 * This function will disable interrupts for the giver USART.
 */
static void usart_atmega644p_disable_interrupt(void *data)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(data);

    /* Disable the USART0 IRQ channel. */
    UCSR0B &= ((uint8_t)~((1 << RXCIE0) | (1 << TXCIE0)));

} /* usart_atmega644p_disable_interrupt */

/*
 * usart_atmega644p_puts
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data needed to be sent over USART.
 * @nbytes: Number of bytes to be printed from the buffer.
 * @flags: Flags to specify the operation.
 * This function sends a buffer on the given USART.
 */
static int32_t usart_atmega644p_puts(void *fd, void *priv_data, uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_print = nbytes;
    SERIAL *usart = (SERIAL *)priv_data;
#ifdef SERIAL_INTERRUPT_MODE
    FS_BUFFER *buffer;
    uint8_t chr;
#else

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(flags);
#endif /* SERIAL_INTERRUPT_MODE */

#ifdef SERIAL_INTERRUPT_MODE
    /* If we need to put this string using interrupts. */
    if (flags & SERIAL_INT)
    {
        /* Get a buffer to be transmitted. */
        buffer = fs_buffer_get(fd, FS_BUFFER_TX, FS_BUFFER_INPLACE);

        /* If we have a buffer and we are in TX. */
        if ((buffer != NULL) && ((usart->flags & SERIAL_IN_TX) == 0))
        {
            /* Set the in TX flag. */
            usart->flags |= SERIAL_IN_TX;

            /* Pull a byte from the buffer. */
            fs_buffer_pull(buffer, &chr, 1, 0);

            if (buffer->total_length == 0)
            {
                /* Actually remove this buffer. */
                buffer = fs_buffer_get(fd, FS_BUFFER_TX, 0);

                /* Free this buffer. */
                fs_buffer_add(fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            }

            /* Put a byte on USART to start TX. */
            UDR0 = (chr);
        }
    }
    else
#endif /* SERIAL_INTERRUPT_MODE */
    {
        /* Disable USART interrupts. */
        usart_atmega644p_disable_interrupt(usart);

        /* While we have some data to be printed. */
        while (nbytes > 0)
        {
            /* Add this byte. */
            UDR0 = (*buf);

            /* Wait for this byte to be sent. */
            while ((UCSR0A & (1 << UDRE0)) == 0)
            {
                ;
            }

            /* Decrement number of bytes remaining. */
            nbytes --;

            /* Move forward in the buffer. */
            buf++;
        }
    }

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* usart_atmega644p_puts */

/*
 * usart_atmega644p_gets
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data received will be returned in this buffer.
 * @nbytes: Number of bytes received on serial port.
 * @flags: For now unused.
 * This function receives and return data from a serial port.
 */
static int32_t usart_atmega644p_gets(void *fd, void *priv_data, uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_read = nbytes;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(priv_data);
    UNUSED_PARAM(flags);

    /* Disable USART interrupts. */
    usart_atmega644p_disable_interrupt(&usart0);

    /* While we have some data to be printed. */
    while (nbytes > 0)
    {
        /* While we have a byte to read. */
        while ((UCSR0A & (1 << RXC0)) == 0)
        {
            ;
        }

        /* Read a byte from USART. */
        *buf = (uint8_t)UDR0;

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes read. */
    return (to_read - nbytes);

} /* usart_atmega644p_gets */
#endif /* CONFIG_SERIAL */
