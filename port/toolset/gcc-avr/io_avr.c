/*
 * io_avr.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>
#include <io.h>
#include <stdio.h>

/* Function prototypes. */
static int io_avr_put(char, struct __file *);
static int io_avr_get(struct __file *);

/* Define file for STDIO. */
static FILE stdio;

/*
 * io_avr_init
 * This will initialize IO routines for AVR.
 */
void io_avr_init()
{
    /* Initialize SDIO stream. */
    fdev_setup_stream(&stdio, &io_avr_put, &io_avr_get, _FDEV_SETUP_RW);

    /* Initialize SDIO streams. */
    stdin = stdout = stderr = &stdio;

} /* io_avr_init */

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
