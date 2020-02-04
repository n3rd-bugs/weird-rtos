/*
 * adc_stm32.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_ADC
#include <adc_stm32.h>
#include <adc.h>

/*
 * adc_stm32_init
 * This function is responsible for initializing ADC hardware for STM32
 * platform.
 */
void adc_stm32_init(void)
{
    /* Reset ADC1. */
    RCC->APB2RSTR |= (RCC_APB2Periph_ADC1);
    RCC->APB2RSTR &= (uint32_t)~(RCC_APB2Periph_ADC1);

    /* Select ADC pre-scaler (72 / 6 = 12MHz). */
    RCC->CFGR &= (uint32_t)~(0xC000);
    RCC->CFGR |= RCC_PCLK2_Div6;

    /* Enable clock for ADC1. */
    RCC->APB2ENR |= RCC_APB2Periph_ADC1;

    /* Initialize ADC. */
    ADC1->CR1 = 0x0;
    ADC1->CR2 = ADC_ExternalTrigConv_None;
    ADC1->SQR1 = 0x0;

    /* Enable ADC for calibration. */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Reset ADC calibration. */
    ADC1->CR2 |= ADC_CR2_RSTCAL;

    /* Wait for ADC calibration to reset. */
    while (ADC1->CR2 & ADC_CR2_RSTCAL)
    {
        ;
    }

    /* Start ADC calibration. */
    ADC1->CR2  |= ADC_CR2_CAL;

    /* Wait for ADC calibration. */
    while (ADC1->CR2 & ADC_CR2_CAL)
    {
        ;
    }

    /* Disable ADC. */
    ADC1->CR2 &= (uint32_t)~(ADC_CR2_ADON);

} /* adc_stm32_init */

/*
 * adc_stm32_channel_select
 * @channel: Channel from which we will be taking readings.
 * This function will select an ADC channel.
 */
void adc_stm32_channel_select(uint32_t channel)
{
    /* Initialize the required channel. */
    switch (channel)
    {
    /* If this is PB0 (IN8). */
    case ADC_STM32_CHN_PB0:

        /* Enable clock for GPIOB. */
        RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

        /* Configure GPIO mode AIN for PB0. */
        GPIOB->CRL &= (uint32_t)~(0xF << (0 << 2));
        GPIOB->CRL |= (((GPIO_Mode_AIN) & 0xF) << (0 << 2));

        /* Configure ADC sample time. */
        ADC1->SMPR2 &= (uint32_t)~(0x7 << (3 * ADC_STM32_CHN_PB0));
        ADC1->SMPR2 |= (ADC_SMP << (3 * ADC_STM32_CHN_PB0));

        break;

    /* If this is PB1 (IN9). */
    case ADC_STM32_CHN_PB1:

        /* Enable clock for GPIOB. */
        RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

        /* Configure GPIO mode AIN for PB1. */
        GPIOB->CRL &= (uint32_t)(~(0xF << (1<< 2)));
        GPIOB->CRL |= (((GPIO_Mode_AIN) & 0xF) << (1 << 2));

        /* Configure ADC sample time. */
        ADC1->SMPR2 &= (uint32_t)~(0x7 << (3 * ADC_STM32_CHN_PB1));
        ADC1->SMPR2 |= (ADC_SMP << (3 * ADC_STM32_CHN_PB1));

        break;
    }

    /* Configure ADC channel rank (we will always use rank 1). */
    ADC1->SQR3 &= (uint32_t)~(0x1F);
    ADC1->SQR3 |= channel;

    /* Enable ADC. */
    ADC1->CR2 |= ADC_CR2_ADON;

} /* adc_stm32_channel_select */

/*
 * adc_stm32_channel_unselect
 * @channel: Channel needed to be un-select.
 * This function will un-select an ADC channel.
 */
void adc_stm32_channel_unselect(uint32_t channel)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(channel);

    /* Disable ADC. */
    ADC1->CR2 &= (uint32_t)~(ADC_CR2_ADON);

} /* adc_stm32_channel_unselect */

/*
 * adc_stm32_read
 * @return: Returns the ADC reading.
 * This function will take a reading from the given ADC channel.
 */
uint32_t adc_stm32_read(void)
{
    /* Start ADC conversion. */
    ADC1->CR2 |= (ADC_CR2_EXTTRIG | ADC_CR2_SWSTART);

    /* Wait for ADC conversion to complete. */
    while (!(ADC1->SR & ADC_SR_EOC))
    {
        ;
    }

    /* Return the ADC value. */
    return ((uint32_t)ADC1->DR);

} /* adc_stm32_read */
#endif /* CONFIG_ADC */
