/*
 * adc.c
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
#include <adc.h>

/*
 * adc_init
 * This function is responsible for initializing ADC subsystem.
 */
void adc_init()
{
#ifdef ADC_TGT_INIT
    /* Initialize ADC hardware. */
    ADC_TGT_INIT();
#endif

} /* adc_init */

/*
 * adc_channel_select
 * @channel: ADC channel from which we need to take readings.
 * This function will select a required ADC channel.
 */
void adc_channel_select(uint32_t channel)
{
    /* This function will select an ADC channel. */
    ADC_TGT_CHANNEL_SELECT(channel);

} /* adc_channel_select */

/*
 * adc_channel_unselect
 * @channel: ADC channel needed to un-selected.
 * This function will un-selcet an ADC channel.
 */
void adc_channel_unselect(uint32_t channel)
{
    /* Uns-elect the required ADC channel. */
    ADC_TGT_CHANNEL_UNSELECT(channel);

} /* adc_read */

/*
 * adc_read
 * This function will take a reading from the ADC.
 */
uint32_t adc_read()
{
    /* Take an ADC reading and return it's value. */
    return (ADC_TGT_READ());

} /* adc_read */

/*
 * adc_read_average
 * @num_readings: Number of reading to take.
 * This function will take given number of readings from ADC and return an
 * average value.
 */
uint32_t adc_read_average(uint32_t num_readings)
{
    uint32_t i, ret_value;

    /* Take required number of readings. */
    for (i = 0, ret_value = 0; i < num_readings; i ++)
    {
        /* Read and accumulate ADC reading. */
        ret_value += adc_read();
    }

    /* Return the average value. */
    return (ret_value / num_readings);

} /* adc_read_average */

#endif /* CONFIG_ADC */