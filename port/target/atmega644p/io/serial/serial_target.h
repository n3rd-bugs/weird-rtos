/*
 * serial_target.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _SERIAL_TARGET_H_
#define _SERIAL_TARGET_H_
#include <os.h>
#include <usart_atmega644p.h>

/* Hook-up serial OS stack. */
#define SERIAL_TGT_INIT  serial_atmega644p_init

#endif /* _SERIAL_TARGET_H_ */
