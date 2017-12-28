/*
 * serial.h
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
#ifndef _SERIAL_H_
#define _SERIAL_H_
#include <kernel.h>

#ifdef CONFIG_SERIAL
#ifdef CONFIG_FS
#include <fs.h>
#ifdef FS_CONSOLE
#include <console.h>
#endif /* FS_CONSOLE */
#endif  /* CONFIG_FS */
#include <stdarg.h>

/* Serial device flag definitions. */
#define SERIAL_DEBUG                0x01
#define SERIAL_INT                  0x02
#define SERIAL_IN_TX                0x04

typedef int32_t (SERIAL_INIT) (void *);
typedef int32_t (SERIAL_PUTS) (void *, void *, const uint8_t *, int32_t, uint32_t);
typedef int32_t (SERIAL_GETS) (void *, void *, uint8_t *, int32_t, uint32_t);

/* Serial target device structure. */
typedef struct _serial_device
{
    /* Target APIs. */
    SERIAL_INIT     *init;
    SERIAL_PUTS     *puts;
    SERIAL_GETS     *gets;
#ifdef FS_CONSOLE
    SEM_INT_LOCK    *int_lock;
    SEM_INT_UNLOCK  *int_unlock;
#endif

    /* Device specific data. */
    void    *data;

} SERIAL_DEVICE;

/* Serial OS device structure. */
typedef struct _serial
{
#ifdef FS_CONSOLE
    /* Associated console device. */
    CONSOLE         console;
#endif

    /* Associated serial device. */
    SERIAL_DEVICE   device;

#ifdef CONFIG_FS
    /* Transmission buffers. */
    FS_BUFFER_LIST  tx_buffer;
    FS_BUFFER_LIST  rx_buffer;
#endif

    /* Serial flags. */
    uint32_t        flags;

} SERIAL;

/* Debug file descriptor. */
#ifdef FS_CONSOLE
extern FD debug_fd;
#else
extern SERIAL *debug_serial;
#endif /* FS_CONSOLE */

/* Function prototypes. */
void serial_init(void);
void serial_register(SERIAL *, const char *, void *, uint32_t);
void serial_assert_puts(uint8_t *, int32_t);

#endif /* CONFIG_SERIAL */
#endif /* _SERIAL_H_ */
