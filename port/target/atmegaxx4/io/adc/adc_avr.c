/*
 * adc_avr.c
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
#include <kernel.h>

#ifdef IO_ADC
#include <avr/interrupt.h>
#include <adc_avr.h>
#include <adc.h>

ADC_DATA_CALLBACK *adc_callback = NULL;

/*
 * ISR(TIMER0_COMPA_vect, ISR_BLOCK)
 * This is stubbed TIMER0 ISR routine.
 */
ISR(TIMER0_COMPA_vect, ISR_NAKED)
{
    /* Return from a naked ISR. */
    ISR_RETURN();

} /* ISR(TIMER0_COMPA_vect, ISR_BLOCK) */

/*
 * ISR(ADC_vect, ISR_NAKED)
 * This is ADC interrupt that will be called when an ADC interrupt is complete.
 */
ISR(ADC_vect, ISR_NAKED)
{
    /* We have entered an ISR. */
    ISR_ENTER();

    /* If we have a callback to call when ADC conversion completes. */
    if (adc_callback != NULL)
    {
        /* Call the callback with received data. */
        adc_callback(ADC);
    }

    /* We are now exiting the ISR. */
    ISR_EXIT();

} /* ISR(ADC_vect, ISR_NAKED) */

/*
 * adc_avr_init
 * This function is responsible for initializing ADC hardware for AVR
 * platform.
 */
void adc_avr_init(void)
{
    /* Initialize ADC multiplexer configuration. */
    ADMUX = (ADC_AVR_REF);

    /* Initialize ADC control register. */
    ADCSRA = (ADC_AVR_PRESCALE);
    ADCSRB = 0x0;

    /* Enable ADC. */
    ADCSRA |= (ADC_AVR_ENABLE);

} /* adc_avr_init */

/*
 * adc_avr_channel_select
 * @channel: Channel from which we will be taking readings.
 * This function will select an ADC channel.
 */
void adc_avr_channel_select(uint32_t channel)
{
    /* Enable digital input on the required channel. */
    DIDR0 |= (1 << channel);

    /* Select the required ADC channel. */
    ADMUX &= (uint8_t)(~(0x1F));
    ADMUX |= (channel);

} /* adc_avr_channel_select */

/*
 * adc_avr_channel_unselect
 * @channel: Channel needed to be un-select.
 * This function will un-select an ADC channel.
 */
void adc_avr_channel_unselect(uint32_t channel)
{
    /* Disable digital input on the required channel. */
    DIDR0 &= (uint8_t)(~(1 << channel));

} /* adc_avr_channel_unselect */

/*
 * adc_avr_read
 * @return: Returns the ADC reading.
 * This function will take a reading from the given ADC channel.
 */
uint32_t adc_avr_read(void)
{
    /* Start ADC conversion. */
    ADCSRA |= (1 << ADSC);

    /* Wait for ADC conversion to complete. */
    while (ADCSRA & (1 << ADSC)) ;

    /* Read and return the ADC value. */
    return (ADC);

} /* adc_avr_read */

/*
 * adc_avr_periodic_read_start
 * @callback: Function needed to be called when an ADC conversion completes.
 * @period: Hardware ticks after which a ADC sample is required.
 * This function will start periodic sampling on ADC.
 */
void adc_avr_periodic_read_start(ADC_DATA_CALLBACK *callback, uint32_t period)
{
    /* Configure ADC to start a conversion on timer 0 compare interrupt. */
    ADCSRA |= (1 << ADATE);
    ADCSRB |= 0x3;

    /* Configure the timer 0 for required period. */
    TCCR0A = 0x2;
    TCCR0B = ADC_AVR_TIMER_PRESCALE;
    OCR0A = (uint8_t)period;

    /* Enable timer interrupt. */
    TIMSK0 = 0x2;

    /* Set the ADC conversion callback. */
    adc_callback = callback;

    /* Enable ADC conversion interrupt. */
    ADCSRA |= (1 << ADIE);

} /* adc_avr_periodic_read_start */

/*
 * adc_avr_periodic_read_stop
 * @callback: Function needed to be called when an ADC conversion completes.
 * @period: Hardware ticks after which a ADC sample is required.
 * This function will start periodic sampling on ADC.
 */
void adc_avr_periodic_read_stop(void)
{
    /* Move back to free running mode. */
    ADCSRA &= (uint8_t) ~(1 << ADATE);

    /* Disable timer 0 interrupt. */
    TIMSK0 &= (uint8_t)~0x2;

} /* adc_avr_periodic_read_stop */
#endif /* IO_ADC */
