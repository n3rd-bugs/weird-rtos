/*
 * adc_stm32.h
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
#ifndef _ADC_STM32_H_
#define _ADC_STM32_H_
#include <kernel.h>

#ifdef IO_ADC
#include <adc.h>
#include <adc_stm32_config.h>

/* ADC channel definitions. */
#define ADC_STM32_CHN_PB0   (8)
#define ADC_STM32_CHN_PB1   (9)

/* Function prototypes. */
void adc_stm32_init(void);
void adc_stm32_channel_select(uint32_t);
void adc_stm32_channel_unselect(uint32_t);
uint32_t adc_stm32_read(void);

#endif /* IO_ADC */
#endif /* _ADC_STM32_H_ */
