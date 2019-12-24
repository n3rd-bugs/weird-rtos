/*
 * serial_target.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _SERIAL_TARGET_H_
#define _SERIAL_TARGET_H_
#include <kernel.h>
#include <usart_stm32f411.h>

/* Hook-up serial OS stack. */
#define SERIAL_TGT_INIT  serial_stm32f411_init

#endif /* _SERIAL_TARGET_H_ */
