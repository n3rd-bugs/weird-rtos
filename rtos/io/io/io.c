/*
 * io.c
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
#include <io.h>
#ifdef IO_LCD_AN
#include <lcd_an.h>
#endif /* IO_LCD_AN */
#ifdef IO_GFX
#include <gfx.h>
#endif /* IO_GFX */
#ifdef IO_SERIAL
#include <serial.h>
#endif /* IO_SERIAL */

/*
 * io_puts
 * @ptr: String to be printed on IO.
 * @n: Number of bytes to be printed.
 * @return: Number of bytes printed.
 * This is a RTL function to provide IO hooks for STDIO.
 */
int io_puts(const void *ptr, int32_t n)
{
    const char *s = (const char *)ptr;
#ifdef IO_SERIAL
#ifdef FS_CONSOLE
    SERIAL *serial = (SERIAL *)debug_fd;
#else
    SERIAL *serial = debug_serial;
#endif /* FS_CONSOLE */
#endif /* IO_SERIAL */

#ifdef IO_LCD_AN
    if (lcd_an_fd != NULL)
    {
        /* Print this string on the Alphanumeric LCD. */
        fs_puts(lcd_an_fd, (const uint8_t *)s, n);
    }
#endif /* IO_LCD_AN */

#ifdef IO_GFX
    if (gfx_fd != NULL)
    {
        /* Print this string on the graphics driver. */
        fs_puts(gfx_fd, (const uint8_t *)s, n);
    }
#endif /* IO_GFX */

#ifdef IO_SERIAL
    /* If we do have a serial port. */
    if (serial != NULL)
    {
#ifdef FS_CONSOLE
        /* Write given string on the serial port. */
        n = fs_puts(serial, (const uint8_t *)s, n);
#else
        /* Print the result on the serial. */
        n = debug_serial->device.puts(serial, serial->device.data, (const uint8_t *)s, n, 0);
#endif /* FS_CONSOLE */
    }
#endif /* IO_SERIAL */

#ifndef IO_GFX
#ifndef IO_SERIAL
#ifndef IO_LCD_AN
    /* Remove a compiler warning. */
    UNUSED_PARAM(s);
#endif /* IO_SERIAL */
#endif /* IO_LCD_AN */
#endif /* IO_GFX */

    /* Return number of bytes printed. */
    return (n);

} /* io_puts */

/*
 * io_gets
 * @ptr: Buffer in which data is needed to be received.
 * @n: Number of bytes to be received.
 * @return: Number of bytes read.
 * This is a RTL function to provide IO hooks for STDIO.
 */
int io_gets(void *ptr, int32_t n)
{
    char *s = (char*)ptr;
#ifdef IO_SERIAL
#ifdef FS_CONSOLE
    SERIAL *serial = (SERIAL *)debug_fd;
#else
    SERIAL *serial = debug_serial;
#endif /* FS_CONSOLE */

    /* If we do have a serial port. */
    if (serial != NULL)
    {
        /* Read data from the serial port. */
#ifdef FS_CONSOLE
        n = fs_gets(serial, (uint8_t *)s, n);
#else
        n = debug_serial->device.gets(serial, serial->device.data, (uint8_t *)s, n, 0);
#endif /* FS_CONSOLE */
    }
#else
    /* Remove a compiler warning. */
    UNUSED_PARAM(s);
#endif /* IO_SERIAL */

    /* Return number of bytes read. */
    return (n);

} /* io_gets */
