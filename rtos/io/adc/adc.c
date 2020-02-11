/*
 * adc.c
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
#include <adc.h>

/*
 * adc_init
 * This function is responsible for initializing ADC subsystem.
 */
void adc_init(void)
{
#ifdef ADC_TGT_INIT
    /* Initialize ADC hardware. */
    ADC_TGT_INIT();
#endif

} /* adc_init */

/*
 * adc_channel_init
 * @channel: ADC channel from which we will be taking readings.
 * This function will initialize an ADC channel.
 */
void adc_channel_init(uint32_t channel)
{
#ifdef ADC_TGT_CHANNEL_INIT
    /* This function will initialize an ADC channel. */
    ADC_TGT_CHANNEL_INIT(channel);
#else
    UNUSED_PARAM(channel);
#endif /* ADC_TGT_CHANNEL_INIT */

} /* adc_channel_init */

/*
 * adc_channel_select
 * @channel: ADC channel from which we need to take readings.
 * This function will select a required ADC channel.
 */
void adc_channel_select(uint32_t channel)
{
#ifdef ADC_TGT_CHANNEL_SELECT
    /* This function will select an ADC channel. */
    ADC_TGT_CHANNEL_SELECT(channel);
#else
    UNUSED_PARAM(channel);
#endif /* ADC_TGT_CHANNEL_SELECT */

} /* adc_channel_select */

/*
 * adc_channel_unselect
 * @channel: ADC channel needed to un-selected.
 * This function will un-selcet an ADC channel.
 */
void adc_channel_unselect(uint32_t channel)
{
#ifdef ADC_TGT_CHANNEL_UNSELECT
    /* Un-select the required ADC channel. */
    ADC_TGT_CHANNEL_UNSELECT(channel);
#else
    UNUSED_PARAM(channel);
#endif /* ADC_TGT_CHANNEL_UNSELECT */

} /* adc_channel_unselect */

/*
 * adc_read
 * This function will take a reading from the ADC.
 */
ADC_SAMPLE adc_read(void)
{
    /* Take an ADC reading and return it's value. */
    return (ADC_TGT_READ());

} /* adc_read */

/*
 * adc_read_n
 * @buffer: Buffer in which samples are needed to be read.
 * @n: Number of samples to collect.
 * This function will take the given number of samples from the ADC.
 */
void adc_read_n(ADC_SAMPLE *buffer, uint32_t n)
{
#ifdef ADC_TGT_READ_N
    /* Take an ADC reading and return it's value. */
    ADC_TGT_READ_N(buffer, n);
#else
    UNUSED_PARAM(buffer);
    UNUSED_PARAM(n);
#endif

} /* adc_read_n */

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

#endif /* IO_ADC */
