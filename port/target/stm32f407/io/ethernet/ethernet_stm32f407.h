/*
 * ethernet_stm32f407.h
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
#ifndef _ETHERNET_STM32F407_H_
#define _ETHERNET_STM32F407_H_
#include <kernel.h>

#ifdef CONFIG_ETHERNET

/* Function prototypes. */
void ethernet_stm32f407_init(void);

#endif /* CONFIG_ETHERNET */
#endif /* _ETHERNET_STM32F407_H_ */
