/*
 * adc_stm32f030.h
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
#ifndef _ADC_STM32F030_H_
#define _ADC_STM32F030_H_
#include <kernel.h>

#ifdef IO_ADC
#include <adc.h>

/* Type definition for ADC sample. */
typedef uint16_t ADC_SAMPLE;

/* Function prototypes. */
void adc_stm32f030_init(void);
void adc_stm32f030_channel_init(uint32_t);
void adc_stm32f030_channel_select(uint32_t);
void adc_stm32f030_channel_unselect(uint32_t);
ADC_SAMPLE adc_stm32f030_read(void);
void adc_stm32f030_read_n(ADC_SAMPLE *, uint32_t);

#endif /* IO_ADC */
#endif /* _ADC_STM32F030_H_ */
