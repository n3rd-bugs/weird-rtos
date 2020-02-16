/*
 * adc_stm32f030.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
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
#include <adc_stm32f030.h>
#include <adc_stm32_config.h>

/*
 * adc_stm32f030_init
 * This function is responsible for initializing ADC hardware.
 */
void adc_stm32f030_init(void)
{
    /* Reset ADC1. */
    RCC->APB2RSTR |= RCC_APB2RSTR_ADCRST;
    RCC->APB2RSTR &= (uint32_t)~RCC_APB2RSTR_ADCRST;

    /* Enable AHB clock for ADC. */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* Setup ADC. */
    /* Disable everything. */
    ADC1->CFGR1 &= (uint32_t)~(ADC_CFGR1_AWDCH | ADC_CFGR1_AWDEN | ADC_CFGR1_AWDSGL | ADC_CFGR1_DISCEN | ADC_CFGR1_AUTOFF | ADC_CFGR1_WAIT | ADC_CFGR1_CONT | ADC_CFGR1_OVRMOD | ADC_CFGR1_EXTEN | ADC_CFGR1_EXTSEL | ADC_CFGR1_ALIGN | ADC_CFGR1_RES | ADC_CFGR1_SCANDIR | ADC_CFGR1_DMACFG | ADC_CFGR1_DMAEN);

    /* Select ADC clock. */
    ADC1->CFGR2 &= (uint32_t)~ADC_CFGR2_CKMODE;

    /* Set ADC sample time. */
    ADC1->SMPR &= (uint32_t)~ADC_SMPR_SMP;
    ADC1->SMPR |= ADC_STM32_SAMPLE_TIME;

#if (ADC_STM32_PERIODIC_INTERVAL > 0)
    /* Setup Timer 1 for the periodic sampling. */
    /* Enable clock for TIMER1 */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    /* Count up, and count with the frequency of PCLK/2. */
    TIM1->CR1 &= (uint16_t)~(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD);

    /* Reload when overflow. */
    TIM1->ARR = US_TO_HW_TICK(ADC_STM32_PERIODIC_INTERVAL);

    /* Clear the initial count. */
    TIM1->CNT = 0;

    /* No pre-scale. */
    TIM1->PSC = 0;

    /* Use timer update as TRGO. */
    TIM1->CR2 &= (uint16_t)~(TIM_CR2_MMS);
    TIM1->CR2 |= TIM_CR2_MMS_1;

    /* Only timer overflow will generate update event. */
    TIM1->CR1 |= TIM_CR1_URS;

    /* Enable timer. */
    TIM1->CR1 |= TIM_CR1_CEN;

    /* Update the ADC to use external trigger, on rising edge. */
    ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0;
#endif /* (ADC_STM32_PERIODIC_INTERVAL > 0) */

    /* Enable ADC. */
    ADC1->CR |= ADC_CR_ADEN;
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) ;

} /* adc_stm32f030_init */

/*
 * adc_stm32f030_channel_init
 * @channel: Channel from which we will be taking readings.
 * This function will initialize the given ADC channel.
 */
void adc_stm32f030_channel_init(uint32_t channel)
{
    /* ADC channels on GPIOA. */
    if (channel < 8)
    {
        /* Enable clock for GPIOA. */
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

        /* Set analog for given channel. */
        GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER0 << (channel * 2));
        GPIOA->MODER |= ((GPIO_MODER_MODER0_0 | GPIO_MODER_MODER0_1) << (channel * 2));

        /* Disable pull-up on the given channel. */
        GPIOA->PUPDR &= (uint32_t)~(GPIO_PUPDR_PUPDR0 << (channel * 2));
    }

    /* ADC channel on GPIOB */
    else if (channel == 9)
    {
        /* Enable clock for GPIOB. */
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

        /* Set analog for given channel. */
        GPIOB->MODER &= (uint32_t)~(GPIO_MODER_MODER0 << ((channel - 8) * 2));
        GPIOB->MODER |= ((GPIO_MODER_MODER0_0 | GPIO_MODER_MODER0_1) << ((channel - 8) * 2));

        /* Disable pull-up on the given channel. */
        GPIOB->PUPDR &= (uint32_t)~(GPIO_PUPDR_PUPDR0 << ((channel - 8) * 2));
    }

} /* adc_stm32f030_channel_init */

/*
 * adc_stm32f030_channel_select
 * @channel: Channel from which we will be taking readings.
 * This function will select an ADC channel.
 */
void adc_stm32f030_channel_select(uint32_t channel)
{
    /* Set the required channel. */
    ADC1->CHSELR = (uint32_t)(1 << channel);

} /* adc_stm32f030_channel_select */

/*
 * adc_stm32f030_channel_unselect
 * @channel: Channel needed to be un-select.
 * This function will un-select an ADC channel.
 */
void adc_stm32f030_channel_unselect(uint32_t channel)
{
    /* Un-select the channel. */
    ADC1->CHSELR &= (uint32_t)~(1 << channel);

} /* adc_stm32f030_channel_unselect */

/*
 * adc_stm32f030_read
 * @return: Returns the ADC reading.
 * This function will take a reading from the given ADC channel.
 */
ADC_SAMPLE adc_stm32f030_read(void)
{
    /* Start ADC conversion. */
    ADC1->CR |= ADC_CR_ADSTART;

    /* Wait for ADC conversion to complete. */
    while ((ADC1->ISR & ADC_ISR_EOC) == 0) ;

    /* Stop ADC conversion. */
    ADC1->CR |= ADC_CR_ADSTP;

    /* Read and return the ADC value. */
    return ((ADC1->DR) & 0xFFFF);

} /* adc_stm32f030_read */

/*
 * adc_stm32f030_read_n
 * @buffer: Buffer in which ADC samples are needed to be collected.
 * @n: Number of items in the buffer.
 * This function will take a reading from the given ADC channel.
 */
void adc_stm32f030_read_n(ADC_SAMPLE *buffer, uint32_t n)
{
    uint32_t i;

    /* Start ADC conversion. */
    ADC1->CR |= ADC_CR_ADSTART;

    for (i = 0; i < n; i++)
    {
        /* Wait for ADC conversion to complete. */
        while ((ADC1->ISR & ADC_ISR_EOC) == 0) ;

        /* Save this sample. */
        buffer[i] = ((ADC1->DR) & 0xFFFF);
    }

    /* Stop ADC conversion. */
    ADC1->CR |= ADC_CR_ADSTP;

} /* adc_stm32f030_read_n */

#endif /* IO_ADC */
