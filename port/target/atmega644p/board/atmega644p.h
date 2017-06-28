/*
 * atmega644p.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _ATMEGA644P_H_
#define _ATMEGA644P_H_

#ifdef CONFIG_BOOTLOAD
#include <bootload_atmega644p.h>
#endif

#include <usart_atmega644p.h>

#ifdef CONFIG_FS
#include <fs_target.h>
#endif

/* Any other configuration is being managed by Eclipse plugin for avr-gcc. */

#endif /* _ATMEGA644P_H_ */