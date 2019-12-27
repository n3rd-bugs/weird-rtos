/*
 * usart_stm32f030.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
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
#include <usart_stm32f030.h>
#ifdef SERIAL_INTERRUPT_MODE
#ifndef FS_CONSOLE
#error "Console is required for interrupt mode serial."
#endif /* FS_CONSOLE */
#endif /* SERIAL_INTERRUPT_MODE */

/* Global port data. */
static STM32_USART *usart1_data;

/* Internal function prototypes. */
static int32_t usart_stm32f030_init(void *);
static void usart_handle_tx_interrupt(STM32_USART *);
static void usart_handle_rx_interrupt(STM32_USART *);
static void usart_stm32f030_enable_interrupt(void *);
static void usart_stm32f030_disable_interrupt(void *);
static int32_t usart_stm32f030_puts(void *, void *, const uint8_t *, int32_t, uint32_t);
static int32_t usart_stm32f030_gets(void *, void *, uint8_t *, int32_t, uint32_t);

/* USART1 data. */
static STM32_USART usart1;
#ifdef SERIAL_INTERRUPT_MODE
static FS_BUFFER_DATA usart1_buffer_data;
static uint8_t usart1_buffer_space[SERIAL_MAX_BUFFER_SIZE * SERIAL_NUM_BUFFERS];
static FS_BUFFER usart1_buffer_ones[SERIAL_NUM_BUFFERS];
static FS_BUFFER_LIST usart1_buffer_lists[SERIAL_NUM_BUFFER_LIST];
#endif /* SERIAL_INTERRUPT_MODE */

/*
 * serial_stm32f030_init
 * This will initialize serial interface(s) for this target.
 */
void serial_stm32f030_init(void)
{
#ifdef SERIAL_INTERRUPT_MODE
    /* Register this serial device. */
    usart1_buffer_data.buffer_space = usart1_buffer_space;
    usart1_buffer_data.buffer_size = SERIAL_MAX_BUFFER_SIZE;
    usart1_buffer_data.buffers = usart1_buffer_ones;
    usart1_buffer_data.num_buffers = SERIAL_NUM_BUFFERS;
    usart1_buffer_data.buffer_lists = usart1_buffer_lists;
    usart1_buffer_data.num_buffer_lists = SERIAL_NUM_BUFFER_LIST;
    usart1_buffer_data.threshold_buffers = SERIAL_THRESHOLD_BUFFER;
    usart1_buffer_data.threshold_lists = SERIAL_THRESHOLD_BUFFER_LIST;
    usart_stm32f030_register(&usart1, "usart1", 1, SERIAL_BAUD_RATE, &usart1_buffer_data, FALSE, TRUE);
#else
    usart_stm32f030_register(&usart1, "usart1", 1, SERIAL_BAUD_RATE, NULL, FALSE, TRUE);
#endif /* SERIAL_INTERRUPT_MODE */

} /* serial_stm32f030_init */

/*
 * usart_stm32f030_register
 * @usart: STM32 USART instance t be registered.
 * @name: Name of this USART instance.
 * @device_num: USART device number we need to register.
 * @boud_rate: USART baud rate.
 * @buffer_data: Buffer data for this USART, if not null USART interrupt mode
 *  will be enabled.
 * @hw_flow: If we need to enable hardware flow control for this USART.
 * @is_debug: If this USART is needed to be used as debug console.
 * @return: Success will be returned if USART was successfully registered,
 *  SERIAL_NOT_FOUND will be returned if requested serial device was not found.
 * This function will register a USART for STM32 platform.
 */
int32_t usart_stm32f030_register(STM32_USART *usart, const char *name, uint8_t device_num, uint32_t baud_rate, FS_BUFFER_DATA *buffer_data, uint8_t hw_flow, uint8_t is_debug)
{
    int32_t status = SUCCESS;
    uint32_t usart_flags = ((is_debug == TRUE) ? SERIAL_DEBUG : 0);

    /* Remove compiler warnings. */
    UNUSED_PARAM(hw_flow);

    /* Save the USART data. */
    usart->device_num = device_num;
    usart->baud_rate = baud_rate;

    /* Initialize USART GPIO. */
    switch (usart->device_num)
    {
    /* If this is USART1 device. */
    case 1:

        /* Reset USART1. */
        RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
        RCC->APB2RSTR &= (uint32_t)~RCC_APB2RSTR_USART1RST;

        /* Enable clock for USART1. */
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

        /* Enable clock for GPIOA. */
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

        /* Set alternate function for PA2 (TX) and PA3 (RX). */
        GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
        GPIOA->MODER |= (GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1);

        /* Select output mode for PA2 and input mode for PA3. */
        GPIOA->OTYPER &= (uint16_t)~(GPIO_OTYPER_OT_2);
        GPIOA->OTYPER |= GPIO_OTYPER_OT_3;

        /* Select high speed for PA2 */
        GPIOA->OSPEEDR &= (uint32_t)~(GPIO_OSPEEDER_OSPEEDR2);
        GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2_1;

        /* Select USART1 AF for PA9 and PA10. */
        GPIOA->AFR[0] &= (uint32_t)~((0xF << ((2 % 8) << 2)) | (0xF << ((3 % 8) << 2)));
        GPIOA->AFR[0] |= (0x1 << ((2 % 8) << 2)) | (0x1 << ((3 % 8) << 2));

        /* Save the USART register. */
        usart->reg = USART1;

        /* Save the USART1 data. */
        usart1_data = usart;

        break;

    default:

        /* Requested serial device as not found. */
        status = SERIAL_NOT_FOUND;

        break;
    }

    /* Initialize serial device data. */
    usart->serial.device.init = &usart_stm32f030_init;
    usart->serial.device.puts = &usart_stm32f030_puts;
    usart->serial.device.gets = &usart_stm32f030_gets;

    /* If we need to use interrupts for this USART. */
    if (buffer_data)
    {
        /* Hook-up interrupt locks. */
        usart->serial.device.int_lock = &usart_stm32f030_disable_interrupt;
        usart->serial.device.int_unlock = &usart_stm32f030_enable_interrupt;

        /* Update USART flags. */
        usart_flags |= SERIAL_INT;
    }

    /* Save the device data. */
    usart->serial.device.data = usart;

    /* Register this USARt as a serial device. */
    serial_register(&usart->serial, name, buffer_data, usart_flags);

    /* Return status to the caller. */
    return (status);

} /* usart_stm32f030_register */

/*
 * usart_stm32f030_init
 * @data: USART device data.
 * @return: Will always return success.
 * This function initializes USART1 for stm32f030.
 */
static int32_t usart_stm32f030_init(void *data)
{
    STM32_USART *usart = (STM32_USART *)data;
    uint32_t temp, integral, fractional;

    /* Enable RX and TX for this USART, also use 8-bit word length and
     * disable parity bit. */
    usart->reg->CR1 &= (uint16_t)~((USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE));
    usart->reg->CR1 |= (USART_CR1_RE | USART_CR1_TE);

    /* Use one stop bit for this USART. */
    usart->reg->CR2 &= (uint16_t)~(USART_CR2_STOP);

    /* If we need to use hardware flow control. */
    if (usart->flags & STM32_USART_HW_FCTRL)
    {
        /* Use CTS/RTS signals. */
        usart->reg->CR3 |= ((USART_CR3_RTSE | USART_CR3_CTSE));
    }

    /* Calculate and set the baud rate parameters. */
    integral = ((25 * PCLK_FREQ) / (4 * usart->baud_rate));
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

} /* usart_stm32f030_init */

/*
 * usart1_interrupt
 * This function is interrupt handler for USART1 interrupt.
 */
ISR_FUN usart1_interrupt(void)
{
    ISR_ENTER();

    /* If a transmission was successfully completed. */
    if (USART1->ISR & USART_ISR_TC)
    {
        /* Handle transmission complete interrupt. */
        usart_handle_tx_interrupt(usart1_data);
    }

    /* If some data is available to read. */
    else if ((USART1->ISR & USART_ISR_RXNE) || (USART1->ISR & USART_ISR_IDLE))
    {
        /* Handle receive data available interrupt. */
        usart_handle_rx_interrupt(usart1_data);
    }

    ISR_EXIT();

} /* usart1_interrupt */

/*
 * usart_handle_tx_interrupt
 * @usart: Device data on which we need to process TX interrupt.
 * This function handles TX interrupt.
 */
static void usart_handle_tx_interrupt(STM32_USART *usart)
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

        /* Put a byte on USART to continue TX. */
        usart->reg->TDR = ((uint32_t)chr) & 0xFF;
    }
    else
    {
        /* Just clear this interrupt status. */
        usart->reg->ICR = USART_ICR_TCCF;

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
    FS_BUFFER_LIST *buffer;
    uint8_t chr;

    /* If there is some data available to read. */
    if (usart->reg->ISR & USART_ISR_RXNE)
    {
        /* Read the incoming data and also clear the interrupt status. */
        chr = (uint8_t)usart->reg->RDR;

        /* Get a RX buffer. */
        buffer = fs_buffer_get(usart, FS_BUFFER_RX, FS_BUFFER_INPLACE);

        /* If we don't have a buffer. */
        if (buffer == NULL)
        {
            /* Get a buffer. */
            buffer = fs_buffer_get(usart, FS_LIST_FREE, FS_BUFFER_TH);

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
        }
    }

    /* If line is idle. */
    if (usart->reg->ISR & USART_ISR_IDLE)
    {
        /* Tell upper layers that some data is available to read. */
        fd_data_available(usart);

        /* Disable idle interrupts. */
        usart->reg->CR1 &= (uint16_t)~(USART_CR1_IDLEIE);
    }
    else
    {
        /* Enable idle interrupts. */
        usart->reg->CR1 |= (USART_CR1_IDLEIE);
    }

} /* usart_handle_rx_interrupt */

/*
 * usart_stm32f030_enable_interrupt.
 * @data: USART for which interrupts are needed to be enabled.
 * This function will enable interrupts for the given USART.
 */
static void usart_stm32f030_enable_interrupt(void *data)
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
    }

} /* usart_stm32f030_enable_interrupt */

/*
 * usart_stm32f030_disable_interrupt.
 * @data: USART for which interrupts are needed to be disabled.
 * This function will disable interrupts for the given USART.
 */
static void usart_stm32f030_disable_interrupt(void *data)
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
    }

} /* usart_stm32f030_disable_interrupt */

/*
 * usart_stm32f030_puts
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data needed to be sent over USART.
 * @nbytes: Number of bytes to be printed from the buffer.
 * @flags: Flags to specify the operation.
 * @return: Number of bytes printed on the USART.
 * This function sends a buffer on the given USART.
 */
static int32_t usart_stm32f030_puts(void *fd, void *priv_data, const uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_print = nbytes;
    STM32_USART *usart = (STM32_USART *)priv_data;
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

            /* Put a byte on USART to start TX. */
            usart->reg->TDR = ((uint32_t)chr) & 0xFF;
        }
    }
    else
    {
        /* Disable USART interrupts. */
        usart_stm32f030_disable_interrupt(usart);

        /* While we have some data to be printed. */
        while (nbytes > 0)
        {
            /* Put a byte on USART. */
            usart->reg->TDR = ((uint32_t)*buf) & 0xFF;

            /* Decrement number of bytes remaining. */
            nbytes --;

            /* Move forward in the buffer. */
            buf++;

            /* Wait for transmission of the last byte. */
            while (!(usart->reg->ISR & USART_ISR_TC))
            {
                ;
            }
        }
    }

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* usart_stm32f030_puts */

/*
 * usart_stm32f030_gets
 * @fd: Serial file descriptor.
 * @priv_data: USART data.
 * @buf: Data received will be returned in this buffer.
 * @nbytes: Number of bytes received on serial port.
 * @flags: For now unused.
 * @return: Number of bytes read from the USART.
 * This function receives and return data from a serial port.
 */
static int32_t usart_stm32f030_gets(void *fd, void *priv_data, uint8_t *buf, int32_t nbytes, uint32_t flags)
{
    int32_t to_read = nbytes;
    STM32_USART *usart = (STM32_USART *)priv_data;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(flags);

    /* Disable USART interrupts. */
    usart_stm32f030_disable_interrupt(usart);

    /* While we have some data to be printed. */
    while (nbytes > 0)
    {
        /* While we have a byte to read. */
        while (!(usart->reg->ISR & USART_ISR_RXNE))
        {
            ;
        }

        /* Read a byte from USART. */
        *buf = (uint8_t)usart->reg->RDR;

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes read. */
    return (to_read - nbytes);

} /* usart_stm32f030_gets */
#endif /* CONFIG_SERIAL */
