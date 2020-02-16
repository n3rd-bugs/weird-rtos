/*
 * adc_target.h
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
#ifndef _ADC_TARGET_H_
#define _ADC_TARGET_H_
#include <kernel.h>

#ifdef IO_ADC
#include <adc_stm32f103.h>

/* Hook-up ADC OS stack. */
#define ADC_TGT_INIT                adc_stm32f103_init
#define ADC_TGT_CHANNEL_SELECT      adc_stm32f103_channel_select
#define ADC_TGT_CHANNEL_UNSELECT    adc_stm32f103_channel_unselect
#define ADC_TGT_READ                adc_stm32f103_read

#endif /* IO_ADC */
#endif /* _ADC_TARGET_H_ */
