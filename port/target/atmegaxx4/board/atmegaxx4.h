/*
 * atmegaxx4.h
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
#ifndef _ATMEGAXX4_H_
#define _ATMEGAXX4_H_

#ifdef CONFIG_SERIAL
#include <usart_avr.h>
#endif /* CONFIG_SERIAL */

#ifdef CONFIG_FS
#include <fs_target.h>
#endif /* CONFIG_FS */

/* Function prototypes. */
#ifdef CONFIG_SLEEP
uint64_t current_hardware_tick(void);
#endif /* CONFIG_SLEEP */

#endif /* _ATMEGAXX4_H_ */
