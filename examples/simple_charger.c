/*
 * simple_charger.c
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include <string.h>
#include <fs.h>
#include <adc.h>
#include <net.h>
#include <math.h>
#include <sys_info.h>
#include <serial.h>

#define ADC_SAMPLES                     100

/* Control task definitions. */
#define CONTROL_TASK_STACK_SIZE         512
uint8_t control_stack[CONTROL_TASK_STACK_SIZE];
TASK    control_cb;
void control_entry(void *argv);

/* ADC configuration and data. */
#define ADC_PRESCALE            ((uint32_t)125)
#define ADC_WAVE_FREQ           ((uint32_t)100)
#define ADC_ATIMER_PRESCALE     ((uint32_t)8)
#define ADC_SAMPLE_PER_WAVE     ((uint32_t)PCLK_FREQ / (ADC_ATIMER_PRESCALE * ADC_PRESCALE * ADC_WAVE_FREQ))

#define ADC_CHANNEL_DELAY       (SOFT_TICKS_PER_SEC / 5)

static uint16_t adc_sample[ADC_SAMPLES];
static CONDITION adc_condition;
static SUSPEND adc_suspend;

/* Charge controller definitions. */
#define ADC_CHN_BATTERY         (1)

#define BATTERY_LOW             43000
#define BATTERY_HIGH            53000
#define POWER_ON_DELAY          700
#define STATE_DELAY             500
#define INITIAL_CHARGE_DELAY    (SOFT_TICKS_PER_SEC * 5 * 60)
#define ENABLE_WDT              TRUE

static volatile uint32_t battery_volt = 0;
static volatile uint8_t charger_on = FALSE;

/* ADC APIs. */
void adc_data_callback(uint32_t);
void adc_sample_process(void *, int32_t);

/*
 * control_entry
 * @argv: Task argument.
 * This is main entry function for smart switch control task.
 */
void control_entry(void *argv)
{
    uint64_t charge_start = 0;
    uint32_t battery_level;
    INT_LVL interrupt_level;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Wait for system to stabilize. */
    sleep_ms(POWER_ON_DELAY);

    while(1)
    {
        /* Wait for ADC to take new readings. */
        sleep_ms(STATE_DELAY);

        /* Toggle PC4. */
        PORTC ^= (1 << 4);

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Save the battery level. */
        battery_level = battery_volt;

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        printf("ADC: %ld\r\n", battery_level);

        /* Check if we battery is dying out. */
        if (battery_level < BATTERY_LOW)
        {
            if (charger_on == FALSE)
            {
                /* Show that we have turned on the charger. */
                PORTC |= (1 << 2);

                /* Turn on the charger. */
                PORTD |= (1 << 3);

                /* Charger is now turned on. */
                charger_on = TRUE;
                charge_start = current_system_tick();
            }
        }

        /* Check if battery is fully charged. */
        else if (battery_level > BATTERY_HIGH)
        {
            /* If charger was on for more than the initial
             * on delay. */
            if ((charger_on == TRUE) &&
                ((current_system_tick() - charge_start) > INITIAL_CHARGE_DELAY))
            {
                /* Show that we have turned off the charger. */
                PORTC &= ((uint8_t)~(1 << 2));

                /* Turn off the charger. */
                PORTD &= (uint8_t)(~(1 << 3));

                /* Charger is now turned off. */
                charger_on = FALSE;
            }
        }
    }

} /* control_entry */

/*
 * adc_data_callback
 * @data: ADC reading.
 * This is callback function for ADC conversion. This function will be called
 * in the context of a ISR.
 */
void adc_data_callback(uint32_t data)
{
    static int32_t n = 0;
    INT_LVL interrupt_level;

    /* Put data on the ADC sample. */
    adc_sample[n] = (uint16_t)data;

    /* We have taken a sample. */
    n++;

    /* If we have required number of samples. */
    if (n == ADC_SAMPLES)
    {
        /* Reset the sample counter. */
        n = 0;

        /* Stop ADC sampling. */
        adc_avr_periodic_read_stop();

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Set the ping flag for ADC condition. */
        adc_condition.flags |= CONDITION_PING;

        /* Resume any tasks waiting for ADC condition. */
        resume_condition(&adc_condition, NULL, TRUE);

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
    }

} /* adc_data_callback */

/*
 * adc_sample_process
 * @data: For now unused.
 * @status: For now unused.
 * This is callback to process an ADC sample. This will be called in the
 * context of networking condition task.
 */
void adc_sample_process(void *data, int32_t status)
{
    uint32_t i, v_int = 0;
    INT_LVL interrupt_level;

    /* Remove some compiler warning. */
    UNUSED_PARAM(data);
    UNUSED_PARAM(status);

    /* Toggle PC5. */
    PORTC ^= (1 << 5);

#if (ENABLE_WDT == TRUE)
    /* Reset watch dog timer. */
    WDT_RESET();
#endif

    /* Were we waiting for ADC channel to stabilize. */
    if (adc_suspend.timeout_enabled != FALSE)
    {
        /* Stop the ADC timer. */
        adc_suspend.timeout_enabled = FALSE;

        /* Start periodic ADC conversion. */
        adc_avr_periodic_read_start(&adc_data_callback, (ADC_PRESCALE - 1));
    }

    else
    {
        /* Compute the sum of ADC sample. */
        for (i = 0; i < ADC_SAMPLES; i++)
        {
            v_int += adc_sample[i];
        }

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Save the ADC reading. */
        battery_volt = v_int;

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Restart the ADC readings. */
        adc_suspend.timeout = (current_system_tick());
        adc_suspend.timeout_enabled = TRUE;
    }

} /* adc_sample_process */

/*
 * adc_int_lock
 * @data: For now unused.
 * This will lock the ADC condition by disabling ADC interrupt.
 */
void adc_int_lock(void *data)
{
    UNUSED_PARAM(data);

    /* Disable ADC interrupt. */
    ADCSRA &= ((uint8_t)~(1 << ADIE));

} /* adc_int_lock */

/*
 * adc_int_unlock
 * @data: For now unused.
 * This will unlock the ADC condition by enabling ADC interrupt.
 */
void adc_int_unlock(void *data)
{
    UNUSED_PARAM(data);

    /* Enable ADC interrupt. */
    ADCSRA |= (1 << ADIE);

} /* adc_int_unlock */

/* Main entry function for AVR. */
int main(void)
{
#if (ENABLE_WDT == TRUE)
    /* Reset watchdog timer. */
    WDT_RESET();

    /* Start timed sequence */
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    /* Set new prescaler (time-out). */
    WDTCSR = (1 << WDE) | (1 << WDP3) | (1 << WDP0);
#endif

    /* Configure PC2, PC3, PC4, PC5. */
    PORTC &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)));
    DDRC |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
    PORTC &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)));

    /* Configure PD3 as output. */
    PORTD &= (uint8_t)(~(1 << 3));
    DDRD |= (1 << 3);
    PORTD &= (uint8_t)(~(1 << 3));

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

#ifdef IO_SERIAL
    /* Initialize serial. */
    serial_init();
#endif

    /* Initialize ADC. */
    adc_init();

    /* Initialize networking stack. */
    net_init();

    /* Select ADC channel. */
    adc_channel_select(ADC_CHN_BATTERY);

    /* Initialize condition data. */
    adc_suspend.timeout = (uint32_t)(current_system_tick() + ADC_CHANNEL_DELAY);
    adc_suspend.timeout_enabled = TRUE;
    adc_suspend.priority = NET_USER_PRIORITY;
    adc_suspend.status = SUCCESS;

    /* Initialize ADC condition data. */
    adc_condition.lock = &adc_int_lock;
    adc_condition.unlock = &adc_int_unlock;
    adc_condition.data = NULL;

    /* Add a networking condition for to process ADC sample event. */
    net_condition_add(&adc_condition, &adc_suspend, &adc_sample_process, (void *)NULL);

    /* Initialize control task. */
    task_create(&control_cb, P_STR("CONTROL"), control_stack, CONTROL_TASK_STACK_SIZE, &control_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&control_cb, 0);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
