/*
 * usart_avr.c
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
#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <usart_avr.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef SERIAL_INTERRUPT_MODE
#ifndef FS_CONSOLE
#error "Console is required for interrupt mode serial."
#endif /* FS_CONSOLE */
#endif /* SERIAL_INTERRUPT_MODE */

/* Internal function prototypes. */
static int32_t usart_avr_init(void *);
void usart_handle_tx_interrupt(AVR_USART *);
void usart_handle_rx_interrupt(AVR_USART *);
static void usart_avr_enable_interrupt(void *);
static void usart_avr_disable_interrupt(void *);
static int32_t usart_avr_puts(void *, void *, const uint8_t *, int32_t, uint32_t);
static int32_t usart_avr_gets(void *, void *, uint8_t *, int32_t, uint32_t);

/* Target serial port. */
static AVR_USART usart0;
#ifdef SERIAL_INTERRUPT_MODE
static FS_BUFFER_DATA usart0_buffer_data;
static uint8_t usart0_buffer_space[SERIAL_MAX_BUFFER_SIZE * SERIAL_NUM_BUFFERS];
static FS_BUFFER usart0_buffer_ones[SERIAL_NUM_BUFFERS];
static FS_BUFFER_LIST usart0_buffer_lists[SERIAL_NUM_BUFFER_LIST];
#endif /* SERIAL_INTERRUPT_MODE */

/*
 * serial_avr_init
 * This will initialize serial interface(s) for this target.
 */
void serial_avr_init(void)
{
    /* Initialize serial device data. */
    usart0.serial.device.init = &usart_avr_init;
    usart0.serial.device.puts = &usart_avr_puts;
    usart0.serial.device.gets = &usart_avr_gets;
    usart0.serial.device.data = &usart0;

    /* Initialize AVR debug USART data. */
    usart0.num = 0;
    usart0.baudrate = SERIAL_BAUD_RATE;
#ifdef SERIAL_ENABLE_HW_FLOW
    usart0.hw_flow_enabled = TRUE;
#else
    usart0.hw_flow_enabled = FALSE;
#endif /* SERIAL_ENABLE_HW_FLOW */

#ifdef SERIAL_INTERRUPT_MODE
    /* Initialize buffer data. */
    usart0_buffer_data.buffer_space = usart0_buffer_space;
    usart0_buffer_data.buffer_size = SERIAL_MAX_BUFFER_SIZE;
    usart0_buffer_data.buffers = usart0_buffer_ones;
    usart0_buffer_data.num_buffers = SERIAL_NUM_BUFFERS;
    usart0_buffer_data.buffer_lists = usart0_buffer_lists;
    usart0_buffer_data.num_buffer_lists = SERIAL_NUM_BUFFER_LIST;

    /* Register an AVR USART. */
    usart_avr_register(&usart0, "usart0", &usart0_buffer_data, TRUE);
#else
    /* Register an AVR USART. */
    usart_avr_register(&usart0, "usart0", NULL, TRUE);
#endif /* SERIAL_INTERRUPT_MODE */

} /* serial_avr_init */

/*
 * usart_avr_register
 * usart: AVR USART to be registered.
 * name: Name of this USART.
 * buffer_data: Buffer data for this USART.
 * is_debug: If this is a debugging USART.
 * This will register an AVR USART device.
 */
void usart_avr_register(AVR_USART *usart, const char *name, FS_BUFFER_DATA *buffer_data, uint8_t is_debug)
{
    uint32_t flags;

    /* If this is a debug USART. */
    if (is_debug)
    {
        /* Set debug flag. */
        flags = SERIAL_DEBUG;
    }
    else
    {
        /* Clear debug flag. */
        flags = 0;
    }

    /* If we buffer data for this device. */
    if (buffer_data != NULL)
    {
        /* Enable interrupt flag. */
        flags |= SERIAL_INT;

        /* Setup interrupt manipulation APIs. */
        usart->serial.device.int_lock = &usart_avr_disable_interrupt;
        usart->serial.device.int_unlock = &usart_avr_enable_interrupt;

        /* Register this USART with serial file system. */
        serial_register(&usart->serial, name, buffer_data, flags);
    }
    else
    {
        /* Register this USART with serial file system. */
        serial_register(&usart->serial, name, NULL, flags);
    }

} /* usart_avr_register */

/*
 * usart_avr_init
 * This function initializes UART1 for AVR.
 */
static int32_t usart_avr_init(void *data)
{
    AVR_USART *usart = (AVR_USART *)data;
    int32_t status = SUCCESS;
    uint16_t ubrr_val_u1x, ubrr_val_u2x;
    double baud_u1x, baud_u2x, error_u1x, error_u2x;

    /* Calculate the UBRR values. */
    ubrr_val_u1x = (((F_CPU) - (8UL * usart->baudrate)) / (16UL * usart->baudrate));
    ubrr_val_u2x = (((F_CPU) - (4UL * usart->baudrate)) / (8UL * usart->baudrate));

    /* Recalculate resulting baudrate. */
    baud_u1x = F_CPU / ((16UL * ubrr_val_u1x) + 8UL);
    baud_u2x = F_CPU / ((8UL * ubrr_val_u1x) + 4UL);

    /* Calculate the error rating. */
    error_u1x = (baud_u1x / usart->baudrate) - 1;
    error_u2x = (baud_u2x / usart->baudrate) - 1;

    /* Build absolute value of error. */
    if (error_u1x < 0)
    {
        error_u1x *= -1.0;
    }

    /* Build absolute value of error. */
    if (error_u2x < 0)
    {
        error_u2x *= -1.0;
    }

    /* Process according to the device number. */
    switch (usart->num)
    {
    /* If this is USART0. */
    case 0:

        /* If HW flow control is enabled. */
        if (usart->hw_flow_enabled == TRUE)
        {
            /* Configure CTS pin as input.  */
            USART0_HW_CTS_DDR &= (uint8_t)~(1 << USART0_HW_CTS);

            /* Configure RTS pin as input.  */
            USART0_HW_RTS_DDR &= (uint8_t)~(1 << USART0_HW_RTS);

            /* Configure RTS reset pin as output. */
            USART0_HW_RTS_RESET_DDR |= (1 << USART0_HW_RTS_RESET);
            USART0_HW_RTS_RESET_PORT |= (1 << USART0_HW_RTS_RESET);

            /* If RTS is in reset state. */
            if (!(USART0_HW_RTS_PIN & (1 << USART0_HW_RTS)))
            {
                /* Toggle RTS reset. */
                USART0_HW_RTS_RESET_PORT &= (uint8_t)~(1 << USART0_HW_RTS_RESET);
                while (!(USART0_HW_RTS_PIN & (1 << USART0_HW_RTS))) ;
                USART0_HW_RTS_RESET_PORT |= (1 << USART0_HW_RTS_RESET);
            }
        }

        /* Only choose U2X if error rating is better. */
        if (error_u1x <= error_u2x)
        {
            UCSR0A &= (uint8_t)~(1 << U2X0);
            UBRR0H = ubrr_val_u1x >> 8;
            UBRR0L = ubrr_val_u1x  & 0xFF;
        }
        else
        {
            UCSR0A |= (1 << U2X0);
            UBRR0H = ubrr_val_u2x >> 8;
            UBRR0L = ubrr_val_u2x  & 0xFF;
        }

        /* Enable RX and TX. */
        UCSR0B = (1 << RXEN0)  | (1 << TXEN0);
        UCSR0C = (1 << USBS0) | (3 << UCSZ00);

        break;
    }

    /* Return status to the caller. */
    return (status);

} /* usart_avr_init */

/*
 * ISR(USART0_TX_vect, ISR_NAKED)
 * This is USART0 TX interrupt.
 */
ISR(USART0_TX_vect, ISR_NAKED)
{
    ISR_ENTER();

    /* Handle TX interrupt. */
    usart_handle_tx_interrupt(&usart0);

    ISR_EXIT();

} /* ISR(USART0_TX_vect, ISR_NAKED) */

/*
 * usart_handle_tx_interrupt
 * This function handles TX interrupt.
 */
void usart_handle_tx_interrupt(AVR_USART *usart)
{
    FS_BUFFER_LIST *buffer;
    uint8_t chr;

    /* Get a buffer to be transmitted. */
    buffer = fs_buffer_get(usart, FS_BUFFER_TX, FS_BUFFER_INPLACE);

    /* If we have a buffer to transmit. */
    if (buffer != NULL)
    {
        /* Pull a byte from the buffer. */
        fs_buffer_list_pull(buffer, &chr, 1, 0);

        /* If there is nothing more to be sent from this buffer. */
        if (buffer->total_length == 0)
        {
            /* Actually remove this buffer. */
            buffer = fs_buffer_get(usart, FS_BUFFER_TX, 0);

            /* Free this buffer. */
            fs_buffer_add(usart, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
        }

        /* Process according to the device number. */
        switch (usart->num)
        {
        /* If this is USART0. */
        case 0:
            /* If HW flow control is enabled. */
            if (usart->hw_flow_enabled == TRUE)
            {
                /* Wait for CTS signal. */
                while (USART0_HW_CTS_PIN & (1 << USART0_HW_CTS)) ;
            }

            /* Put a byte on USART to continue TX. */
            UDR0 = ((uint32_t)chr) & 0xFF;

            break;
        }
    }
    else
    {
        /* Clear the in TX flag. */
        usart->serial.flags &= (uint32_t)(~SERIAL_IN_TX);
    }

} /* usart_handle_tx_interrupt */

/*
 * ISR(USART0_RX_vect, ISR_NAKED)
 * This is USART0 RX interrupt.
 */
ISR(USART0_RX_vect, ISR_NAKED)
{
    ISR_ENTER();

    /* While we have a byte to read. */
    while (UCSR0A & (1 << RXC0))
    {
        /* Handle RX interrupt. */
        usart_handle_rx_interrupt(&usart0);
    }

    ISR_EXIT();

} /* ISR(USART0_RX_vect, ISR_NAKED) */

/*
 * usart_handle_rx_interrupt
 * This function handles RX interrupt.
 */
void usart_handle_rx_interrupt(AVR_USART *usart)
{
    FS_BUFFER_LIST *buffer;
    uint8_t chr;

    /* Process according to the device number. */
    switch (usart->num)
    {
    /* If this is USART0. */
    case 0:
        /* Read the incoming data and also clear the interrupt. */
        chr = (uint8_t)UDR0;

        /* If HW flow control is enabled. */
        if (usart->hw_flow_enabled == TRUE)
        {
            /* If RTS is in reset state. */
            if (!(USART0_HW_RTS_PIN & (1 << USART0_HW_RTS)))
            {
                /* Toggle RTS reset. */
                USART0_HW_RTS_RESET_PORT &= (uint8_t)~(1 << USART0_HW_RTS_RESET);
                while (!(USART0_HW_RTS_PIN & (1 << USART0_HW_RTS))) ;
                USART0_HW_RTS_RESET_PORT |= (1 << USART0_HW_RTS_RESET);
            }
        }
        break;
    }

    /* Get a RX buffer. */
    buffer = fs_buffer_get(usart, FS_BUFFER_RX, FS_BUFFER_INPLACE);

    /* If we don't have a buffer. */
    if (buffer == NULL)
    {
        /* Get a buffer. */
        buffer = fs_buffer_get(usart, FS_LIST_FREE, 0);

        /* If we do have a buffer. */
        if (buffer != NULL)
        {
            /* Add this buffer on the receive list. */
            fs_buffer_add(usart, buffer, FS_BUFFER_RX, 0);
        }
    }

    /* If we do have a buffer. */
    if (buffer != NULL)
    {
        /* Append received byte on the buffer. */
        fs_buffer_list_push(buffer, &chr, 1, 0);

        /* Tell upper layers that some data is available to read. */
        fd_data_available(usart);
    }

} /* usart_handle_rx_interrupt */

/*
 * usart_avr_enable_interrupt.
 * This function will enable interrupts for the given USART.
 */
static void usart_avr_enable_interrupt(void *data)
{
    AVR_USART *usart = (AVR_USART *)data;

    /* Process according to the device number. */
    switch (usart->num)
    {
    /* If this is USART0. */
    case 0:
        /* Enable the USART0 IRQ channel. */
        UCSR0B |= ((1 << RXCIE0) | (1 << TXCIE0));

        break;
    }

} /* usart_avr_enable_interrupt */

/*
 * usart_avr_disable_interrupt.
 * This function will disable interrupts for the given USART.
 */
static void usart_avr_disable_interrupt(void *data)
{
    AVR_USART *usart = (AVR_USART *)data;

    /* Process according to the device number. */
    switch (usart->num)
    {
    /* If this is USART0. */
    case 0:
        /* Disable the USART0 IRQ channel. */
        UCSR0B &= ((uint8_t)~((1 << RXCIE0) | (1 << TXCIE0)));

        break;
    }

} /* usart_avr_disable_interrupt */

/*
 * usart_avr_puts
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data needed to be sent over USART.
 * @nbytes: Number of bytes to be printed from the buffer.
 * @flags: Flags to specify the operation.
 * This function sends a buffer on the given USART.
 */
static int32_t usart_avr_puts(void *fd, void *priv_data, const uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_print = nbytes;
    AVR_USART *usart = (AVR_USART *)priv_data;
    FS_BUFFER_LIST *buffer;
    uint8_t chr;

    /* If we need to put this string using interrupts. */
    if (flags & SERIAL_INT)
    {
        /* Get a buffer to be transmitted. */
        buffer = fs_buffer_get(fd, FS_BUFFER_TX, FS_BUFFER_INPLACE);

        /* If we have a buffer and we are in TX. */
        if ((buffer != NULL) && ((usart->serial.flags & SERIAL_IN_TX) == 0))
        {
            /* Set the in TX flag. */
            usart->serial.flags |= SERIAL_IN_TX;

            /* Pull a byte from the buffer. */
            fs_buffer_list_pull(buffer, &chr, 1, 0);

            if (buffer->total_length == 0)
            {
                /* Actually remove this buffer. */
                buffer = fs_buffer_get(fd, FS_BUFFER_TX, 0);

                /* Free this buffer. */
                fs_buffer_add(fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);
            }

            /* Process according to the device number. */
            switch (usart->num)
            {
            /* If this is USART0. */
            case 0:
                /* If HW flow control is enabled. */
                if (usart->hw_flow_enabled == TRUE)
                {
                    /* Wait for CTS signal. */
                    while (USART0_HW_CTS_PIN & (1 << USART0_HW_CTS)) ;
                }

                /* Put a byte on USART to start TX. */
                UDR0 = (chr);

                break;
            }
        }
    }
    else
    {
        /* Disable USART interrupts. */
        usart_avr_disable_interrupt(usart);

        /* Process according to the device number. */
        switch (usart->num)
        {
        /* If this is USART0. */
        case 0:
            /* While we have some data to be printed. */
            while (nbytes > 0)
            {
                /* If HW flow control is enabled. */
                if (usart->hw_flow_enabled == TRUE)
                {
                    /* Wait for CTS signal. */
                    while (USART0_HW_CTS_PIN & (1 << USART0_HW_CTS)) ;
                }

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

            break;
        }
    }

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* usart_avr_puts */

/*
 * usart_avr_gets
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data received will be returned in this buffer.
 * @nbytes: Number of bytes received on serial port.
 * @flags: For now unused.
 * This function receives and return data from a serial port.
 */
static int32_t usart_avr_gets(void *fd, void *priv_data, uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    AVR_USART *usart = (AVR_USART *)priv_data;
    int32_t to_read = nbytes;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(flags);

    /* Disable USART interrupts. */
    usart_avr_disable_interrupt(usart);

    /* Process according to the device number. */
    switch (usart->num)
    {
    /* If this is USART0. */
    case 0:

        /* While we have some data to be printed. */
        while (nbytes > 0)
        {
            /* While we have a byte to read. */
            while ((UCSR0A & (1 << RXC0)) == 0)
            {
                /* Yield the current task. */
                task_yield();
            }

            /* Read a byte from USART. */
            *buf = (uint8_t)UDR0;

            /* If HW flow control is enabled. */
            if (usart->hw_flow_enabled == TRUE)
            {
                /* If RTS is in reset state. */
                if (!(USART0_HW_RTS_PIN & (1 << USART0_HW_RTS)))
                {
                    /* Toggle RTS reset. */
                    USART0_HW_RTS_RESET_PORT &= (uint8_t)~(1 << USART0_HW_RTS_RESET);
                    while (!(USART0_HW_RTS_PIN & (1 << USART0_HW_RTS))) ;
                    USART0_HW_RTS_RESET_PORT |= (1 << USART0_HW_RTS_RESET);
                }
            }

            /* Decrement number of bytes remaining. */
            nbytes --;

            /* Move forward in the buffer. */
            buf++;
        }

        break;
    }

    /* Return number of bytes read. */
    return (to_read - nbytes);

} /* usart_avr_gets */
#endif /* CONFIG_SERIAL */
