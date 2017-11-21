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

#ifdef SERIAL_INTERRUPT_MODE
#ifndef FS_CONSOLE
#error "Console is required for interrupt mode serial."
#endif /* FS_CONSOLE */
#endif /* SERIAL_INTERRUPT_MODE */

/* Global port data. */
static STM32_USART *usart1_data, *usart2_data;

/* Internal function prototypes. */
static int32_t usart_stm32f103_init(void *);
static void usart_handle_tx_interrupt(STM32_USART *);
static void usart_handle_rx_interrupt(STM32_USART *);
static void usart_stm32f103_enable_interrupt(void *);
static void usart_stm32f103_disable_interrupt(void *);
static int32_t usart_stm32f103_puts(void *, void *, const uint8_t *, int32_t, uint32_t);
static int32_t usart_stm32f103_gets(void *, void *, uint8_t *, int32_t, uint32_t);

/* USART1 data. */
static STM32_USART usart1;
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
    usart_stm32f103_register(&usart1, "usart1", 1, BAUD_RATE, &usart1_buffer_data, TRUE);
#else
    usart_stm32f103_register(&usart1, "usart1", 1, BAUD_RATE, NULL, TRUE);
#endif /* SERIAL_INTERRUPT_MODE */

} /* serial_stm32f103_init */

/*
 * usart_stm32f103_register
 * @usart: STM32 USART instance t be registered.
 * @name: Name of this USART instance.
 * @device_num: USART device number we need to register.
 * @boud_rate: USART baud rate.
 * @buffer_data: Buffer data for this USART, if not null USART interrupt mode
 *  will be enabled.
 * @is_debug: If this USART is needed to be used as debug console.
 * @return: Success will be returned if USART was successfully registered.
 * This function will register a USART for STM32 platform.
 */
int32_t usart_stm32f103_register(STM32_USART *usart, const char *name, uint8_t device_num, uint32_t baud_rate, FS_BUFFER_DATA *buffer_data, uint8_t is_debug)
{
    uint32_t usart_flags = ((is_debug == TRUE) ? SERIAL_DEBUG : 0);

    /* Save the USART data. */
    usart->device_num = device_num;
    usart->baud_rate = baud_rate;

    /* Initialize USART GPIO. */
    switch (usart->device_num)
    {
    /* If this is USART1 device. */
    case 1:

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

        /* Save the USART register. */
        usart->reg = USART1;

        /* Save the USART1 data. */
        usart1_data = usart;

        break;

    /* If this is USART2 device. */
    case 2:

        /* Reset USART2. */
        RCC->APB1RSTR |= RCC_APB1Periph_USART2;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1Periph_USART2;

        /* Enable clock for USART2. */
        RCC->APB1ENR |= RCC_APB1Periph_USART2;

        /* Enable clock for GPIOA. */
        RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

        /* Set alternate function for PA2 (TX) and PA3 (RX). */
        GPIOA->CRL &= (uint32_t)(~((0x0F << ((2 - 0) << 2)) | (0x0F << ((3 - 0) << 2))));
        GPIOA->CRL |= (((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << ((2 - 0) << 2));
        GPIOA->CRL |= (((GPIO_Mode_IN_FLOATING) & 0x0F) << ((3 - 0) << 2));

        /* Save the USART register. */
        usart->reg = USART2;

        /* Save the USART2 data. */
        usart2_data = usart;

        break;
    }

    /* Initialize serial device data. */
    usart->serial.device.init = &usart_stm32f103_init;
    usart->serial.device.puts = &usart_stm32f103_puts;
    usart->serial.device.gets = &usart_stm32f103_gets;

    /* If we need to use interrupts for this USART. */
    if (buffer_data)
    {
        /* Hook-up interrupt locks. */
        usart->serial.device.int_lock = &usart_stm32f103_disable_interrupt;
        usart->serial.device.int_unlock = &usart_stm32f103_enable_interrupt;

        /* Update USART flags. */
        usart_flags |= SERIAL_INT;
    }

    /* Save the device data. */
    usart->serial.device.data = usart;

    /* Register this USARt as a serial device. */
    serial_register(&usart->serial, name, buffer_data, usart_flags);

    /* Always return success. */
    return (SUCCESS);

} /* usart_stm32f103_register */

/*
 * usart_stm32f103_init
 * @data: USART device data.
 * @return: Will always return success.
 * This function initializes USART1 for STM32F103.
 */
static int32_t usart_stm32f103_init(void *data)
{
    STM32_USART *usart = (STM32_USART *)data;
    uint32_t temp, integral, fractional;

    /* Enable RX and TX for this USART, also use 8-bit word length and
     * disable parity bit. */
    usart->reg->CR1 &= (uint16_t)~((USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE));
    usart->reg->CR1 |= (USART_CR1_RE | USART_CR1_TE);

    /* Use one stop bit for this USART. */
    usart->reg->CR2 &= (uint16_t)~(USART_CR2_STOP);

    /* Don't use CTS/RTS signals. */
    usart->reg->CR3 &= (uint16_t)~((USART_CR3_RTSE | USART_CR3_CTSE));

    /* Calculate and set the baud rate parameters. */
    if (usart->reg == USART1)
    {
        integral = ((25 * PCLK2_FREQ) / (4 * usart->baud_rate));
    }
    else
    {
        integral = ((25 * PCLK1_FREQ) / (4 * usart->baud_rate));
    }
    temp = (integral / 100) << 4;
    fractional = integral - (100 * (temp >> 4));
    temp |= ((((fractional * 16) + 50) / 100)) & (0x0F);
    usart->reg->BRR = (uint16_t)temp;

    /* Enable USART. */
    usart->reg->CR1 |= USART_CR1_UE;

    /* Enable transmission complete and receive data available interrupts. */
    usart->reg->CR1 |= (USART_CR1_TCIE | USART_CR1_RXNEIE);

    /* Always return success. */
    return (SUCCESS);

} /* usart_stm32f103_init */

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
        usart_handle_tx_interrupt(usart1_data);
    }

    /* If some data is available to read. */
    if (USART1->SR & USART_SR_RXNE)
    {
        /* Handle receive data available interrupt. */
        usart_handle_rx_interrupt(usart1_data);
    }

    ISR_EXIT();

} /* usart1_interrupt */

/*
 * usart2_interrupt
 * This function is interrupt handler for USART2 interrupt.
 */
ISR_FUN usart2_interrupt(void)
{
    ISR_ENTER();

    /* If a transmission was successfully completed. */
    if (USART2->SR & USART_SR_TC)
    {
        /* Handle transmission complete interrupt. */
        usart_handle_tx_interrupt(usart2_data);
    }

    /* If some data is available to read. */
    if (USART2->SR & USART_SR_RXNE)
    {
        /* Handle receive data available interrupt. */
        usart_handle_rx_interrupt(usart2_data);
    }

    ISR_EXIT();

} /* usart2_interrupt */

/*
 * usart_handle_tx_interrupt
 * @usart: Device data on which we need to process TX interrupt.
 * This function handles TX interrupt.
 */
static void usart_handle_tx_interrupt(STM32_USART *usart)
{
    FS_BUFFER *buffer;
    uint8_t chr;

    /* Get a buffer to be transmitted. */
    buffer = fs_buffer_get(usart, FS_BUFFER_TX, FS_BUFFER_INPLACE);

    /* If we have a buffer to transmit. */
    if (buffer != NULL)
    {
        /* Pull a byte from the buffer. */
        fs_buffer_pull(buffer, &chr, 1, 0);

        /* If there is nothing more to be sent from this buffer. */
        if (buffer->total_length == 0)
        {
            /* Actually remove this buffer. */
            buffer = fs_buffer_get(usart, FS_BUFFER_TX, 0);

            /* Free this buffer. */
            fs_buffer_add(usart, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Put a byte on USART to continue TX. */
        usart->reg->DR = ((uint32_t)chr) & 0xFF;
    }
    else
    {
        /* Just clear this interrupt status. */
        usart->reg->SR &= (uint16_t)(~USART_SR_TC);

        /* Clear the in TX flag. */
        usart->serial.flags &= (uint32_t)(~SERIAL_IN_TX);
    }

} /* usart_handle_tx_interrupt */

/*
 * usart_handle_rx_interrupt
 * @usart: Device data on which we need to process RX interrupt.
 * This function handles RX interrupt.
 */
static void usart_handle_rx_interrupt(STM32_USART *usart)
{
    FS_BUFFER *buffer;
    uint8_t chr;

    /* Get a RX buffer. */
    buffer = fs_buffer_get(usart, FS_BUFFER_RX, FS_BUFFER_INPLACE);

    /* If we don't have a buffer. */
    if (buffer == NULL)
    {
        /* Get a buffer. */
        buffer = fs_buffer_get(usart, FS_BUFFER_LIST, 0);

        /* If we do have a buffer. */
        if (buffer != NULL)
        {
            /* Add this buffer on the receive list. */
            fs_buffer_add(usart, buffer, FS_BUFFER_RX, 0);
        }
    }

    /* Read the incoming data and also clear the interrupt status. */
    chr = (uint8_t)usart->reg->DR;

    /* If we do have a buffer. */
    if (buffer != NULL)
    {
        /* Append received byte on the buffer. */
        fs_buffer_push(buffer, &chr, 1, 0);

        /* Tell upper layers that some data is available to read. */
        fd_data_available(usart);
    }

} /* usart_handle_rx_interrupt */

/*
 * usart_stm32f103_enable_interrupt.
 * @data: USART for which interrupts are needed to be enabled.
 * This function will enable interrupts for the given USART.
 */
static void usart_stm32f103_enable_interrupt(void *data)
{
    STM32_USART *usart = (STM32_USART *)data;

    /* Process the according to USART device. */
    switch (usart->device_num)
    {
    /* This is USART1 device. */
    case 1:

        /* Enable the USART1 IRQ channel. */
        NVIC->ISER[USART1_IRQn >> 0x05] = (uint32_t)0x01 << (USART1_IRQn & (uint8_t)0x1F);

        break;

    /* This is USART2 device. */
    case 2:

        /* Enable the USART2 IRQ channel. */
        NVIC->ISER[USART2_IRQn >> 0x05] = (uint32_t)0x01 << (USART2_IRQn & (uint8_t)0x1F);

        break;
    }

} /* usart_stm32f103_enable_interrupt */

/*
 * usart_stm32f103_disable_interrupt.
 * @data: USART for which interrupts are needed to be disabled.
 * This function will disable interrupts for the given USART.
 */
static void usart_stm32f103_disable_interrupt(void *data)
{
    STM32_USART *usart = (STM32_USART *)data;

    /* Process the according to USART device. */
    switch (usart->device_num)
    {
    /* This is USART1 device. */
    case 1:

        /* Disable the USART1 IRQ channel. */
        NVIC->ICER[USART1_IRQn >> 0x05] = (uint32_t)0x01 << (USART1_IRQn & (uint8_t)0x1F);

        break;

    /* This is USART2 device. */
    case 2:

        /* Disable the USART2 IRQ channel. */
        NVIC->ICER[USART2_IRQn >> 0x05] = (uint32_t)0x01 << (USART2_IRQn & (uint8_t)0x1F);

        break;
    }

} /* usart_stm32f103_disable_interrupt */

/*
 * usart_stm32f103_puts
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data needed to be sent over USART.
 * @nbytes: Number of bytes to be printed from the buffer.
 * @flags: Flags to specify the operation.
 * @return: Number of bytes printed on the USART.
 * This function sends a buffer on the given USART.
 */
static int32_t usart_stm32f103_puts(void *fd, void *priv_data, const uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_print = nbytes;
    STM32_USART *usart = (STM32_USART *)priv_data;
    FS_BUFFER *buffer;
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
            fs_buffer_pull(buffer, &chr, 1, 0);

            if (buffer->total_length == 0)
            {
                /* Actually remove this buffer. */
                buffer = fs_buffer_get(fd, FS_BUFFER_TX, 0);

                /* Free this buffer. */
                fs_buffer_add(fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            }

            /* Put a byte on USART to start TX. */
            usart->reg->DR = ((uint32_t)chr) & 0xFF;
        }
    }
    else
    {
        /* Disable USART interrupts. */
        usart_stm32f103_disable_interrupt(usart);

        /* While we have some data to be printed. */
        while (nbytes > 0)
        {
            /* Put a byte on USART. */
            usart->reg->DR = ((uint32_t)*buf) & 0xFF;

            /* Decrement number of bytes remaining. */
            nbytes --;

            /* Move forward in the buffer. */
            buf++;

            /* Wait for transmission of the last byte. */
            while (!(usart->reg->SR & USART_SR_TC))
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
 * @return: Number of bytes read from the USART.
 * This function receives and return data from a serial port.
 */
static int32_t usart_stm32f103_gets(void *fd, void *priv_data, uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_read = nbytes;
    STM32_USART *usart = (STM32_USART *)priv_data;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(flags);

    /* Disable USART interrupts. */
    usart_stm32f103_disable_interrupt(usart);

    /* While we have some data to be printed. */
    while (nbytes > 0)
    {
        /* While we have a byte to read. */
        while (!(usart->reg->SR & USART_SR_RXNE))
        {
            ;
        }

        /* Read a byte from USART. */
        *buf = (uint8_t)usart->reg->DR;

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes read. */
    return (to_read - nbytes);

} /* usart_stm32f103_gets */
#endif /* CONFIG_SERIAL */
