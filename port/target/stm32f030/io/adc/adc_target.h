/*
 * adc_target.h
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
#ifndef _ADC_TARGET_H_
#define _ADC_TARGET_H_
#include <kernel.h>

#ifdef IO_ADC
#include <adc_stm32f030.h>

/* Hook-up ADC OS stack. */
#define ADC_TGT_INIT                adc_stm32f030_init
#define ADC_TGT_CHANNEL_INIT        adc_stm32f030_channel_init
#define ADC_TGT_CHANNEL_SELECT      adc_stm32f030_channel_select
#define ADC_TGT_CHANNEL_UNSELECT    adc_stm32f030_channel_unselect
#define ADC_TGT_READ                adc_stm32f030_read
#define ADC_TGT_READ_N              adc_stm32f030_read_n

#endif /* IO_ADC */
#endif /* _ADC_TARGET_H_ */
