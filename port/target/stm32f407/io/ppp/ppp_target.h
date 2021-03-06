/*
 * ppp_target.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _PPP_TARGET_H_
#define _PPP_TARGET_H_
#include <kernel.h>

#ifdef IO_PPP
#include <ppp_stm32f407.h>

/* Hook-up PPP OS stack. */
#define PPP_TGT_INIT  ppp_stm32f407_init

#endif /* IO_PPP */
#endif /* _PPP_TARGET_H_ */
