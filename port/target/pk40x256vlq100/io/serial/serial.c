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
    while(nbytes > 0)
    {
        /* Wait for UART to print the existing data. */
        while(!(UART_S1_REG(UART1_BASE_PTR) & UART_S1_TDRE_MASK))
        {
            ;
        }

        /* Put a character on the UART's output buffer. */
        UART_D_REG(UART1_BASE_PTR) = (uint8_t)*buf;

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
    uint16_t ubd, brfa;
    uint8_t temp;

    /* Enable clock for UART1 module. */
    SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;

    UART_C2_REG(UART1_BASE_PTR) &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );
    UART_C1_REG(UART1_BASE_PTR) = 0;

    /* Calculate baudrate configuration. */
    ubd = (uint16_t)((PCLK_FREQ)/(BAUD_RATE * 16));
    temp = UART_BDH_REG(UART1_BASE_PTR) & ~(UART_BDH_SBR(0x1F));
    UART_BDH_REG(UART1_BASE_PTR) = temp | UART_BDH_SBR(((ubd & 0x1F00) >> 8));
    UART_BDL_REG(UART1_BASE_PTR) = (uint8_t)(ubd & UART_BDL_SBR_MASK);
    brfa = (((PCLK_FREQ)/(BAUD_RATE * 16)) - (ubd * 32));
    temp = UART_C4_REG(UART1_BASE_PTR) & ~(UART_C4_BRFA(0x1F));
    UART_C4_REG(UART1_BASE_PTR) = temp | UART_C4_BRFA(brfa);
    UART_C2_REG(UART1_BASE_PTR) |= (UART_C2_TE_MASK | UART_C2_RE_MASK );

#ifdef FS_CONSOLE
#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect this console data. */
    memset(&uart_1.lock, 0, sizeof(SEMAPHORE));
    semaphore_create(&uart_1.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

    /* Register this serial port with console. */
    console_register(&uart_1.console);

    /* Set debug file descriptor. */
    debug_fd = fs_open("\\console\\uart1", 0);

#endif /* FS_CONSOLE */

} /* serial_init */
