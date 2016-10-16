/*
 * smart_switch_over.c
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <stdio.h>
#include <stdlib.h>
#include <os.h>
#include <string.h>
#include <fs.h>
#include <net.h>
#include <net_udp.h>
#include <sys_info.h>
#include <weird_view_server.h>
#include <adc.h>
#include <math.h>

/* Definitions to communicate with other side. */
#define DEVICE_NAME         "Smart Auto Generator"

#define ADC_SAMPLES        100

/* Function prototypes. */
int32_t weird_view_demo_task_stats(uint16_t, FS_BUFFER *);
int32_t weird_view_demo_switch_data(uint16_t, uint8_t *);
void weird_view_demo_switch_req(uint16_t, uint8_t);
int32_t weird_view_demo_analog_data(uint16_t, uint32_t *, uint32_t *, uint32_t *);

/* Weird view server definitions. */
WEIRD_VIEW_SERVER           weird_view;
WEIRD_VIEW_PLUGIN           weird_view_plugins[] =
{
        /* Analog plugin. */
        {
                .id         = 0x01,
                .name       = "Main Line Voltage",
                .data       = (void *)&weird_view_demo_analog_data,
                .request    = NULL,
                .type       = WV_PLUGIN_ANALOG
        },

        /* Analog plugin. */
        {
                .id         = 0x02,
                .name       = "Generator Voltage",
                .data       = (void *)&weird_view_demo_analog_data,
                .request    = NULL,
                .type       = WV_PLUGIN_ANALOG
        },

        /* Task statistics plugin. */
        {
                .id         = 0x03,
                .name       = "Task Statistics",
                .data       = (void *)&weird_view_demo_task_stats,
                .request    = NULL,
                .type       = WV_PLUGIN_LOG
        },
};

/* Control task definitions. */
#define CONTROL_TASK_STACK_SIZE        256
uint8_t control_stack[CONTROL_TASK_STACK_SIZE];
TASK    control_cb;
void control_entry(void *argv);

/* ADC configuration and data. */
#define ADC_PRESCALE            ((uint32_t)125)
#define ADC_WAVE_FREQ           ((uint32_t)100)
#define ADC_ATIMER_PRESCALE     ((uint32_t)64)
#define ADC_SAMPLE_PER_WAVE     ((uint32_t)PCLK_FREQ / (ADC_ATIMER_PRESCALE * ADC_PRESCALE * ADC_WAVE_FREQ))

#define ADC_CHANNEL_DELAY       (OS_TICKS_PER_SEC / 20)

static uint16_t adc_sample[ADC_SAMPLES];
static CONDITION adc_condition;
static SUSPEND adc_suspend;

/* Charge controller definitions. */
#define ADC_CHN_MAIN            (1)
#define ADC_CHN_GENERATOR       (3)

#define VOLTAGE_THRESHOLD       (15000)
#define POWER_ON_DELAY          (500)
#define STATE_DELAY             (250)
#define DEBOUNCE_DELAY          (120)
#define PIN_READ_DELAY          (10)
#define GENERATOR_SELF_DELAY    (1500)
#define GENERATOR_SELF_DEL_INC  (750)
#define GENERATOR_ON_DELAY      (10000)
#define GENERATOR_SELF_RETRY    (3)
#define ENABLE_WDT              TRUE

static volatile uint32_t main_volt = 0;
static volatile uint32_t generator_volt = 0;
static uint8_t  current_channel = 0;

/* ADC APIs. */
void adc_data_callback(uint32_t);
void adc_sample_process(void *);

/*
 * control_entry
 * @argv: Task argument.
 * This is main entry function for smart switch control task.
 */
void control_entry(void *argv)
{
    uint32_t interrupt_level;
    uint8_t loop, main_on, generator_on;
    uint8_t generator_selfed = FALSE;
    uint8_t generator_switched_on = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Turn off main and generator. */
    PORTC &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4)));
    PORTB &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2)));
    PORTD &= (uint8_t)(~((1 << 3) | (1 << 5) | (1 << 6) | (1 << 7)));

    /* Turn on the connected LED. */
    PORTB |= (1 << 0);

    printf("CTRL Task>\r\n");

    /* Wait for system to stabilize. */
    sleep_ms(POWER_ON_DELAY);

    while(1)
    {
        /* Wait for ADC to take new readings. */
        sleep_ms(STATE_DELAY);
        PORTC ^= (1 << 4);

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        if (main_volt > VOLTAGE_THRESHOLD)
        {
            /* Main detected. */
            PORTC |= (1 << 2);
            main_on = TRUE;
        }
        else
        {
            /* Main is no longer on. */
            PORTC  &= (uint8_t)(~(1 << 2));
            main_on = FALSE;
        }

        if (generator_volt > VOLTAGE_THRESHOLD)
        {
            /* Generator detected. */
            PORTC |= (1 << 3);
            generator_on = TRUE;
        }
        else
        {
            /* Generator is no longer on. */
            PORTC  &= (uint8_t)(~(1 << 3));
            generator_on = FALSE;
        }

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Check if main is on. */
        if (main_on == TRUE)
        {
            /* If generator is still on. */
            if ((generator_on == TRUE) ||
                (generator_switched_on == TRUE))
            {
                /* Turn off the generator. */
                printf("GEN->OFF\r\n");
                PORTB &= (uint8_t)(~(1 << 1));

                /* Generator is no longer on and clear the self-ed flag. */
                generator_switched_on = FALSE;
                generator_selfed = FALSE;
            }
        }

        else
        {
            /* If generator is not turned on. */
            if (generator_switched_on == FALSE)
            {
                /* Turn-on generator. */
                printf("GEN->ON\r\n");
                PORTB |= (1 << 1);

                /* Generator is turned on. */
                generator_switched_on = TRUE;
            }

            /* If we have not yet self-ed the generator. */
            if (generator_selfed == FALSE)
            {
                /* Check if we don't need to turn on the generator. */
                PORTA &= (uint8_t)(~(1 << 6));
                DDRA &= (uint8_t)(~(1 << 6));
                sleep_ms(PIN_READ_DELAY);
                if (PINA & (1 << 6))
                {
                    continue;
                }

                /* Turn off the self LED. */
                PORTC &= (uint8_t)(~(1 << 4));

                /* Denounce sleep. */
                sleep_ms(DEBOUNCE_DELAY);

                /* Check again if we don't need to turn on the generator. */
                PORTA &= (uint8_t)(~(1 << 6));
                DDRA &= (uint8_t)(~(1 << 6));
                sleep_ms(PIN_READ_DELAY);
                if (PINA & (1 << 6))
                {
                    continue;
                }

                /* Try to self the generator. */
                for (loop = 0;
                     ((generator_on == FALSE) &&
                      (loop < GENERATOR_SELF_RETRY));
                     loop++)
                {
                    /* Self the generator. */
                    printf("SELF %d\r\n", loop);
                    PORTC |= (1 << 4);
                    PORTB |= (1 << 2);
                    sleep_ms(GENERATOR_SELF_DELAY + (GENERATOR_SELF_DEL_INC * loop));
                    PORTC &= (uint8_t)(~(1 << 4));
                    PORTB &= (uint8_t)(~(1 << 2));

                    /* Sleep for sometime before self-ing the generator
                     * again. */
                    sleep_ms(GENERATOR_ON_DELAY);

                    /* Get system interrupt level. */
                    interrupt_level = GET_INTERRUPT_LEVEL();

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    if (generator_volt > VOLTAGE_THRESHOLD)
                    {
                        /* Generator detected. */
                        generator_on = TRUE;
                    }
                    else
                    {
                        /* Generator is no longer on. */
                        generator_on = FALSE;
                    }

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);
                }

                /* We have self-ed the generator. */
                generator_selfed = TRUE;
            }
        }
    }

} /* control_entry */


/*
 * weird_view_demo_task_stats
 * @id: Plugin id.
 * @buffer: File system buffer in which reply will be populated.
 * This is callback function to populate the given buffer with task statistics.
 */
int32_t weird_view_demo_task_stats(uint16_t id, FS_BUFFER *buffer)
{
    int32_t status;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(id);

    /* Need to update the existing data. */
    status = fs_buffer_push(buffer, (uint8_t []){ WV_PLUGIN_LOG_UPDATE }, sizeof(uint8_t), 0);

    if (status == SUCCESS)
    {
        /* Push system information in the buffer. */
        status = util_print_sys_info_buffer(buffer);
    }

    /* Always return success. */
    return (SUCCESS);

} /* weird_view_demo_task_stats */

/*
 * weird_view_demo_analog_data
 * @id: Plugin id.
 * @value: Current analog value will be returned here.
 * @value_div: Value divider will be returned here.
 * @max_value: Maximum display value will be returned here.
 * This is callback function to populate the given buffer with state of this
 * switch.
 */
int32_t weird_view_demo_analog_data(uint16_t id, uint32_t *value, uint32_t *value_div, uint32_t *max_value)
{
    uint32_t interrupt_level;

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    switch (id)
    {
    case 0x01:
        /* (x * 2 * ((10k + 100k) / 10k)) */
        *value = main_volt;
        *value_div = 1;
        *max_value = 1024ul * ADC_SAMPLES;
        break;

    case 0x02:
        *value = generator_volt;
        *value_div = 1;
        *max_value = 1024ul * ADC_SAMPLES;
        break;
    }

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Always return success. */
    return (SUCCESS);

} /* weird_view_demo_analog_data */

/*
 * adc_data_callback
 * @data: ADC reading.
 * This is callback function for ADC conversion. This function will be called
 * in the context of a ISR.
 */
void adc_data_callback(uint32_t data)
{
    static int32_t n = 0;
    uint32_t interrupt_level;

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
        adc_atmega644_periodic_read_stop();

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
 * This is callback to process an ADC sample. This will be called in the
 * context of networking condition task.
 */
void adc_sample_process(void *data)
{
    uint32_t i, v_int = 0;
    uint32_t interrupt_level;

    /* Remove some compiler warning. */
    UNUSED_PARAM(data);

    /* Toggle PC5. */
    PORTC ^= (1 << 5);

#if (ENABLE_WDT == TRUE)
    /* Reset watch dog timer. */
    WDT_RESET();
#endif

    /* Were we waiting for ADC channel to stabilize. */
    if (adc_suspend.timeout != MAX_WAIT)
    {
        /* Stop the ADC timer. */
        adc_suspend.timeout = MAX_WAIT;

        /* Start periodic ADC conversion. */
        adc_atmega644_periodic_read_start(&adc_data_callback, (ADC_PRESCALE - 1));
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

        switch (current_channel)
        {
        case ADC_CHN_MAIN:

            /* Save the ADC reading. */
            main_volt = v_int;
            current_channel = ADC_CHN_GENERATOR;

            break;

        case ADC_CHN_GENERATOR:

            /* Save the ADC reading. */
            generator_volt = v_int;
            current_channel = ADC_CHN_MAIN;

            break;
        }

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Select required channel. */
        adc_channel_select(current_channel);

        /* Before actually starting the sampling, wait for channel to switch. */
        adc_suspend.timeout = (current_system_tick() + ADC_CHANNEL_DELAY);
    }

} /* adc_sample_process */

/* Main entry function for AVR. */
int main(void)
{
    SOCKET_ADDRESS  socket_address;

    /* Configure PC2, PC3, PC4, PC5. */
    PORTC &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)));
    DDRC |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
    PORTC &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)));

    /* Configure PB1, PB2, PB3. */
    PORTB &= (uint8_t)(~((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4)));
    DDRB |= ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
    PORTB &= (uint8_t)(~((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4)));

    /* Configure PD3, PD5, PD6, PD7. */
    PORTD &= (uint8_t)(~((1 << 3) | (1 << 5) | (1 << 6) | (1 << 7)));
    DDRD |= ((1 << 3) | (1 << 5) | (1 << 6) | (1 << 7));
    PORTD &= (uint8_t)(~((1 << 3) | (1 << 5) | (1 << 6) | (1 << 7)));

#if (ENABLE_WDT == TRUE)
    /* Reset watchdog timer. */
    WDT_RESET();

    /* Start timed sequence */
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    /* Set new prescaler (time-out). */
    WDTCSR = (1 << WDE) | (1 << WDP3) | (1 << WDP0);
#endif

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize networking stack. */
    net_init();

    /* Populate the socket structure. */
    socket_address.foreign_ip = IPV4_ADDR_UNSPEC;
    socket_address.foreign_port = NET_PORT_UNSPEC;
    socket_address.local_ip = IPV4_ADDR_UNSPEC;
    socket_address.local_port = 11000;

    /* Initialize ADC. */
    adc_init();

    /* Select ADC channel. */
    current_channel = ADC_CHN_MAIN;
    adc_channel_select(current_channel);

    /* Initialize a weird view server instance. */
    weird_view_server_init(&weird_view, &socket_address, DEVICE_NAME, weird_view_plugins, sizeof(weird_view_plugins)/sizeof(WEIRD_VIEW_PLUGIN));

    /* Initialize condition data. */
    adc_suspend.timeout = (uint32_t)(current_system_tick() + ADC_CHANNEL_DELAY);
    adc_suspend.flags = SUSPEND_TIMER;

    /* Add a networking condition for to process ADC sample event. */
    net_condition_add(&adc_condition, &adc_suspend, &adc_sample_process, (void *)NULL);

    /* Initialize control task. */
    task_create(&control_cb, "CONTROL", control_stack, CONTROL_TASK_STACK_SIZE, &control_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&control_cb, TASK_APERIODIC, 0, 0);

    /* Run scheduler. */
    os_run();

    return (0);

}
