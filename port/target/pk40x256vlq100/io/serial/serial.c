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

#include <MK40DZ10.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <os_target.h>
#include <serial.h>

/*
 * serial_puts
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * This function prints a string on the UART1.
 */
int serial_puts(const char *buf, int nbytes)
{
    int to_print = nbytes;

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

    /* Return number of bytes printed. */
    return (to_print - nbytes);

} /* serial_puts */

/*
 * serial_printf
 * @format: Formated string to be printed on UART.
 * This function prints a formated string on the UART1.
 */
int serial_printf(char *format, ...)
{
    int n = 0;
    char buf[100];
    va_list vl;

    /* Arguments start from the format. */
    va_start(vl, format);

    /* Process the given string and save the result in a temporary buffer. */
    n = vsnprintf(buf, 100, format, vl);

    /* Print the result on the UART. */
    n = serial_puts(buf, n);

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

} /* serial_init */
