/*
 * io.h
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
#ifndef _IO_H_
#define _IO_H_
#include <kernel.h>

/* Function prototypes. */
int io_puts(const char *s, int32_t n);
int io_gets(char *s, int32_t n);

#endif /* _IO_H_ */
