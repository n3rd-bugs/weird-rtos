/*
 * usart_stm32f407.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <stdio.h>
#include <os.h>
#include <fs.h>
#include <console.h>

#ifdef FS_CONSOLE

/* Console data. */
static CONSOLE usart_1 =
{
    .fs =
    {
        /* Name of this port. */
        .name = "usart1",

        /* Console manipulation APIs. */
        .write = &usart_stm32f407_puts,

        /* Space is available on this descriptor. */
        .flags = FS_SPACE_AVAILABLE,
    }
};
#endif

/*
 * usart_stm32f407_puts
 * @priv_data: For now it is unused.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * This function prints a string on the UART1.
 */
int32_t usart_stm32f407_puts(void *priv_data, uint8_t *buf, int32_t nbytes)
{
    int32_t to_print = nbytes;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);

    /* While we have some data to be printed. */
    while (nbytes > 0)
    {
        /* If needed wait for transmission of the last byte. */
        while (!(USART1->SR & USART_SR_TC))
        {
            ;
        }

        /* Put a byte on USART. */
        USART1->DR = ((uint32_t)*buf) & 0xFF;

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* usart_stm32f407_puts */

/*
 * usart_stm32f407_printf
 * @format: Formated string to be printed on UART.
 * This function prints a formated string on the UART1.
 */
int32_t usart_stm32f407_printf(char *format, ...)
{
    int32_t n = 0;
    uint8_t buf[PRINTF_BUFFER_SIZE];
    va_list vl;

    /* Arguments start from the format. */
    va_start(vl, format);

    /* Process the given string and save the result in a temporary buffer. */
    n = vsnprintf((char *)buf, PRINTF_BUFFER_SIZE, format, vl);

#ifdef FS_CONSOLE
    /* Assert if debug FD is not yet initialized. */
    OS_ASSERT(debug_fd == NULL);

    /* Use the debug FD. */
    n = fs_write(debug_fd, buf, n);
#else
    /* Print the result on the UART. */
    n = usart_stm32f407_puts(NULL, buf, n);
#endif

    /* Destroy the argument list. */
    va_end(vl);

    /* Return number of bytes printed on UART. */
    return (n);

} /* usart_stm32f407_printf */

/*
 * usart_stm32f407_vprintf
 * @format: Formated string to be printed on USART.
 * This function prints a formated log message on the console.
 */
int32_t usart_stm32f407_vprintf(const char *format, va_list vl)
{
    int32_t n;
    uint8_t buf[PRINTF_BUFFER_SIZE];

    /* Process the given string and save the result in a temporary buffer. */
    n = vsnprintf((char *)buf, PRINTF_BUFFER_SIZE, format, vl);

    if (n > 0)
    {
#ifdef FS_CONSOLE
        /* Assert if debug FD is not yet initialized. */
        OS_ASSERT(debug_fd == NULL);

        /* Use the debug FD. */
        n = fs_write(debug_fd, buf, n);
#else
        /* Print the result on the UART. */
        n = usart_stm32f407_puts(NULL, buf, n);
#endif
    }

    /* Return number of bytes printed on UART. */
    return (n);

} /* usart_stm32f407_vprintf */

/*
 * usart_stm32f407_init
 * This function initializes UART1 for STM32F407.
 */
void usart_stm32f407_init()
{
    uint32_t temp, integral, fractional;

    /* Enable clock for USART1. */
    RCC->APB2ENR |= 0x10;

    /* Enable clock for GPIOB. */
    RCC->AHB1ENR |= 0x02;

    /* Set alternate function for the PB6 (TX) and PB7 (RX). */
    GPIOB->MODER &= ~((GPIO_MODER_MODER0 << (5 * 2)) | (GPIO_MODER_MODER0 << (6 * 2)));
    GPIOB->MODER |= ((0x2 << (5 * 2)) | (0x2 << (6 * 2)));

    /* Select 50MHz IO speed. */
    GPIOB->OSPEEDR &= ~((GPIO_OSPEEDER_OSPEEDR0 << (5 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (6 * 2)));
    GPIOB->OSPEEDR |= ((0x2 << (5 * 2)) | (0x2 << (6 * 2)));

    /* Output mode configuration. */
    GPIOB->OTYPER  &= ~((GPIO_OTYPER_OT_0 << (5 * 2)) | (GPIO_OTYPER_OT_0 << (6 * 2)));

    /* Enable pull-ups on USART pins. */
    GPIOB->PUPDR &= ~((GPIO_PUPDR_PUPDR0 << (5 * 2)) | (GPIO_PUPDR_PUPDR0 << (6 * 2)));
    GPIOB->PUPDR |= ((0x01 << (5 * 2)) | (0x01 << (6 * 2)));

    /* Select USART1 as Alternate function for these pins. */
    GPIOB->AFR[(0x6 >> 0x03)] &= ~((uint32_t)(0xF << (0x6 * 4))) ;
    GPIOB->AFR[(0x7 >> 0x03)] &= ~((uint32_t)(0xF << (0x7 * 4))) ;
    GPIOB->AFR[(0x6 >> 0x03)] |= (uint32_t)(0x7 << (0x6 * 4));
    GPIOB->AFR[(0x7 >> 0x03)] |= (uint32_t)(0x7 << (0x7 * 4));

    /* Enable RX and TX for this USART, also use 8-bit word length and
     * disable parity bit. */
    USART1->CR1 &= ~((USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE));
    USART1->CR1 |= (0xC);

    /* Use one stop bit for this USART. */
    USART1->CR2 &= ~(USART_CR2_STOP);

    /* Don't use CTS/RTS signals. */
    USART1->CR3 &= ~((USART_CR3_RTSE | USART_CR3_CTSE));

    /* Calculate and set the baud rate parameters. */
    integral = ((25 * PCLK_FREQ) / (4 * BAUD_RATE));
    temp = (integral / 100) << 4;
    fractional = integral - (100 * (temp >> 4));
    temp |= ((((fractional * 16) + 50) / 100)) & (0x0F);
    USART1->BRR = temp;

    /* Enable USART1. */
    USART1->CR1 |= USART_CR1_UE;

#ifdef FS_CONSOLE

    /* Register serial port with console. */
    console_register(&usart_1);

    /* Set debug file descriptor. */
    debug_fd = fs_open("\\console\\usart1", 0);

#endif /* FS_CONSOLE */

} /* usart_stm32f407_init */
