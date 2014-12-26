/*
 * serial.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>

/* Serial stream definition. */
extern FILE serial_fd;

/* Function prototypes. */
void serial_init();

#endif /* _SERIAL_H_ */
