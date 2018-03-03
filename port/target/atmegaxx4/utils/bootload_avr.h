/*
 * bootload_avr.h
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _BOOTLOAD_AVR_H_
#define _BOOTLOAD_AVR_H_

#include <kernel.h>

#ifdef CONFIG_BOOTLOAD
#ifdef CMAKE_BUILD
#include <bootload_avr_config.h>
#else
#define BOOTLOADER_LOADED
#define BOOTLOAD_MMC
#define BOOTLOAD_STK
//#define BOOTLOAD_MMC_HEX_NONLINEAR
#endif /* CMAKE_BUILD */

/* Error code definitions. */
#define BOOTLOAD_COMPLETE   -21000
#define BOOTLOAD_ERROR      -21001

/* Serial configurations for boot loader. */
#define BOOTLOAD_BAUD_RATE  (115200)
#define BOOTLOAD_BAUD_TOL   (5)

/* Macro to be used to move a function in the boot loader section. */
#define BOOTLOAD_SECTION    __attribute__ ((section (".boot")))
#define BOOTVECTOR_SECTION  __attribute__ ((section (".boot_vector")))
#define BOOTLOAD_RESET      0x1f000

/* Defines the condition when we need to perform boot load operation. */
#define BOOTLOAD_COND_INIT  (DDRA &= ((uint8_t)~(1 << 6)))
#define BOOTLOAD_COND       ((PINA & (1 << 6)) == 0)

/* Helper macros. */
#define BOOTLOAD_BTOH(a)    (((a) > 9) ? (a) + 0x37 : (a) + '0')

/* Link the boot loader API. */
#define BOOTLOAD            bootload_entry

/* Function prototypes. */
void bootload_entry(void);
#ifndef BOOTLOADER_LOADED
void bootload_avr(void) BOOTLOAD_SECTION;
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
#endif /* _BOOTLOAD_AVR_H_ */
