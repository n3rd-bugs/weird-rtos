/*
 * dhtxx_stm32.c
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
#include <dhtxx_stm32.h>

#ifdef GPIO_DHT
/* Internal function prototypes. */
void dhtxx_stm32_pin_init(DHT_XX *);
void dhtxx_stm32_set_pin_mode(DHT_XX *, uint8_t);
uint8_t dhtxx_stm32_get_pin_state(DHT_XX *);
void dhtxx_stm32_set_pin_state(DHT_XX *, uint8_t);

DHT_XX_STM32 sht_stm32 =
{
    /* Hook-up DHT. */
    .dht =
    {
        .pin_init = &dhtxx_stm32_pin_init,
        .set_pin_mode = &dhtxx_stm32_set_pin_mode,
        .get_pin_state = &dhtxx_stm32_get_pin_state,
        .set_pin_state = &dhtxx_stm32_set_pin_state,
    },

    .port = (GPIO_TypeDef *)DHTXX_STM32_DATA_PORT,
    .pin = DHTXX_STM32_DATA_PIN,
};

/*
 * dhtxx_stm32_init
 * This function will initialize an DHTXX sensors for this platform.
 */
void dhtxx_stm32_init()
{
    /* Register DHT device. */
    dhtxx_register(&sht_stm32.dht);

} /* dhtxx_stm32_init */

/*
 * dhtxx_stm32_pin_init
 * @dht: DHT device for which data pin is needed to be initialized.
 * This function will initialize PIN for DHT sensor.
 */
void dhtxx_stm32_pin_init(DHT_XX *dht)
{
    DHT_XX_STM32 *stm_dht = (DHT_XX_STM32 *)dht;

    /* Enable clock for required GPIO port. */
    switch ((uint32_t)(stm_dht->port))
    {
    case ((uint32_t)GPIOA):
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
        break;
    case ((uint32_t)GPIOB):
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
        break;
    case ((uint32_t)GPIOC):
        RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
        break;
    case ((uint32_t)GPIOD):
        RCC->AHBENR |= RCC_AHBENR_GPIODEN;
        break;
    case ((uint32_t)GPIOE):
        RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
        break;
    case ((uint32_t)GPIOF):
        RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
        break;
    }

    /* Configure the PIN as GPIO output. */
    stm_dht->port->MODER &= (uint32_t)~(0x3 << (stm_dht->pin * 2));
    stm_dht->port->MODER |= (uint32_t)(0x1 << (stm_dht->pin * 2));

    /* Set the pin in idle condition. */
    stm_dht->port->ODR |= (uint16_t)(0x1 << stm_dht->pin);

    /* Enable pull-up for this pin. */
    stm_dht->port->PUPDR &= (uint32_t)~(0x3 << (stm_dht->pin * 2));
    stm_dht->port->PUPDR |= (uint32_t)(0x1 << (stm_dht->pin * 2));

} /* dhtxx_stm32_pin_init */

/*
 * dhtxx_stm32_set_pin_mode
 * @dht: DHT device for which data pin is needed to be updated.
 * @mode: Pin mode needed to be set,
 *  TRUE if needed to be set to output,
 *  FLASE if needed to be set as input.
 * This function will update the PIN mode for DHT sensor.
 */
void dhtxx_stm32_set_pin_mode(DHT_XX *dht, uint8_t mode)
{
    DHT_XX_STM32 *stm_dht = (DHT_XX_STM32 *)dht;

    /* If needed to be set as output. */
    if (mode == TRUE)
    {
        /* Configure the PIN as GPIO output. */
        stm_dht->port->MODER &= (uint32_t)~(0x3 << (stm_dht->pin * 2));
        stm_dht->port->MODER |= (uint32_t)(0x1 << (stm_dht->pin * 2));
    }
    else
    {

        /* Configure the PIN as GPIO input. */
        stm_dht->port->MODER &= (uint32_t)~(0x3 << (stm_dht->pin * 2));
    }
} /* dhtxx_stm32_set_pin_mode */

/*
 * dhtxx_stm32_get_pin_state
 * @dht: DHT device for which data pin state is needed.
 * @return: Pin state will be return here,
 *  TRUE if pin is high,
 *  FLASE if pin is low.
 * This function will return the pin state.
 */
uint8_t dhtxx_stm32_get_pin_state(DHT_XX *dht)
{
    DHT_XX_STM32 *stm_dht = (DHT_XX_STM32 *)dht;

    /* Return the pin state. */
    return ((stm_dht->port->IDR & (1 << stm_dht->pin)) != 0);

} /* dhtxx_stm32_get_pin_state */

/*
 * dhtxx_stm32_set_pin_state
 * @dht: DHT device for which data pin state is needed to be set.
 * @state: Pin state to be set,
 *  TRUE if pin is needed to be set high,
 *  FLASE if pin is needed to be set low.
 * This function will return the pin state.
 */
void dhtxx_stm32_set_pin_state(DHT_XX *dht, uint8_t state)
{
    DHT_XX_STM32 *stm_dht = (DHT_XX_STM32 *)dht;

    if (state == TRUE)
    {
        /* Set the data pin. */
        stm_dht->port->BSRR |= (uint32_t)(1 << stm_dht->pin);
    }
    else
    {
        /* Reset the data pin. */
        stm_dht->port->BSRR |= (uint32_t)(1 << (stm_dht->pin + 16));
    }

} /* dhtxx_stm32_set_pin_state */

#endif /* GPIO_DHT */
