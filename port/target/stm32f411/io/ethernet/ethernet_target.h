/*
 * ethernet_target.h
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
#ifndef _ETHERNET_TARGET_H_
#define _ETHERNET_TARGET_H_

#include <kernel.h>

#ifdef IO_ETHERNET
#include <ethernet_stm32f411.h>

/* Hook-up ethernet OS stack. */
#define ETHERENET_TGT_INIT  ethernet_stm32f411_init

#endif /* IO_ETHERNET */
#endif /* _ETHERNET_TARGET_H_ */
