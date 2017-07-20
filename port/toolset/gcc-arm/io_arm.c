/*
 * io_arm.c
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
#include <errno.h>
#undef errno
extern int errno;

/*
 * io_arm_init
 * This will initialize IO routines for ARM.
 */
void io_arm_init()
{
#ifndef IO_BUFFERED
    /* Disable buffering on the SDIO files. */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif

} /* io_arm_init */

/*
 * _write
 * @file: File on which we need to write data.
 * @ptr: Data needed to be written.
 * @len: Length of data needed to be written.
 * @return: Number of bytes written will be returned,
 *  otherwise an error will be returned.
 * This is an RTL API that diverts a write to an IO.
 */
int _write(int file, char *ptr, int len)
{
    int status = SUCCESS;

    /* If this is a SDIO file. */
    if ((file == fileno(stdout)) || (file == fileno(stdin)) || (file == fileno(stderr)))
    {
        /* Write on the IO. */
        status = io_puts(ptr, len);
    }
    else
    {
        /* Return error to the caller. */
        errno = EIO;
        status = -1;
    }

    /* Return status to the caller. */
    return (status);

} /* _write */

/*
 * _read
 * @file: File from which we need to read data.
 * @ptr: Buffer in which data will be read.
 * @len: Size of provided buffer.
 * @return: Number of bytes read will be returned,
 *  otherwise an error will be returned.
 * This is an RTL API that diverts a read to an IO.
 */
int _read(int file, char *ptr, int len)
{
    int status = SUCCESS;

    /* If this is a SDIO file. */
    if ((file == fileno(stdout)) || (file == fileno(stdin)) || (file == fileno(stderr)))
    {
        /* Read from IO.. */
        status = io_gets(ptr, len);
    }
    else
    {
        /* Return error to the caller. */
        errno = EIO;
        status = -1;
    }

    /* Return status to the caller. */
    return (status);

} /* _read */
