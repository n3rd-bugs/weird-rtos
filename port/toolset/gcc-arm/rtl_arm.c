/*
 * rtl_arm.c
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

/*
 * rtl_arm_init
 * This will initialize RTL for ARM.
 */
void rtl_arm_init(void)
{
#ifndef IO_BUFFERED
    /* Disable buffering on the SDIO files. */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif

} /* rtl_arm_init */

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

/*
 * _sbrk
 * @incr: Number of bytes to move ahead.
 * @return: Return the start of newly allocated region.
 * This will move the heap pointer by the given number of bytes.
 */
void *_sbrk(int incr)
{
   static char  *heap_end = (char *)&_ebss;
   char         *prev_heap_end;

   /* Pick the previous heap end. */
    prev_heap_end = heap_end;

    /* If we may pass the heap boundary. */
    if (((heap_end - (char *)&_ebss) + incr) > TARGET_HEAP_SIZE)
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
