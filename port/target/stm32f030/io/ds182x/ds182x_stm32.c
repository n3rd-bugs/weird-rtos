/*
 * ds182x_stm32.c
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
#include <ds182x_stm32.h>

#ifdef GPIO_DS182X
/* Internal function prototypes. */
void ds182x_stm32_pin_init(ONE_WIRE *);
void ds182x_stm32_set_pin_mode(ONE_WIRE *, uint8_t);
uint8_t ds182x_stm32_get_pin_state(ONE_WIRE *);
void ds182x_stm32_set_pin_state(ONE_WIRE *, uint8_t);

DS182X_STM32 ds182x_stm32 =
{
    /* Hook-up DS182X bus. */
    .bus =
    {
        .onewire =
        {
            .pin_init = &ds182x_stm32_pin_init,
            .set_pin_mode = &ds182x_stm32_set_pin_mode,
            .get_pin_state = &ds182x_stm32_get_pin_state,
            .set_pin_state = &ds182x_stm32_set_pin_state,
        },
    },

    .port = (GPIO_TypeDef *)DS182X_STM32_DATA_PORT,
    .pin = DS182X_STM32_DATA_PIN,
};

/*
 * ds182x_stm32_init
 * This function will initialize an DS182X sensors for this platform.
 */
void ds182x_stm32_init()
{
    /* Register DS182X bus. */
    ds182x_register(&ds182x_stm32.bus);

} /* ds182x_stm32_init */

/*
 * ds182x_stm32_pin_init
 * @bus: DS182X bus for which data pin is needed to be initialized.
 * This function will initialize PIN for DS182X sensor.
 */
void ds182x_stm32_pin_init(ONE_WIRE *bus)
{
    DS182X_STM32 *stm_bus = (DS182X_STM32 *)bus;

    /* Enable clock for required GPIO port. */
    switch ((uint32_t)(stm_bus->port))
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

    /* Configure the PIN as GPIO input. */
    stm_bus->port->MODER &= (uint32_t)~(0x3 << (stm_bus->pin * 2));

    /* Use open drain. */
    stm_bus->port->OTYPER &= (uint16_t)~(0x1 << stm_bus->pin);
    stm_bus->port->OTYPER |= (uint16_t)(0x1 << stm_bus->pin);

    /* Disable pull-up/down for this pin. */
    stm_bus->port->PUPDR &= (uint32_t)~(0x3 << (stm_bus->pin * 2));

} /* ds182x_stm32_pin_init */

/*
 * ds182x_stm32_set_pin_mode
 * @bus: DS182X bus for which data pin is needed to be updated.
 * @mode: Pin mode needed to be set,
 *  TRUE if needed to be set to output,
 *  FLASE if needed to be set as input.
 * This function will update the PIN mode for DS182X sensor.
 */
void ds182x_stm32_set_pin_mode(ONE_WIRE *bus, uint8_t mode)
{
    DS182X_STM32 *stm_bus = (DS182X_STM32 *)bus;

    /* If needed to be set as output. */
    if (mode == TRUE)
    {
        /* Configure the PIN as GPIO output. */
        stm_bus->port->MODER &= (uint32_t)~(0x3 << (stm_bus->pin * 2));
        stm_bus->port->MODER |= (uint32_t)(0x1 << (stm_bus->pin * 2));
    }
    else
    {

        /* Configure the PIN as GPIO input. */
        stm_bus->port->MODER &= (uint32_t)~(0x3 << (stm_bus->pin * 2));
    }
} /* ds182x_stm32_set_pin_mode */

/*
 * ds182x_stm32_get_pin_state
 * @bus: DS182X bus for which data pin state is needed.
 * @return: Pin state will be return here,
 *  TRUE if pin is high,
 *  FLASE if pin is low.
 * This function will return the pin state.
 */
uint8_t ds182x_stm32_get_pin_state(ONE_WIRE *bus)
{
    DS182X_STM32 *stm_bus = (DS182X_STM32 *)bus;

    /* Return the pin state. */
    return ((stm_bus->port->IDR & (1 << stm_bus->pin)) != 0);

} /* ds182x_stm32_get_pin_state */

/*
 * ds182x_stm32_set_pin_state
 * @bus: DS182X bus for which data pin state is needed to be set.
 * @state: Pin state to be set,
 *  TRUE if pin is needed to be set high,
 *  FLASE if pin is needed to be set low.
 * This function will return the pin state.
 */
void ds182x_stm32_set_pin_state(ONE_WIRE *bus, uint8_t state)
{
    DS182X_STM32 *stm_bus = (DS182X_STM32 *)bus;

    if (state == TRUE)
    {
        /* Set the data pin. */
        stm_bus->port->BSRR |= (uint32_t)(1 << stm_bus->pin);
    }
    else
    {
        /* Reset the data pin. */
        stm_bus->port->BSRR |= (uint32_t)(1 << (stm_bus->pin + 16));
    }

} /* ds182x_stm32_set_pin_state */

#endif /* GPIO_DS182X */
