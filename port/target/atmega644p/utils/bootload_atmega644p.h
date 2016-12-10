/*
 * bootload_atmega644p.h
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
#ifndef _BOOTLOAD_ATMEGA644P_H_
#define _BOOTLOAD_ATMEGA644P_H_

#include <os.h>

#ifdef CONFIG_BOOTLOAD
#define BOOTLOADER_LOADED

/* Serial configurations for boot loader. */
#define BOOT_BAUD_RATE      (115200)
#define BOOT_BAUD_TOL       (5)

/* Macro to be used to move a function in the boot loader section. */
#define BOOTLOAD_SECTION    __attribute__ ((section (".boot")))
#define BOOTLOAD_RESET      0x1f800

/* Defines the condition when we need to perform boot load operation. */
#define BOOTLOAD_COND_INIT  (DDRD &= ((uint8_t)~(1 << 3)))
#define BOOTLOAD_COND       ((PIND & (1 << 3)) == 0)

/* Link the boot loader API. */
#define BOOTLOAD            bootload_atmega644p

/* Function prototypes. */
#ifdef BOOTLOADER_LOADED
void bootload_atmega644p();
#else
void bootload_atmega644p() BOOTLOAD_SECTION;
void stk500_reply(uint8_t) BOOTLOAD_SECTION;
void stk500_empty_reply() BOOTLOAD_SECTION;
void bootload_atmega644p_putc(volatile uint8_t) BOOTLOAD_SECTION;
uint8_t bootload_atmega644p_getc() BOOTLOAD_SECTION;
#endif

#endif /* CONFIG_BOOTLOAD */
#endif /* _BOOTLOAD_ATMEGA644P_H_ */
