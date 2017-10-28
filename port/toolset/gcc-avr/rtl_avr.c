/*
 * rtl_avr.c
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
#include <io.h>
#include <stdio.h>
#include <errno.h>
#undef errno
extern int errno;

/* Function prototypes. */
static int io_avr_put(char, struct __file *);
static int io_avr_get(struct __file *);

/* Define file for STDIO. */
static FILE stdio;

/*
 * rtl_avr_init
 * This will initialize RTL for AVR.
 */
void rtl_avr_init(void)
{
    /* Initialize SDIO stream. */
    fdev_setup_stream(&stdio, &io_avr_put, &io_avr_get, _FDEV_SETUP_RW);

    /* Initialize SDIO streams. */
    stdin = stdout = stderr = &stdio;

} /* rtl_avr_init */

/*
 * io_avr_put
 * @stream: File stream needed to be written.
 * This will write a character on IO.
 */
static int io_avr_put(char c, struct __file *stream)
{
    /* Remove some compiler warning. */
    UNUSED_PARAM(stream);

    /* Write the given character on IO. */
    return (io_puts(&c, 1));

} /* io_avr_put */

/*
 * io_avr_get
 * @stream: File stream needed to be read.
 * This will read a character from IO.
 */
static int io_avr_get(struct __file *stream)
{
    char c;

    /* Remove some compiler warning. */
    UNUSED_PARAM(stream);

    /* Read a character from IO. */
    io_gets(&c, 1);

    /* Return the read character. */
    return (c);

} /* io_avr_get */

/*
 * _sbrk
 * @incr: Number of bytes to move ahead.
 * @return: Return the start of newly allocated region.
 * This will move the heap pointer by the given number of bytes.
 */
void *_sbrk(int incr)
{
   static char  *heap_end = (char *)&__heap_start;
   char         *prev_heap_end;

   /* Pick the previous heap end. */
    prev_heap_end = heap_end;

    /* If we may pass the heap boundary. */
    if(((heap_end - (char *)&__heap_start) + incr) > TARGET_HEAP_SIZE)
    {
        /* There is no memory available. */
        errno = ENOMEM;

        /* Return error to the caller. */
        return ((void *)-1);
    }

    /* Update  the heap pointer to new location. */
    heap_end += incr;

    /* Return the start of new memory. */
    return (prev_heap_end);

} /* _sbrk */
