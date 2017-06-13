/*
 * uart_pk40x256vlq100.c
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

#include <stdio.h>
#include <os.h>
#include <string.h>
#include <stdarg.h>

#ifdef FS_CONSOLE
/* Console data. */
static CONSOLE uart_1 =
{
    .fs =
    {
        /* Name of this port. */
        .name = "uart1",

        /* Console manipulation APIs. */
        .write = &uart_pk40x256vlq100_puts,
    }
};
#endif

/*
 * uart_pk40x256vlq100_puts
 * @priv_data: For now it is unused.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * This function prints a string on the UART1.
 */
uint32_t uart_pk40x256vlq100_puts(void *priv_data, uint8_t *buf, uint32_t nbytes)
{
    uint32_t to_print = nbytes;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);

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

} /* uart_pk40x256vlq100_puts */

/*
 * uart_pk40x256vlq100_printf
 * @format: Formated string to be printed on UART.
 * This function prints a formated string on the UART1.
 */
uint32_t uart_pk40x256vlq100_printf(char *format, ...)
{
    uint32_t n = 0;
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
    n = uart_pk40x256vlq100_puts(NULL, buf, n);
#endif

    /* Destroy the argument list. */
    va_end(vl);

    /* Return number of bytes printed on UART. */
    return (n);

} /* uart_pk40x256vlq100_printf */

/*
 * uart_pk40x256vlq100_vprintf
 * @format: Formated string to be printed on USART.
 * This function prints a formated log message on the console.
 */
int32_t uart_pk40x256vlq100_vprintf(const char *format, va_list vl)
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
        n = uart_pk40x256vlq100_puts(NULL, buf, n);
#endif
    }

    /* Return number of bytes printed on UART. */
    return (n);

} /* uart_pk40x256vlq100_vprintf */

/*
 * uart_pk40x256vlq100_init
 * This function initializes UART1 so that user can use serial_printf for
 * printing data on it.
 */
void uart_pk40x256vlq100_init()
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

    /* Enable clock for PORTE. */
    SIM_SCGC5 = SIM_SCGC5_PORTE_MASK;

    /* Set MUX for TX and RX pins. */
    PORTE_PCR0 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
    PORTE_PCR1 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;

#ifdef FS_CONSOLE
    /* Register this serial port with console. */
    console_register(&uart_1);

    /* Set debug file descriptor. */
    debug_fd = fs_open("\\console\\uart1", 0);
#endif /* FS_CONSOLE */

} /* uart_pk40x256vlq100_init */
