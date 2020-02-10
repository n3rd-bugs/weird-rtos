/*
 * adc_target.h
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
#ifndef _ADC_TARGET_H_
#define _ADC_TARGET_H_
#include <kernel.h>

#ifdef IO_ADC
#include <adc_avr.h>

/* Hook-up ADC OS stack. */
#define ADC_TGT_INIT                adc_avr_init
#define ADC_TGT_CHANNEL_SELECT      adc_avr_channel_select
#define ADC_TGT_CHANNEL_UNSELECT    adc_avr_channel_unselect
#define ADC_TGT_READ                adc_avr_read

#endif /* IO_ADC */
#endif /* _ADC_TARGET_H_ */
