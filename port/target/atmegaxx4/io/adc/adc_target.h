/*
 * adc_target.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _ADC_TARGET_H_
#define _ADC_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_ADC
#include <adc_avr.h>

/* Hook-up ADC OS stack. */
#define ADC_TGT_INIT                adc_avr_init
#define ADC_TGT_CHANNEL_SELECT      adc_avr_channel_select
#define ADC_TGT_CHANNEL_UNSELECT    adc_avr_channel_unselect
#define ADC_TGT_READ                adc_avr_read

#endif /* CONFIG_ADC */
#endif /* _ADC_TARGET_H_ */
