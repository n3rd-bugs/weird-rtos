/*
 * ethernet_avr.h
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
#ifndef _ETHERNET_AVR_H_
#define _ETHERNET_AVR_H_
#include <kernel.h>

#ifdef IO_ETHERNET

/* Function prototypes. */
void ethernet_avr_init(void);

#endif /* IO_ETHERNET */
#endif /* _ETHERNET_AVR_H_ */
