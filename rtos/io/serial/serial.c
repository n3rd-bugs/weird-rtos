/*
 * serial.c
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
#include <stdio.h>
#include <serial.h>
#include <serial_target.h>
#ifdef CONFIG_NET
#include <net.h>
#endif /* CONFIG_NET */
#ifdef CONFIG_LCD_AN
#include <lcd_an.h>
#endif /* CONFIG_LCD_AN */
#include <string.h>

#ifdef FS_CONSOLE
/* Debug file descriptor. */
FD debug_fd = NULL;
#else
SERIAL *debug_serial = NULL;
#endif

#ifdef FS_CONSOLE
/* Internal function prototypes. */
static int32_t serial_write(void *, const uint8_t *, int32_t);
static int32_t serial_read(void *, uint8_t *, int32_t);
#endif

/*
 * serial_init
 * This will initialize serial sub system.
 */
void serial_init(void)
{
#ifdef SERIAL_TGT_INIT
    /* Initialize serial target. */
    SERIAL_TGT_INIT();
#endif
} /* serial_init */

/*
 * serial_register
 * @serial: Serial port needed to be registered.
 * @name: Name of serial port device.
 * This will register a serial port.
 */
void serial_register(SERIAL *serial, const char *name, void *buffer, uint32_t flags)
{
#ifdef FS_CONSOLE
    FS_BUFFER_DATA *buffer_data = (FS_BUFFER_DATA *)buffer;
    char fs_name[64];
#else

    /* Remove some compiler warnings. */
    UNUSED_PARAM(name);
    UNUSED_PARAM(buffer);
#endif

    /* Initialize serial target device. */
    if (serial->device.init(serial->device.data) == SUCCESS)
    {
        /* Save the serial flags. */
        serial->flags = flags;

#ifdef FS_CONSOLE
        /* Initialize console data. */
        serial->console.fs.name = name;
        serial->console.fs.write = &serial_write;
        serial->console.fs.read = &serial_read;

        /* There is space available to be written. */
        serial->console.fs.flags |= (FS_SPACE_AVAILABLE);

        /* If we want to use interrupt mode. */
        if (flags & SERIAL_INT)
        {
            /* Enable blocking mode. */
            serial->console.fs.flags |= FS_BLOCK;
        }
        else
        {
            /* There is always data available on the port. */
            serial->console.fs.flags |= FS_DATA_AVAILABLE;
        }

        /* Register serial port with console. */
        console_register(&serial->console);
#endif

        /* If this is to be used as debug console. */
        if (flags & SERIAL_DEBUG)
        {
#ifdef FS_CONSOLE
            /* Set debug file descriptor. */
            snprintf(fs_name, 64, "\\console\\%s", name);
            debug_fd = fs_open(fs_name, 0);
#else
            /* Set the debug serial port. */
            debug_serial = serial;
#endif
        }

#ifdef FS_CONSOLE
        /* If this we need to use interrupts to transfer data on this
         * port. */
        if (flags & SERIAL_INT)
        {
            /* User should have provided the buffer data. */
            ASSERT(buffer_data == NULL);

            /* Initialize buffer data. */
            fs_buffer_dataset(serial, buffer_data);

            /* This is a buffered descriptor. */
            serial->console.fs.flags |= FS_BUFFERED;

            /* Assign interrupt data. */
            semaphore_set_interrupt_data(&serial->console.lock, serial->device.data, serial->device.int_lock, serial->device.int_unlock);

            /* Unlock serial interrupts. */
            serial->device.int_unlock(serial->device.data);
        }
#endif
    }

} /* serial_register */

#ifdef FS_CONSOLE
/*
 * serial_write
 * @fs: Serial data for which this was called.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * @return: Number of bytes will be returned if write was successful.
 * This function will send a given buffer on the serial port.
 */
static int32_t serial_write(void *fs, const uint8_t *buf, int32_t nbytes)
{
    SERIAL *serial = (SERIAL *)fs;
    FS_BUFFER *buffer = (FS_BUFFER *)buf;
    int32_t n;

    /* If this is a buffered serial port. */
    if ((serial != NULL) && (serial->flags & SERIAL_INT))
    {
        /* Add a buffer to be transmitted on the transmission list. */
        fs_buffer_add(serial, buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);

        /* Start TX on the serial port. */
        serial->device.puts(fs, serial->device.data, NULL, 0, SERIAL_INT);

        /* Return number of bytes provided. */
        n = nbytes;
    }
    else
    {
        /* Transmit required buffer on the serial. */
        n = serial->device.puts(fs, serial->device.data, (uint8_t *)buf, nbytes, 0);
    }

    /* Return number of bytes transmitted. */
    return (n);

} /* serial_write */

/*
 * serial_read
 * @priv_data: Serial data for which this was called.
 * @buf: Buffer in which data is needed to be read.
 * @nbytes: At most number of bytes to be read.
 * @return: Number of bytes read will be returned here.
 * This function will read data from given serial port.
 */
static int32_t serial_read(void *fs, uint8_t *buf, int32_t nbytes)
{
    SERIAL *serial = (SERIAL *)fs;
    FS_BUFFER *buffer;
    int32_t n = nbytes;

    /* If this is a buffered serial port. */
    if ((serial != NULL) && (serial->flags & SERIAL_INT))
    {
        /* See if we have a buffer to read from. */
        buffer = fs_buffer_get(serial, FS_BUFFER_RX, 0);

        /* If we have a buffer and we are in TX. */
        if (buffer != NULL)
        {
            /* Return a buffer from the RX list. */
            (*((FS_BUFFER **)buf)) = buffer;
        }
    }
    else
    {
        /* Read some data in the given buffer. */
        n = serial->device.gets(fs, serial->device.data, buf, nbytes, 0);
    }

    /* Return number of bytes read from the serial port. */
    return (n);

} /* serial_read */
#endif

/*
 * serial_assert_puts
 * @buffer: Buffer to be printed on the debug serial.
 * @nbytes: Number of bytes to be printed.
 * This function prints a given buffer on the debug serial, this is usually
 * called from the context of the assert.
 */
void serial_assert_puts(uint8_t *buffer, int32_t nbytes)
{
#ifdef FS_CONSOLE
    SERIAL *serial = (SERIAL *)debug_fd;
#else
    SERIAL *serial = debug_serial;
#endif

    /* Assert if debug serial is not yet initialized. */
    ASSERT(serial == NULL);

    /* Transmit required buffer on the serial. */
    serial->device.puts(serial, serial->device.data, buffer, ((nbytes == 0) ? (int32_t)strlen((char *)buffer) : nbytes), 0);

} /* serial_assert_puts */
#endif /* FS_CONSOLE */
