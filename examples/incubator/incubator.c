/*
 * incubator.c
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
#include <string.h>
#include <adc.h>
#include <serial.h>
#include <stdio.h>
#include <sys_info.h>
#include <avr/eeprom.h>
#include <stdlib.h>

/* Function prototypes. */
static void control_task(void *);
static void update_task(void *);

/* Control task definitions. */
#define CONTROL_STACK_SIZE          1024
static TASK control_task_cb;
static uint8_t control_task_stack[CONTROL_STACK_SIZE];

/* Update task definitions. */
#define UPDATE_STACK_SIZE           1024
static TASK update_task_cb;
static uint8_t update_task_stack[UPDATE_STACK_SIZE];

/* Default PWM value. */
#define TEMP_SET_INIT               3750        /* 38.0 centigrade. */
#define TEMP_SET_MAX                4500        /* 45.0 centigrade. */
#define TEMP_SET_MIN                3000        /* 30.0 centigrade. */
#define TEMP_HYSTERESIS             50          /* +- temperature hysteresis */
static uint32_t temp_set = TEMP_SET_INIT;
static uint8_t show_task_stats = FALSE;

static void control_task(void *argv)
{
    uint32_t reading;
    INT_LVL interrupt_level;
    uint8_t heater_on = FALSE;

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Turn off all LEDs. */
    PORTC ^= (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);

    while(1)
    {
        /* Toggle progress LED. */
        PORTC ^= (1 << 5);

        /* Read 1000 samples. */
        reading = adc_read_average(1000);

        /* Convert sample into temperature. */
        reading = (reading * 10000) / 1024;

        /* Print set temperature */
        printf("Set Temperature: %lu.%02luC,", (temp_set / 100), (temp_set % 100));

        /* Print current temperature */
        printf("Current Temperature: %lu.%02luC\r\n", (reading / 100), (reading % 100));

        /* If we need to show task statistics. */
        if (show_task_stats)
        {
            /* Print system usage. */
            util_print_sys_info();
            usage_reset();
        }

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* If we have an over temperature condition. */
        if (reading > (temp_set + TEMP_HYSTERESIS))
        {
            PORTC |= (1 << 2);
        }
        else
        {
            PORTC &= (uint8_t)~(1 << 2);
        }

        /* If we have an under temperature condition. */
        if (reading < (temp_set - TEMP_HYSTERESIS))
        {
            PORTC |= (1 << 4);
        }
        else
        {
            PORTC &= (uint8_t)~(1 << 4);
        }

        /* If desired temperate has been reached. */
        if ((reading >= (temp_set - TEMP_HYSTERESIS)) && (reading <= (temp_set + TEMP_HYSTERESIS)))
        {
            PORTC |= (1 << 3);
        }
        else
        {
            PORTC &= (uint8_t)~(1 << 3);
        }

        /* If heater is on. */
        if (heater_on)
        {
            /* Power off heater when we have reached the upper limit. */
            if (reading >= (temp_set + TEMP_HYSTERESIS))
            {
                /* Turn off heater. */
                PORTA &= (uint8_t)~(1 << 7);

                /* Heater is now off. */
                heater_on = FALSE;
            }
        }
        else
        {
            /* Turn off when we are about to cross lower limit. */
            if (reading <= (temp_set - TEMP_HYSTERESIS))
            {
                /* Turn on heater. */
                PORTA |= (1 << 7);

                /* Heater is now on. */
                heater_on = TRUE;
            }
        }

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Sleep till next reading. */
        sleep_ms(5000);
    }
}

static void update_task(void *argv)
{
    int first_part, second_part;
    uint32_t  new_temp;
    INT_LVL interrupt_level;

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Read temperature from EEPROM. */
    new_temp = eeprom_read_dword(0);

    /* If we have a valid temperature. */
    if ((new_temp <= TEMP_SET_MAX) && (new_temp >= TEMP_SET_MIN))
    {
        /* Update new temperature. */
        temp_set = new_temp;
    }

    while (1)
    {
        /* Read a line. */
        if (fscanf(stdin, "%02d.%02d", &first_part, &second_part) == 2)
        {
            /* Calculate the new temperate. */
            new_temp = first_part * 100 + second_part;

            /* If we need to toggle task statistics. */
            if (new_temp == 0)
            {
                /* Toggle task statistics. */
                show_task_stats ^= TRUE;
            }
            else if ((new_temp > TEMP_SET_MAX) || (new_temp < TEMP_SET_MIN))
            {
                printf("Invalid temperature %d.%02d\r\n", first_part, second_part);
                printf("Must be between %d.%02d and %d.%02d\r\n", TEMP_SET_MAX / 100, TEMP_SET_MAX % 100, TEMP_SET_MIN / 100, TEMP_SET_MIN % 100);
            }
            else
            {
                /* Get system interrupt level. */
                interrupt_level = GET_INTERRUPT_LEVEL();

                /* Disable global interrupts. */
                DISABLE_INTERRUPTS();

                /* Update new temperature. */
                temp_set = new_temp;

                /* Restore old interrupt level. */
                SET_INTERRUPT_LEVEL(interrupt_level);

                /* Print updated temperature. */
                printf("New temperature set at %ld.%02ld\r\n", temp_set / 100, temp_set % 100);

                /* Update this temperature in EEPROM. */
                eeprom_write_dword(0, temp_set);
            }
        }
    }
}

int main(void)
{
    uint32_t saved_temp;

    /* Initialize scheduler. */
    scheduler_init();

#ifdef CONFIG_SERIAL
#ifdef CONFIG_FS
    /* Initialize file system. */
    fs_init();
#endif /* CONFIG_FS */

    /* Initialize serial. */
    serial_init();
#endif /* CONFIG_SERIAL */

    /* Configure indicator LEDs. */
    DDRC |= (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);

    /* Turn on all LEDs until we have a system initialization. */
    PORTC |= (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);

    /* Configure PA7 as output. */
    DDRA = (1 << 7);

    /* Initialize ADC. */
    adc_init();

    /* Select ADC channel. */
    adc_channel_select(4);

    /* Read temperature from EEPROM. */
    saved_temp = eeprom_read_dword(0);

    /* If we have a valid temperature. */
    if ((saved_temp <= TEMP_SET_MAX) && (saved_temp >= TEMP_SET_MIN))
    {
        /* Update new temperature. */
        temp_set = saved_temp;
    }

    /* Create a task for control. */
    task_create(&control_task_cb, P_STR("CONTROL"), control_task_stack, CONTROL_STACK_SIZE, &control_task, (void *)(NULL), 0);
    scheduler_task_add(&control_task_cb, 5);

    /* Create a task for temperature update. */
    task_create(&update_task_cb, P_STR("UPDATE"), update_task_stack, UPDATE_STACK_SIZE, &update_task, (void *)(NULL), 0);
    scheduler_task_add(&update_task_cb, 6);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
