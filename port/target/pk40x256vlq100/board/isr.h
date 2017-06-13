/*
 * isr.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include <os.h>

/* ISR callback definition. */
typedef void (*const isr)(void);
typedef IRQInterruptIndex interrupt;

/* IRS table type definition. */
typedef struct {
    isr     callback[0x78];
} VECTOR_TABLE;

/* Exported variables. */
extern uint32_t sys_stack_start;
extern VECTOR_TABLE system_isr_table;

#endif /* ISR_H */
