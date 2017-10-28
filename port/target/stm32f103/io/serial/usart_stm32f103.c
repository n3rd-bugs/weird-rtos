/*
 * usart_stm32f103.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
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
#include <usart_stm32f103.h>
#include <serial.h>

#ifdef SERIAL_INTERRUPT_MODE
#ifndef FS_CONSOLE
#error "Console is required for interrupt mode serial."
#endif /* FS_CONSOLE */
#endif /* SERIAL_INTERRUPT_MODE */

/* Internal function prototypes. */
static int32_t usart_stm32f103_init(void *);
#ifdef SERIAL_INTERRUPT_MODE
static void usart1_handle_tx_interrupt(void);
static void usart1_handle_rx_interrupt(void);
static void usart_stm32f103_enable_interrupt(void *);
#endif /* SERIAL_INTERRUPT_MODE */
static void usart_stm32f103_disable_interrupt(void *);
static int32_t usart_stm32f103_puts(void *, void *, const uint8_t *, int32_t, uint32_t);
static int32_t usart_stm32f103_gets(void *, void *, uint8_t *, int32_t, uint32_t);

/* Target serial port. */
static SERIAL usart1;
#ifdef SERIAL_INTERRUPT_MODE
static FS_BUFFER_DATA usart1_buffer_data;
static uint8_t usart1_buffer_space[SERIAL_MAX_BUFFER_SIZE * SERIAL_NUM_BUFFERS];
static FS_BUFFER_ONE usart1_buffer_ones[SERIAL_NUM_BUFFERS];
static FS_BUFFER usart1_buffer_lists[SERIAL_NUM_BUFFER_LIST];
#endif /* SERIAL_INTERRUPT_MODE */

/*
 * serial_stm32f103_init
 * This will initialize serial interface(s) for this target.
 */
void serial_stm32f103_init(void)
{
    /* Initialize serial device data. */
    usart1.device.init = &usart_stm32f103_init;
    usart1.device.puts = &usart_stm32f103_puts;
    usart1.device.gets = &usart_stm32f103_gets;
#ifdef SERIAL_INTERRUPT_MODE
    usart1.device.int_lock = &usart_stm32f103_disable_interrupt;
    usart1.device.int_unlock = &usart_stm32f103_enable_interrupt;
#endif /* SERIAL_INTERRUPT_MODE */
    usart1.device.data = &usart1;

#ifdef SERIAL_INTERRUPT_MODE
    /* Register this serial device. */
    usart1_buffer_data.buffer_space = usart1_buffer_space;
    usart1_buffer_data.buffer_size = SERIAL_MAX_BUFFER_SIZE;
    usart1_buffer_data.buffer_ones = usart1_buffer_ones;
    usart1_buffer_data.num_buffer_ones = SERIAL_NUM_BUFFERS;
    usart1_buffer_data.buffer_lists = usart1_buffer_lists;
    usart1_buffer_data.num_buffer_lists = SERIAL_NUM_BUFFER_LIST;
    usart1_buffer_data.threshold_buffers = SERIAL_THRESHOLD_BUFFER;
    usart1_buffer_data.threshold_lists = SERIAL_THRESHOLD_BUFFER_LIST;
    serial_register(&usart1, "usart1", &usart1_buffer_data, (SERIAL_DEBUG | SERIAL_INT));
#else
    serial_register(&usart1, "usart1", NULL, (SERIAL_DEBUG));
#endif /* SERIAL_INTERRUPT_MODE */

} /* serial_stm32f103_init */

/*
 * usart_stm32f103_init
 * This function initializes UART1 for STM32F103.
 */
static int32_t usart_stm32f103_init(void *data)
{
    uint32_t temp, integral, fractional;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(data);

    /* Reset USART1. */
    RCC->APB2RSTR |= RCC_APB2Periph_USART1;
    RCC->APB2RSTR &= (uint32_t)~RCC_APB2Periph_USART1;

    /* Enable clock for USART1. */
    RCC->APB2ENR |= RCC_APB2Periph_USART1;

    /* Enable clock for GPIOA. */
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

    /* Set alternate function for PA9 (TX) and PA10 (RX). */
    GPIOA->CRH &= (uint32_t)(~((0x0F << ((9 - 8) << 2)) | (0x0F << ((10 - 8) << 2))));
    GPIOA->CRH |= (((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << ((9 - 8) << 2));
    GPIOA->CRH |= (((GPIO_Mode_IN_FLOATING) & 0x0F) << ((10 - 8) << 2));

    /* Enable RX and TX for this USART, also use 8-bit word length and
     * disable parity bit. */
    USART1->CR1 &= (uint16_t)~((USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE));
    USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE);

    /* Use one stop bit for this USART. */
    USART1->CR2 &= (uint16_t)~(USART_CR2_STOP);

    /* Don't use CTS/RTS signals. */
    USART1->CR3 &= (uint16_t)~((USART_CR3_RTSE | USART_CR3_CTSE));

    /* Calculate and set the baud rate parameters. */
    integral = ((25 * PCLK_FREQ) / (4 * BAUD_RATE));
    temp = (integral / 100) << 4;
    fractional = integral - (100 * (temp >> 4));
    temp |= ((((fractional * 16) + 50) / 100)) & (0x0F);
    USART1->BRR = (uint16_t)temp;

    /* Enable USART1. */
    USART1->CR1 |= USART_CR1_UE;

    /* Enable transmission complete and receive data available interrupts. */
    USART1->CR1 |= (USART_CR1_TCIE | USART_CR1_RXNEIE);

    /* Always return success. */
    return (SUCCESS);

} /* usart_stm32f103_init */

#ifdef SERIAL_INTERRUPT_MODE
/*
 * usart1_interrupt
 * This function is interrupt handler for USART1 interrupt.
 */
ISR_FUN usart1_interrupt(void)
{
    ISR_ENTER();

    /* If a transmission was successfully completed. */
    if (USART1->SR & USART_SR_TC)
    {
        /* Handle transmission complete interrupt. */
        usart1_handle_tx_interrupt();
    }

    /* If some data is available to read. */
    if (USART1->SR & USART_SR_RXNE)
    {
        /* Handle receive data available interrupt. */
        usart1_handle_rx_interrupt();
    }

    ISR_EXIT();

} /* usart1_interrupt */

/*
 * usart1_handle_tx_interrupt
 * This function handles TX interrupt.
 */
static void usart1_handle_tx_interrupt(void)
{
    FS_BUFFER *buffer;
    uint8_t chr;

    /* Get a buffer to be transmitted. */
    buffer = fs_buffer_get(&usart1, FS_BUFFER_TX, FS_BUFFER_INPLACE);

    /* If we have a buffer to transmit. */
    if (buffer != NULL)
    {
        /* Pull a byte from the buffer. */
        fs_buffer_pull(buffer, &chr, 1, 0);

        /* If there is nothing more to be sent from this buffer. */
        if (buffer->total_length == 0)
        {
            /* Actually remove this buffer. */
            buffer = fs_buffer_get(&usart1, FS_BUFFER_TX, 0);

            /* Free this buffer. */
            fs_buffer_add(&usart1, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Put a byte on USART to continue TX. */
        USART1->DR = ((uint32_t)chr) & 0xFF;
    }
    else
    {
        /* Just clear this interrupt status. */
        USART1->SR &= (uint16_t)(~USART_SR_TC);

        /* Clear the in TX flag. */
        usart1.flags &= (uint32_t)(~SERIAL_IN_TX);
    }

} /* usart1_handle_tx_interrupt */

/*
 * usart1_handle_rx_interrupt
 * This function handles RX interrupt.
 */
static void usart1_handle_rx_interrupt(void)
{
    FS_BUFFER *buffer;
    uint8_t chr;

    /* Get a RX buffer. */
    buffer = fs_buffer_get(&usart1, FS_BUFFER_RX, FS_BUFFER_INPLACE);

    /* If we don't have a buffer. */
    if (buffer == NULL)
    {
        /* Get a buffer. */
        buffer = fs_buffer_get(&usart1, FS_BUFFER_LIST, 0);

        /* If we do have a buffer. */
        if (buffer != NULL)
        {
            /* Add this buffer on the receive list. */
            fs_buffer_add(&usart1, buffer, FS_BUFFER_RX, 0);
        }
    }

    /* Read the incoming data and also clear the interrupt status. */
    chr = (uint8_t)USART1->DR;

    /* If we do have a buffer. */
    if (buffer != NULL)
    {
        /* Append received byte on the buffer. */
        fs_buffer_push(buffer, &chr, 1, 0);

        /* Tell upper layers that some data is available to read. */
        fd_data_available(&usart1);
    }

} /* usart1_handle_rx_interrupt */

/*
 * usart_stm32f103_enable_interrupt.
 * This function will enable interrupts for the given USART.
 */
static void usart_stm32f103_enable_interrupt(void *data)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(data);

    /* Enable the USART1 IRQ channel. */
    NVIC->ISER[USART1_IRQn >> 0x05] = (uint32_t)0x01 << (USART1_IRQn & (uint8_t)0x1F);

} /* usart_stm32f103_enable_interrupt */
#endif /* SERIAL_INTERRUPT_MODE */

/*
 * usart_stm32f103_disable_interrupt.
 * This function will disable interrupts for the given USART.
 */
static void usart_stm32f103_disable_interrupt(void *data)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(data);

    /* Disable the USART1 IRQ channel. */
    NVIC->ICER[USART1_IRQn >> 0x05] = (uint32_t)0x01 << (USART1_IRQn & (uint8_t)0x1F);

} /* usart_stm32f103_disable_interrupt */

/*
 * usart_stm32f103_puts
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data needed to be sent over USART.
 * @nbytes: Number of bytes to be printed from the buffer.
 * @flags: Flags to specify the operation.
 * This function sends a buffer on the given USART.
 */
static int32_t usart_stm32f103_puts(void *fd, void *priv_data, const uint8_t *buf, int32_t nbytes, uint32_t flags)
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
            USART1->DR = ((uint32_t)chr) & 0xFF;
        }
    }
    else
#endif /* SERIAL_INTERRUPT_MODE */
    {
        /* Disable USART interrupts. */
        usart_stm32f103_disable_interrupt(usart);

        /* While we have some data to be printed. */
        while (nbytes > 0)
        {
            /* Put a byte on USART. */
            USART1->DR = ((uint32_t)*buf) & 0xFF;

            /* Decrement number of bytes remaining. */
            nbytes --;

            /* Move forward in the buffer. */
            buf++;

            /* Wait for transmission of the last byte. */
            while (!(USART1->SR & USART_SR_TC))
            {
                ;
            }
        }
    }

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* usart_stm32f103_puts */

/*
 * usart_stm32f103_gets
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data received will be returned in this buffer.
 * @nbytes: Number of bytes received on serial port.
 * @flags: For now unused.
 * This function receives and return data from a serial port.
 */
static int32_t usart_stm32f103_gets(void *fd, void *priv_data, uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_read = nbytes;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(priv_data);
    UNUSED_PARAM(flags);

    /* Disable USART interrupts. */
    usart_stm32f103_disable_interrupt(&usart1);

    /* While we have some data to be printed. */
    while (nbytes > 0)
    {
        /* While we have a byte to read. */
        while (!(USART1->SR & USART_SR_RXNE))
        {
            ;
        }

        /* Read a byte from USART. */
        *buf = (uint8_t)USART1->DR;

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes read. */
    return (to_read - nbytes);

} /* usart_stm32f103_gets */
#endif /* CONFIG_SERIAL */
