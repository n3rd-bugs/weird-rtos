/*
 * os_pk40x256vlq100.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#ifndef OS_PK40X256VLQ100_H
#define OS_PK40X256VLQ100_H

#include <os.h>
#include <MK40DZ10.h>

/* Peripheral clock configuration. */
#define SYS_FREQ                900000000
#define PCLK_FREQ               48000000

/* Required definitions for scheduling. */
#define CORTEX_M4_PEND_SV_REG           (SCB_ICSR)
#define CORTEX_M4_PEND_SV_MAST          (SCB_ICSR_PENDSVSET_MASK)
#define CORTEX_M4_INT_PEND_SV_PRI_REG   (NVICIP14)
#define CORTEX_M4_SYS_TICK_REG          (SYST_CSR)
#define CORTEX_M4_SYS_TICK_MASK         (SysTick_CSR_TICKINT_MASK)
#define CORTEX_M4_INT_SYS_TICK_PRI_REG  (NVICIP15)
#define CORTEX_M4_INT_SYS_PRI           (0xFF)

#define CORTEX_M4_FPU                   (FALSE)

/* Platform dependent macros. */
#define current_system_tick64()         pit_get_clock()
#define current_system_tick64_usec()    (pit_get_clock() / PCLK_FREQ)

/* Function prototypes. */
uint64_t pit_get_clock();

#endif /* OS_PK40X256VLQ100_H */
