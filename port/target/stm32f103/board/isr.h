/*
 * isr.h
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
#ifndef ISR_H
#define ISR_H

#include <kernel.h>

/* ISR callback definition. */
typedef void (*const isr)(void);
typedef IRQn_Type interrupt;

/* IRS table type definition. */
typedef struct {
    isr     callback[0x4C];
} VECTOR_TABLE;

/* Exported variables. */
extern uint32_t sys_stack_start;
extern VECTOR_TABLE system_isr_table;

#endif /* ISR_H */
