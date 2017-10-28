/*
 * arm_gcc.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

/* This macro is used to tell compiler to not manage stack for it. */

#ifndef _ARM_GCC_H_
#define _ARM_GCC_H_

#ifndef CMAKE_BUILD
#define TARGET_HEAP_SIZE    2048
#endif /* CMAKE_BUILD */

#define STACK_LESS          __attribute__ ((naked))
#define NOINLINE            __attribute__ ((noinline))

#define ISR_FUN             void __attribute__ ((interrupt))
#define NAKED_ISR_FUN       void __attribute__ ((interrupt, naked))
#define NAKED_FUN           void __attribute__ ((naked))

#define NOOPTIMIZATION      __attribute__((optimize("O0")))
#define SPEEDOPTIMIZATION   __attribute__((optimize("O3")))

#define LITTLE_ENDIAN

/* If buffered IO is required. */
#define IO_BUFFERED

/* Function prototypes. */
void rtl_arm_init(void);
int _write(int, char *, int);
int _read(int, char *, int);
void *_sbrk(int);

#endif /* _ARM_GCC_H_ */
