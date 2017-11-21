/*
 * ppp_stm32f103.h
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
#ifndef _PPP_STM32F103_H_
#define _PPP_STM32F103_H_
#include <kernel.h>

#ifdef CONFIG_PPP

#ifndef CMAKE_BUILD
/* PPP configurations. */
#define PPP_BAUD_RATE                   (115200)
#define PPP_MAX_BUFFER_SIZE             (32)
#define PPP_NUM_BUFFERS                 (16)
#define PPP_NUM_BUFFER_LIST             (8)
#define PPP_THRESHOLD_BUFFER            (2)
#define PPP_THRESHOLD_BUFFER_LIST       (2)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void ppp_stm32f103_init(void);

#endif /* CONFIG_PPP */
#endif /* _PPP_STM32F103_H_ */
