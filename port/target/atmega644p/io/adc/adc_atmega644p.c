/*
 * adc_atmega644p.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_ADC
#include <avr/interrupt.h>
#include <adc_atmega644p.h>
#include <adc.h>

ADC_DATA_CALLBACK *adc_callback = NULL;

/*
 * ISR(TIMER0_COMPA_vect, ISR_BLOCK)
 * This is ADC interrupt that will be called when an ADC interrupt is complete.
 */
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
    ;
} /* ISR(TIMER0_COMPA_vect, ISR_BLOCK) */

/*
 * ISR(ADC_vect, ISR_BLOCK)
 * This is ADC interrupt that will be called when an ADC interrupt is complete.
 */
ISR(ADC_vect, ISR_BLOCK)
{
    /* We have entered an ISR. */
    OS_ISR_ENTER();

    /* If we have a callback to call when ADC conversion completes. */
    if (adc_callback != NULL)
    {
        /* Call the callback with received data. */
        adc_callback(ADC);
    }

    /* We are now exiting the ISR. */
    OS_ISR_EXIT();

} /* ISR(ADC_vect, ISR_BLOCK) */

/*
 * adc_atmega644_init
 * This function is responsible for initializing ADC hardware for atmega644p
 * platform.
 */
void adc_atmega644_init()
{
    /* Initialize ADC multiplexer configuration. */
    ADMUX = (ADC_ATMEGA644P_REF);

    /* Initialize ADC control register. */
    ADCSRA = (ADC_ATMEGA644P_PRESCALE);
    ADCSRB = 0x00;

    /* Enable ADC. */
    ADCSRA |= (ADC_ATMEGA644P_ENABLE);

} /* adc_atmega644_init */

/*
 * adc_atmega644_channel_select
 * @channel: Channel from which we will be taking readings.
 * This function will select an ADC channel.
 */
void adc_atmega644_channel_select(uint32_t channel)
{
    /* Enable digital input on the required channel. */
    DIDR0 |= (1 << channel);

    /* Select the required ADC channel. */
    ADMUX &= (uint8_t)(~(0x1F));
    ADMUX |= (channel);

} /* adc_atmega644_channel_select */

/*
 * adc_atmega644_channel_unselect
 * @channel: Channel needed to be un-select.
 * This function will un-select an ADC channel.
 */
void adc_atmega644_channel_unselect(uint32_t channel)
{
    /* Enable digital input on the required channel. */
    DIDR0 &= (uint8_t)(~(1 << channel));

} /* adc_atmega644_channel_unselect */

/*
 * adc_atmega644_read
 * This function will take a reading from the given ADC channel.
 */
uint32_t adc_atmega644_read()
{
    /* Start ADC conversion. */
    ADCSRA |= (1 << ADSC);

    /* Wait for ADC conversion to complete. */
    while (ADCSRA & (1 << ADSC)) ;

    /* Read and return the ADC value. */
    return (ADC);

} /* adc_atmega644_read */

/*
 * adc_atmega644_periodic_read_start
 * @callback: Function needed to be called when an ADC conversion completes.
 * @period: Hardware ticks after which a ADC sample is required.
 * This function will start periodic sampling on ADC.
 */
void adc_atmega644_periodic_read_start(ADC_DATA_CALLBACK *callback, uint32_t period)
{
    /* Configure ADC to start a conversion on timer 0 compare interrupt. */
    ADCSRA |= (1 << ADATE);
    ADCSRB |= 0x03;

    /* Configure the timer 0 for required period. */
    TCCR0A = 0x02;
    TCCR0B = ADC_ATMEGA644P_TIMER_PRESCALE;
    OCR0A = (uint8_t)period;

    /* Enable timer interrupt. */
    TIMSK0 = 0x02;

    /* Set the ADC conversion callback. */
    adc_callback = callback;

    /* Enable ADC conversion interrupt. */
    ADCSRA |= (1 << ADIE);

} /* adc_atmega644_periodic_read_start */

/*
 * adc_atmega644_periodic_read_stop
 * @callback: Function needed to be called when an ADC conversion completes.
 * @period: Hardware ticks after which a ADC sample is required.
 * This function will start periodic sampling on ADC.
 */
void adc_atmega644_periodic_read_stop()
{
    /* Move back to free running mode. */
    ADCSRA &= (uint8_t) ~(1 << ADATE);

    /* Disable timer 0 interrupt. */
    TIMSK0 &= (uint8_t)~0x02;

} /* adc_atmega644_periodic_read_stop */

#endif /* CONFIG_ADC */
