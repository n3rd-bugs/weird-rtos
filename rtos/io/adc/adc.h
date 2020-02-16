/*
 * adc.h
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
#ifndef _ADC_H_
#define _ADC_H_

#include <kernel.h>

#ifdef IO_ADC

/* Include ADC target configuration. */
#include <adc_target.h>

/* Function prototypes. */
void adc_init(void);
void adc_channel_select(uint32_t);
void adc_channel_unselect(uint32_t);
uint32_t adc_read(void);
uint32_t adc_read_average(uint32_t);

#endif /* IO_ADC */

#endif /* _ADC_H_ */
