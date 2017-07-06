/*
 * adc_atmega644p.h
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

#ifndef _ADC_ATMEGA644P_H_
#define _ADC_ATMEGA644P_H_

#include <kernel.h>

#ifdef CONFIG_ADC
#include <adc.h>

/* ADC configuration macros. */
/* ADMUX register definitions. */
#define ADC_ATMEGA644P_REF_AREF         (0)
#define ADC_ATMEGA644P_REF_AVCC         (0x1 << 6)
#define ADC_ATMEGA644P_REF_INT_1_1      (0x2 << 6)
#define ADC_ATMEGA644P_REF_INT_2_56     (0x3 << 6)

/* ADCSRA register definitions. */
#define ADC_ATMEGA644P_ENABLE           (0x1 << ADEN)
#define ADC_ATMEGA644P_START            (0x1 << ADSC)
#define ADC_ATMEGA644P_DIV_2            (0x1)
#define ADC_ATMEGA644P_DIV_4            (0x2)
#define ADC_ATMEGA644P_DIV_8            (0x3)
#define ADC_ATMEGA644P_DIV_16           (0x4)
#define ADC_ATMEGA644P_DIV_32           (0x5)
#define ADC_ATMEGA644P_DIV_64           (0x6)
#define ADC_ATMEGA644P_DIV_128          (0x7)

/* TIMER0 prescale configurations. */
#define TIMER0_ATMEGA644P_DIV_1         (0x01)
#define TIMER0_ATMEGA644P_DIV_8         (0x02)
#define TIMER0_ATMEGA644P_DIV_64        (0x03)
#define TIMER0_ATMEGA644P_DIV_256       (0x04)
#define TIMER0_ATMEGA644P_DIV_1024      (0x05)

/* Default ADC configuration. */
#define ADC_ATMEGA644P_REF              (ADC_ATMEGA644P_REF_INT_2_56)
#define ADC_ATMEGA644P_PRESCALE         (ADC_ATMEGA644P_DIV_128)
#define ADC_ATMEGA644P_TIMER_PRESCALE   (TIMER0_ATMEGA644P_DIV_64)

/* Function prototypes. */
void adc_atmega644_init();
void adc_atmega644_channel_select(uint32_t);
void adc_atmega644_channel_unselect(uint32_t);
uint32_t adc_atmega644_read();
void adc_atmega644_periodic_read_start(ADC_DATA_CALLBACK *, uint32_t);
void adc_atmega644_periodic_read_stop();

#endif /* CONFIG_ADC */
#endif /* _ADC_ATMEGA644P_H_ */
