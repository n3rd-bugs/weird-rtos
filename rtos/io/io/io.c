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
#ifdef CONFIG_LCD_AN
#include <lcd_an.h>
#endif /* CONFIG_LCD_AN */
#ifdef CONFIG_GFX
#include <gfx.h>
#endif /* CONFIG_GFX */
#ifdef CONFIG_SERIAL
#include <serial.h>
#endif /* CONFIG_SERIAL */

/*
 * io_puts
 * @s: String to be printed on IO.
 * @n: Number of bytes to be printed.
 * @return: Number of bytes printed.
 * This is a RTL function to provide IO hooks for STDIO.
 */
int io_puts(const char *s, int32_t n)
{
#ifdef CONFIG_SERIAL
#ifdef FS_CONSOLE
    SERIAL *serial = (SERIAL *)debug_fd;
#else
    SERIAL *serial = debug_serial;
#endif /* FS_CONSOLE */

#ifdef CONFIG_LCD_AN
    if (lcd_an_fd != NULL)
    {
        /* Print this string on the LCD-AN. */
        fs_puts(lcd_an_fd, (const uint8_t *)s, n);
    }
#endif /* CONFIG_LCD_AN */

#ifdef CONFIG_GFX
    if (gfx_fd != NULL)
    {
        /* Print this string on the graphics driver. */
        fs_puts(gfx_fd, (const uint8_t *)s, n);
    }
#endif /* CONFIG_GFX */

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
#endif /* CONFIG_SERIAL */

#ifndef CONFIG_SERIAL
#ifndef CONFIG_LCD_AN
    /* Remove a compiler warning. */
    UNUSED_PARAM(s);
#endif /* CONFIG_SERIAL */
#endif /* CONFIG_LCD_AN */

    /* Return number of bytes printed. */
    return (n);

} /* io_puts */

/*
 * io_gets
 * @s: Buffer in which data is needed to be received.
 * @n: Number of bytes to be received.
 * @return: Number of bytes read.
 * This is a RTL function to provide IO hooks for STDIO.
 */
int io_gets(char *s, int32_t n)
{
#ifdef CONFIG_SERIAL
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
#endif /* CONFIG_SERIAL */

    /* Return number of bytes read. */
    return (n);

} /* io_gets */
