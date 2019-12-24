/*
 * isr.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef ISR_H
#define ISR_H

#include <kernel.h>

/* ISR callback definition. */
typedef void (*const ISR)(void);

/* Exported variables. */
extern uint32_t sys_stack_start;
extern ISR system_isr_table[];

#endif /* ISR_H */
