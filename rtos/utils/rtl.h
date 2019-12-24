/*
 * rtl.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _RTL_H_
#define _RTL_H_

#include <stdint.h>

/* Helper definitions. */
#define RTL_ULTOA_MAX_DIGIT         10
#define RTL_ULTOA_FIRST_DIV         1000000000

/* ULTOA flag definitions. */
#define RTL_ULTOA_LEADING_ZEROS     0x01

/* Function overrides. */
#define rtl_ultoa_b10(a, b)         rtl_ultoa(a, b, 0, 0)

/* Function prototypes. */
void rtl_ultoa(uint32_t, uint8_t *, uint32_t, uint8_t flags);

#endif /* _RTL_H_ */
