/*
 * arm_gcc.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */

/* This macro is used to tell compiler to not manage stack for it. */

#ifndef _ARM_GCC_H_
#define _ARM_GCC_H_

#define STACK_LESS          __attribute__ ((naked))
#define NOINLINE            __attribute__ ((noinline))

#define ISR_FUN             void __attribute__ ((interrupt))
#define NAKED_ISR_FUN       void __attribute__ ((interrupt, naked))
#define NAKED_FUN           void __attribute__ ((naked))

#define NOOPTIMIZATION      __attribute__((optimize("O0")))
#define SPEEDOPTIMIZATION   __attribute__((optimize("O3")))

#define LITTLE_ENDIAN

/* Function prototypes. */
void io_arm_init();
int _write(int, char *, int);
int _read(int, char *, int);

#endif /* _ARM_GCC_H_ */
