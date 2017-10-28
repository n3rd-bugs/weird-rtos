/*
 * avr_gcc.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

/* This macro is used to tell compiler to not manage stack for it. */

#ifndef _AVR_GCC_H_
#define _AVR_GCC_H_
#include <avr/pgmspace.h>

#define STACK_LESS          __attribute__ ((naked))
#define NOINLINE            __attribute__ ((noinline))

#define ISR_FUN             void __attribute__ ((interrupt))
#define NAKED_ISR_FUN       void __attribute__ ((interrupt, naked))
#define NAKED_FUN           void __attribute__ ((naked))

#define NOOPTIMIZATION      __attribute__((optimize("O0")))
#define SPEEDOPTIMIZATION   __attribute__((optimize("O3")))

#define LITTLE_ENDIAN

#define P_STR(s)            PSTR(s)
#define P_STR_T             PGM_P
#define P_STR_MEM           PROGMEM
#define P_STR_CPY           strcpy_P
#define P_STR_NCPY          strncpy_P
#define P_STR_LEN           strlen_P
#define P_MEM_CPY           memcpy_P

/* Function prototypes. */
void rtl_avr_init(void);

#endif /* _AVR_GCC_H_ */
