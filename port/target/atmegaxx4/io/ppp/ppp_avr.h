/*
 * ppp_avr.h
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
#ifndef _PPP_AVR_H_
#define _PPP_AVR_H_
#include <kernel.h>

#ifdef CONFIG_PPP
#include <ppp_avr_config.h>

/* Function prototypes. */
void ppp_avr_init(void);

#endif /* CONFIG_PPP */
#endif /* _PPP_AVR_H_ */
