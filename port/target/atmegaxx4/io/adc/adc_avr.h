/*
 * adc_avr.h
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
#ifndef _ADC_AVR_H_
#define _ADC_AVR_H_
#include <kernel.h>

#ifdef IO_ADC
#include <adc.h>
#include <adc_avr_config.h>

/* ADC configuration macros. */
/* ADMUX register definitions. */
#define ADC_AVR_REF_AREF            (0)
#define ADC_AVR_REF_AVCC            (0x1 << 6)
#define ADC_AVR_REF_INT_1_1         (0x2 << 6)
#define ADC_AVR_REF_INT_2_56        (0x3 << 6)

/* ADCSRA register definitions. */
#define ADC_AVR_ENABLE              (0x1 << ADEN)
#define ADC_AVR_START               (0x1 << ADSC)
#define ADC_AVR_DIV_2               (0x1)
#define ADC_AVR_DIV_4               (0x2)
#define ADC_AVR_DIV_8               (0x3)
#define ADC_AVR_DIV_16              (0x4)
#define ADC_AVR_DIV_32              (0x5)
#define ADC_AVR_DIV_64              (0x6)
#define ADC_AVR_DIV_128             (0x7)

/* TIMER0 prescale configurations. */
#define TIMER0_AVR_DIV_1            (0x1)
#define TIMER0_AVR_DIV_8            (0x2)
#define TIMER0_AVR_DIV_64           (0x3)
#define TIMER0_AVR_DIV_256          (0x4)
#define TIMER0_AVR_DIV_1024         (0x5)

/* This is callback function when ADC data is available to read. */
typedef void ADC_DATA_CALLBACK (uint32_t);

/* Function prototypes. */
void adc_avr_init(void);
void adc_avr_channel_select(uint32_t);
void adc_avr_channel_unselect(uint32_t);
uint32_t adc_avr_read(void);
void adc_avr_periodic_read_start(ADC_DATA_CALLBACK *, uint32_t);
void adc_avr_periodic_read_stop(void);

#endif /* IO_ADC */
#endif /* _ADC_AVR_H_ */
