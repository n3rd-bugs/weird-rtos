/*
 * io.h
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
#ifndef _IO_H_
#define _IO_H_
#include <kernel.h>

/* Function prototypes. */
int io_puts(const char *s, int32_t n);
int io_gets(char *s, int32_t n);

#endif /* _IO_H_ */
