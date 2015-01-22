/*
 * serial.c
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

#include <os.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <serial.h>

#ifdef FS_CONSOLE
/* Debug file descriptor. */
FD debug_fd;

/* UART console data. */
UART_CON uart_1 =
{
    .console.fs =
    {
        /* Name of this port. */
        .name = "uart1",

        /* Console manipulation APIs. */
        .write = &serial_puts,
    }
};
#endif

/*
 * serial_puts
 * @priv_data: For now it is unused.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * This function prints a string on the UART1.
 */
uint32_t serial_puts(void *priv_data, char *buf, uint32_t nbytes)
{
    uint32_t to_print = nbytes;

#ifdef FS_CONSOLE
#ifndef CONFIG_SEMAPHORE
    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);
#else
    /* For now just emulate this. */
    if (priv_data != NULL)
    {
        /* Obtain the UART lock. */
        semaphore_obtain(&((UART_CON *)priv_data)->lock, MAX_WAIT);
    }
#endif
#endif

    /* While we have some data to be printed. */
    while (nbytes > 0)
    {
        /* If needed wait for transmission of the last byte. */
        while (!(USART1->SR & USART_SR_TC))
        {
            ;
        }

        /* Put a byte on USART. */
        USART1->DR = *buf;

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

#ifdef FS_CONSOLE
#ifdef CONFIG_SEMAPHORE
    if (priv_data != NULL)
    {
        /* Release the UART lock. */
        semaphore_release(&((UART_CON *)priv_data)->lock);
    }
#endif
#endif

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* serial_puts */

/*
 * serial_printf
 * @format: Formated string to be printed on UART.
 * This function prints a formated string on the UART1.
 */
uint32_t serial_printf(char *format, ...)
{
    uint32_t n = 0;
    char buf[100];
    va_list vl;

    /* Arguments start from the format. */
    va_start(vl, format);

    /* Process the given string and save the result in a temporary buffer. */
    n = vsnprintf(buf, 100, format, vl);

#ifdef FS_CONSOLE
    /* Use the debug FD. */
    n = fs_write(debug_fd, buf, n);
#else
    /* Print the result on the UART. */
    n = serial_puts(NULL, buf, n);
#endif

    /* Destroy the argument list. */
    va_end(vl);

    /* Return number of bytes printed on UART. */
    return (n);

} /* serial_printf */

/*
 * serial_init
 * This function initializes UART1 so that user can use serial_printf for
 * printing data on it.
 */
void serial_init()
{
    uint32_t temp, integral, fractional;

    /* Enable APB peripheral clock. */
    RCC->APB2ENR |= 0x10;

    /* Enable APB peripheral clock. */
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

    /* Select USART1 as Alternate function for these devices. */
    GPIOB->AFR[(0x6 >> 0x03)] &= ~(0xF << (0x6 * 4)) ;
    GPIOB->AFR[(0x7 >> 0x03)] &= ~(0xF << (0x07 * 4)) ;
    GPIOB->AFR[(0x6 >> 0x03)] |= (0x7 << (0x6 * 4));
    GPIOB->AFR[(0x7 >> 0x03)] |= (0x7 << (0x7 * 4));

    /* Enable RX and TX for this USART, also use 8-bit word length and
     * disable parity bit. */
    USART1->CR1 &= ~((USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE));
    USART1->CR1 |= (0xC);

    /* Use one stop bit for this USART. */
    USART1->CR2 &= ~(USART_CR2_STOP);

    /* Don't use CTS/RTS signals. */
    USART1->CR3 &= ~((USART_CR3_RTSE | USART_CR3_CTSE));

    /* Calculate and set the baud rate parameters. */
    integral = ((25 * PCLK_FREQ) / (4 * 115200));
    temp = (integral / 100) << 4;
    fractional = integral - (100 * (temp >> 4));
    temp |= ((((fractional * 16) + 50) / 100)) & (0x0F);
    USART1->BRR = temp;

    /* Enable USART. */
    USART1->CR1 |= USART_CR1_UE;

#ifdef FS_CONSOLE
#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect this console data. */
    memset(&uart_1.lock, 0, sizeof(SEMAPHORE));
    semaphore_create(&uart_1.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

    /* Initialize console data. */
    uart_1.console.fs.name = "uart1";
    uart_1.console.fs.write = &serial_puts;

    /* Register this serial port with console. */
    console_register(&uart_1.console);

    /* Set debug file descriptor. */
    debug_fd = fs_open("\\console\\uart1", 0);

#endif /* FS_CONSOLE */

} /* serial_init */
