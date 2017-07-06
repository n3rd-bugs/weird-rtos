/*
 * bootload_atmega644p.h
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _BOOTLOAD_ATMEGA644P_H_
#define _BOOTLOAD_ATMEGA644P_H_

#include <kernel.h>

#ifdef CONFIG_BOOTLOAD
#define BOOTLOADER_LOADED
#define BOOTLOAD_MMC
#define BOOTLOAD_STK
//#define BOOTLOAD_MMC_HEX_NONLINEAR

#define BOOTLOAD_COMPLETE   -21000
#define BOOTLOAD_ERROR      -21001

/* Serial configurations for boot loader. */
#define BOOT_BAUD_RATE      (115200)
#define BOOT_BAUD_TOL       (5)

/* Macro to be used to move a function in the boot loader section. */
#define BOOTLOAD_SECTION    __attribute__ ((section (".boot")))
#define BOOTVECTOR_SECTION  __attribute__ ((section (".boot_vector")))
#define BOOTLOAD_RESET      0x1f000

/* Defines the condition when we need to perform boot load operation. */
#define BOOTLOAD_COND_INIT  (DDRD &= ((uint8_t)~(1 << 3)))
#define BOOTLOAD_COND       ((PIND & (1 << 3)) == 0)

/* Helper macros. */
#define BOOTLOAD_BTOH(a)    (((a) > 9) ? (a) + 0x37 : (a) + '0')

/* Link the boot loader API. */
#define BOOTLOAD            bootload_atmega644p

/* Function prototypes. */
#ifdef BOOTLOADER_LOADED
void bootload_atmega644p();
#else
void bootload_atmega644p() BOOTLOAD_SECTION;

int32_t bootload_disk_initialize(uint8_t *) BOOTLOAD_SECTION;
int32_t bootload_disk_read(uint8_t, uint8_t *, uint32_t, uint32_t, uint32_t *) BOOTLOAD_SECTION;
#if _USE_WRITE
int32_t bootload_disk_write(uint8_t, const uint8_t *, uint32_t, uint32_t) BOOTLOAD_SECTION;
#endif
#if _USE_IOCTL
int32_t bootload_disk_ioctl(uint8_t, void *) BOOTLOAD_SECTION;
#endif
#endif /* BOOTLOADER_LOADED */

#endif /* CONFIG_BOOTLOAD */
#endif /* _BOOTLOAD_ATMEGA644P_H_ */
