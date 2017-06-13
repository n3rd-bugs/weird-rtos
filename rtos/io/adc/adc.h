/*
 * adc.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */

#ifndef _ADC_H_
#define _ADC_H_

#include <os.h>

#ifdef CONFIG_ADC

/* This is callback function when ADC data is available to read. */
typedef void ADC_DATA_CALLBACK (uint32_t);

/* Include ADC target configuration. */
#include <adc_target.h>

/* Function prototypes. */
void adc_init();
void adc_channel_select(uint32_t);
void adc_channel_unselect(uint32_t);
uint32_t adc_read();
uint32_t adc_read_average(uint32_t);

#endif /* CONFIG_ADC */

#endif /* _ADC_H_ */
