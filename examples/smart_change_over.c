/*
 * smart_change_over.c
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include <string.h>
#include <fs.h>
#include <net.h>
#include <net_udp.h>
#include <sys_info.h>
#include <weird_view_server.h>
#include <adc.h>
#include <math.h>
#include <lcd_an.h>
#include <serial.h>

/* Definitions to communicate with other side. */
#define DEVICE_NAME         "Smart Change Over"

#define ADC_SAMPLES         50

/* Function prototypes. */
int32_t weird_view_demo_task_stats(uint16_t, FS_BUFFER *);
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
#define CONTROL_TASK_STACK_SIZE         96
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
#define ADC_CHN_MAIN            (1)
#define ADC_CHN_GENERATOR       (3)

#define VOLTAGE_THRESHOLD       (15000)
#define POWER_ON_DELAY          (500)
#define LED_TOGGLE_DELAY        (250)
#define STATE_DELAY             (50)
#define DEBOUNCE_DELAY          (20)
#define KEY_GEN_OFF_DELAY       (750)
#define GENERATOR_SELF_DELAY    (1500)
#define GENERATOR_SELF_DEL_INC  (750)
#define GENERATOR_ON_DELAY      (10000)
#define GENERATOR_SELF_RETRY    (3)
#define SWITCH_DELAY            (5000)
#define ENABLE_WDT              TRUE
#define ENABLE_LOG              FALSE

/* IO configurations. */
#define BOARD_REV               1
#if (BOARD_REV == 1)
#define DDR_GENON_IND           DDRC
#define PORT_GENON_IND          PORTC
#define PIN_GENON_IND           4

#define DDR_MAINON_IND          DDRC
#define PORT_MAINON_IND         PORTC
#define PIN_MAINON_IND          3

#define DDR_SELFON_IND          DDRC
#define PORT_SELFON_IND         PORTC
#define PIN_SELFON_IND          5

#define DDR_SAMPLE_IND          DDRC
#define PORT_SAMPLE_IND         PORTC
#define PIN_SAMPLE_IND          2

#define DDR_CONNECTED           DDRB
#define PORT_CONNECTED          PORTB
#define PIN_CONNECTED           0

#define DDR_GENPWR_ON           DDRB
#define PORT_GENPWR_ON          PORTB
#define PIN_GENPWR_ON           1

#define DDR_GENSELF_ON          DDRB
#define PORT_GENSELF_ON         PORTB
#define PIN_GENSELF_ON          2

#define DDR_CHANGE_OVER         DDRD
#define PORT_CHANGE_OVER        PORTD
#define PIN_CHANGE_OVER         3

#define DDR_AUTO_SEL            DDRA
#define PORT_AUTO_SEL           PORTA
#define IN_AUTO_SEL             PINA
#define PIN_AUTO_SEL            6
#elif (BOARD_REV == 2)
#define DDR_GENON_IND           DDRC
#define PORT_GENON_IND          PORTC
#define PIN_GENON_IND           3

#define DDR_MAINON_IND          DDRC
#define PORT_MAINON_IND         PORTC
#define PIN_MAINON_IND          4

#define DDR_SELFON_IND          DDRC
#define PORT_SELFON_IND         PORTC
#define PIN_SELFON_IND          5

#define DDR_CONNECTED           DDRB
#define PORT_CONNECTED          PORTB
#define PIN_CONNECTED           0

#define DDR_GENPWR_ON           DDRB
#define PORT_GENPWR_ON          PORTB
#define PIN_GENPWR_ON           1

#define DDR_GENSELF_ON          DDRB
#define PORT_GENSELF_ON         PORTB
#define PIN_GENSELF_ON          2

#define DDR_CHANGE_OVER         DDRD
#define PORT_CHANGE_OVER        PORTD
#define PIN_CHANGE_OVER         3

#define DDR_AUTO_SEL            DDRA
#define PORT_AUTO_SEL           PORTA
#define IN_AUTO_SEL             PINA
#define PIN_AUTO_SEL            6
#endif

static volatile uint32_t main_volt = 0;
static volatile uint32_t generator_volt = 0;
static uint8_t current_channel = 0;
static uint8_t auto_start = FALSE;

/* ADC APIs. */
void adc_data_callback(uint32_t);
void adc_sample_process(void *, int32_t);

void generator_self()
{
    uint32_t interrupt_level, i, loop;
    uint8_t main_on, generator_on;

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Turn off the self LED. */
    PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Try to self the generator. */
    for (loop = 0, main_on = FALSE, generator_on = FALSE;
         ((generator_on == FALSE) &&
          (main_on == FALSE) &&
          (loop < GENERATOR_SELF_RETRY));
         loop++)
    {
        /* Self the generator. */
#if ENABLE_LOG
        printf("SELF %d\r\n", loop);
#endif

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Self the generator. */
        PORT_SELFON_IND |= (1 << PIN_SELFON_IND);
        PORT_GENSELF_ON |= (1 << PIN_GENSELF_ON);

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Wait before releasing the self. */
        sleep_ms(GENERATOR_SELF_DELAY + (GENERATOR_SELF_DEL_INC * loop));

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Release the self. */
        PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
        PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Wait after selfing the generator. */
        for (i = 0; i < (GENERATOR_ON_DELAY / STATE_DELAY); i++)
        {
            /* Get system interrupt level. */
            interrupt_level = GET_INTERRUPT_LEVEL();

            /* Disable global interrupts. */
            DISABLE_INTERRUPTS();

            /* If main has crossed the threshold. */
            if (main_volt > VOLTAGE_THRESHOLD)
            {
                /* Main detected. */
                PORT_MAINON_IND |= (1 << PIN_MAINON_IND);
                main_on = TRUE;
            }
            else
            {
                /* Main is no longer on. */
                PORT_MAINON_IND  &= (uint8_t)(~(1 << PIN_MAINON_IND));
                main_on = FALSE;
            }

            /* If generator has crossed the threshold. */
            if (generator_volt > VOLTAGE_THRESHOLD)
            {
                /* Generator detected. */
                PORT_GENON_IND |= (1 << PIN_GENON_IND);
                generator_on = TRUE;
            }
            else
            {
                /* Generator is no longer on. */
                PORT_GENON_IND  &= (uint8_t)(~(1 << PIN_GENON_IND));
                generator_on = FALSE;
            }

            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);

            /* Sleep for some time. */
            sleep_ms(STATE_DELAY);
        }
    }
} /* generator_self */

/*
 * toggle_auto_start
 * @auto_start: Auto start state.
 * @force_off: If we need to forcefully stop the auto start.
 * This function will toggle the auto start state.
 */
void toggle_auto_start(uint8_t *auto_start, uint8_t force_off)
{
    /* If we need to forcefully stop the auto start. */
    if (force_off == TRUE)
    {
        /* Disable auto start. */
        *auto_start = FALSE;
    }
    else
    {
        /* Toggle the auto start. */
        (*auto_start) ^= 0x01;
    }

    /* If we need to auto start the generator. */
    if (*auto_start == TRUE)
    {
        /* We will be auto starting the generator. */
        PORT_SELFON_IND |= (1 << PIN_SELFON_IND);
    }
    else
    {
        /* Generator will not be auto started. */
        PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
    }

} /* toggle_auto_start */

/*
 * control_entry
 * @argv: Task argument.
 * This is main entry function for smart switch control task.
 */
void control_entry(void *argv)
{
    uint32_t interrupt_level, switch_count = 0;
    uint32_t gen_off_count, change_over_count = 0;
#if ENABLE_LOG
    uint32_t gen_volt_tmp, main_volt_tmp;
#endif
    uint8_t main_on, generator_on;
    uint8_t generator_selfed = FALSE;
    uint8_t generator_switched_on = FALSE;
    uint8_t supply_gen = FALSE;
    uint8_t do_gen_off = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

#ifdef CONFIG_LCD_AN
    /* Initialize LCD. */
    lcd_an_init();
#endif

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Turn off all the indicators. */
    PORT_GENON_IND &= (uint8_t)(~(1 << PIN_GENON_IND));
    PORT_MAINON_IND &= (uint8_t)(~(1 << PIN_MAINON_IND));
    PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));

    /* Turn off generator power and self. */
    PORT_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
    PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));

    /* Turn off the change over. */
    PORT_CHANGE_OVER &= (uint8_t)(~(1 << PIN_CHANGE_OVER));

    /* TURN on the connected LED. */
    PORT_CONNECTED |= (1 << PIN_CONNECTED);

    /* Configure the button as input. */
    PORT_AUTO_SEL &= (uint8_t)(~(1 << PIN_AUTO_SEL));
    DDR_AUTO_SEL &= (uint8_t)(~(1 << PIN_AUTO_SEL));

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

#if ENABLE_LOG
    printf("CTRL Task\r\n");
#endif

    /* Wait for system to stabilize. */
    sleep_ms(POWER_ON_DELAY);

    while(1)
    {
        /* Wait for ADC to take new readings. */
        sleep_ms(STATE_DELAY);

        /* Check if button is pressed. */
        if (!(IN_AUTO_SEL & (1 << PIN_AUTO_SEL)))
        {
            /* Wait for sometime. */
            sleep_ms(DEBOUNCE_DELAY);

            /* Is button is still pressed. */
            if (!(IN_AUTO_SEL & (1 << PIN_AUTO_SEL)))
            {
                /* Toggle the auto start. */
                toggle_auto_start(&auto_start, FALSE);
            }
        }

        /* Reset the generator off count. */
        gen_off_count = 0;

        /* Wait for key to be released. */
        while (!(IN_AUTO_SEL & (1 << PIN_AUTO_SEL)))
        {
            /* Increment the generator off count. */
            gen_off_count ++;

            /* See if user wants to turn off the generator. */
            if (gen_off_count > (KEY_GEN_OFF_DELAY / DEBOUNCE_DELAY))
            {
                /* Toggle back the auto start. */
                toggle_auto_start(&auto_start, TRUE);

                /* Lets turn off the generator. */
                do_gen_off = TRUE;

                /* Reset switch count. */
                gen_off_count = 0;

                /* Again wait for key to be released. */
                while (!(IN_AUTO_SEL & (1 << PIN_AUTO_SEL)))
                {
                    if (gen_off_count > (LED_TOGGLE_DELAY / DEBOUNCE_DELAY))
                    {
                        gen_off_count = 0;

                        /* Toggle LED to show generator will be turned off. */
                        PORT_SELFON_IND ^= (1 << PIN_SELFON_IND);
                    }
                    else
                    {
                        gen_off_count ++;
                    }

                    sleep_ms(DEBOUNCE_DELAY);
                }

                break;
            }

            /* Sleep for de-bounce of the key. */
            sleep_ms(DEBOUNCE_DELAY);
        }

        /* If we need to auto start the generator. */
        if (auto_start == TRUE)
        {
            /* We will be auto starting the generator. */
            PORT_SELFON_IND |= (1 << PIN_SELFON_IND);
        }
        else
        {
            /* Generator will not be auto started. */
            PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
        }

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* If main has crossed the threshold. */
#if ENABLE_LOG
        main_volt_tmp = main_volt;
#endif
        if (main_volt > VOLTAGE_THRESHOLD)
        {
            /* Main detected. */
            PORT_MAINON_IND |= (1 << PIN_MAINON_IND);
            main_on = TRUE;
        }
        else
        {
            /* Main is no longer on. */
            PORT_MAINON_IND  &= (uint8_t)(~(1 << PIN_MAINON_IND));
            main_on = FALSE;
        }

        /* If generator has crossed the threshold. */
#if ENABLE_LOG
        gen_volt_tmp = generator_volt;
#endif
        if (generator_volt > VOLTAGE_THRESHOLD)
        {
            /* Generator detected. */
            PORT_GENON_IND |= (1 << PIN_GENON_IND);
            generator_on = TRUE;
        }
        else
        {
            /* Generator is no longer on. */
            PORT_GENON_IND  &= (uint8_t)(~(1 << PIN_GENON_IND));
            generator_on = FALSE;
        }

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

#if ENABLE_LOG
        /* Print main voltage. */
        printf("M - %d G - %d\r\n", main_volt_tmp, gen_volt_tmp);
#endif

        /* Check if generator is no longer on. */
        if (generator_on == FALSE)
        {
            /* If we were on generator. */
            if (supply_gen == TRUE)
            {
                /* Increment change over count. */
                change_over_count ++;

                /* If we have waited long enough to turn off the switch over. */
                if (change_over_count >= (SWITCH_DELAY / STATE_DELAY))
                {
                    /* Get system interrupt level. */
                    interrupt_level = GET_INTERRUPT_LEVEL();

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    /* Toggle the change over. */
#if ENABLE_LOG
                    printf("SW->OFF\r\n");
#endif
                    PORT_CHANGE_OVER &= ((uint8_t)~(1 << PIN_CHANGE_OVER));

                    /* Supply is no longer on the generator. */
                    supply_gen = FALSE;

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);
                }
            }
        }
        else
        {
            /* Reset the change over count. */
            change_over_count = 0;
        }

        /* Check if main is on or we need to turn off the generator. */
        if ((main_on == TRUE) || (do_gen_off == TRUE))
        {
            /* If generator is still on. */
            if ((generator_on == TRUE) || (generator_switched_on == TRUE))
            {
                /* We have detected an update in state. */
                switch_count ++;

                /* If we have waited long enough to process this state or we
                 * need to turn off the generator forcefully. */
                if ((switch_count > (SWITCH_DELAY / STATE_DELAY)) || (do_gen_off == TRUE))
                {
                    /* Turn off the generator. */
#if ENABLE_LOG
                    printf("GEN->OFF\r\n");
#endif
                    PORT_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));

                    /* Generator is no longer on and clear the self-ed flag. */
                    generator_switched_on = FALSE;
                    generator_selfed = FALSE;

                    /* Reset the change count. */
                    switch_count = 0;
                }
            }
            else
            {
                /* Reset the change count. */
                switch_count = 0;
            }

            /* Clear the generator off flag. */
            do_gen_off = FALSE;
        }

        else
        {
            /* If generator is not turned on. */
            if (generator_switched_on == FALSE)
            {
                /* We have detected an update in state. */
                switch_count ++;

                /* If we have waited long enough to process this state. */
                if (switch_count > (SWITCH_DELAY / STATE_DELAY))
                {
                    /* Turn-on generator. */
#if ENABLE_LOG
                    printf("GEN->ON\r\n");
#endif
                    PORT_GENPWR_ON |= (1 << PIN_GENPWR_ON);

                    /* Generator is turned on. */
                    generator_switched_on = TRUE;

                    /* Reset the change count. */
                    switch_count = 0;
                }
            }
            else
            {
                /* Reset the change count. */
                switch_count = 0;
            }

            /* If we have not yet self-ed the generator and generator is
             * turned on. */
            if ((auto_start == TRUE) && (generator_switched_on == TRUE) && (generator_selfed == FALSE))
            {
                /* Get system interrupt level. */
                interrupt_level = GET_INTERRUPT_LEVEL();

                /* Disable global interrupts. */
                DISABLE_INTERRUPTS();

                /* Turn off the self LED. */
                PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));

                /* Turn off the change over. */
#if ENABLE_LOG
                printf("SW->OFF\r\n");
#endif
                PORT_CHANGE_OVER &= ((uint8_t)~(1 << PIN_CHANGE_OVER));
                supply_gen = FALSE;

                /* Restore old interrupt level. */
                SET_INTERRUPT_LEVEL(interrupt_level);

                /* Self the generator. */
                generator_self();

                /* We have self-ed the generator. */
                generator_selfed = TRUE;
            }

            /* Check if generator is now turned on. */
            if ((auto_start == TRUE) && (generator_on == TRUE) && (supply_gen ==  FALSE))
            {
                /* Get system interrupt level. */
                interrupt_level = GET_INTERRUPT_LEVEL();

                /* Disable global interrupts. */
                DISABLE_INTERRUPTS();

                /* Toggle the generator supply. */
#if ENABLE_LOG
                printf("SW->ON\r\n");
#endif
                PORT_CHANGE_OVER |= (1 << PIN_CHANGE_OVER);

                /* Supply is no longer on the generator. */
                supply_gen = TRUE;

                /* Restore old interrupt level. */
                SET_INTERRUPT_LEVEL(interrupt_level);
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
 * @status: For now unused.
 * This is callback to process an ADC sample. This will be called in the
 * context of networking condition task.
 */
void adc_sample_process(void *data, int32_t status)
{
    uint32_t i, v_int = 0;
    uint32_t interrupt_level;

    /* Remove some compiler warning. */
    UNUSED_PARAM(data);
    UNUSED_PARAM(status);

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

#ifdef PORT_SAMPLE_IND
        /* Sample was processed. */
        PORT_SAMPLE_IND ^= (1 << PIN_SAMPLE_IND);
#endif

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
    SOCKET_ADDRESS  socket_address;

    /* Configure and turn off indicators. */
    DDR_GENON_IND |= (1 << PIN_GENON_IND);
    PORT_GENON_IND &= (uint8_t)(~(1 << PIN_GENON_IND));
    DDR_SELFON_IND |= (1 << PIN_SELFON_IND);
    PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
    DDR_MAINON_IND |= (1 << PIN_MAINON_IND);
    PORT_MAINON_IND &= (uint8_t)(~(1 << PIN_MAINON_IND));
#ifdef PORT_SAMPLE_IND
    DDR_SAMPLE_IND |= (1 << PIN_SAMPLE_IND);
    PORT_SAMPLE_IND &= (uint8_t)(~(1 << PIN_SAMPLE_IND));
#endif

    /* Configure and turn off the generator self and power. */
    DDR_GENPWR_ON |= (1 << PIN_GENPWR_ON);
    PORT_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
    DDR_GENSELF_ON |= (1 << PIN_GENSELF_ON);
    PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));

    /* Configure and turn off the change over. */
    DDR_CHANGE_OVER |= (1 << PIN_CHANGE_OVER);
    PORT_CHANGE_OVER &= (uint8_t)(~(1 << PIN_CHANGE_OVER));

    /* Configure connected LED. */
    DDR_CONNECTED |= (1 << PIN_CONNECTED);
    PORT_CONNECTED &= (uint8_t)(~(1 << PIN_CONNECTED));

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

#ifdef CONFIG_SERIAL
    /* Initialize serial. */
    serial_init();
#endif

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
    adc_suspend.timeout_enabled = TRUE;

    /* Initialize ADC condition data. */
    adc_condition.lock = &adc_int_lock;
    adc_condition.unlock = &adc_int_unlock;
    adc_condition.data = NULL;

    /* Add a networking condition for to process ADC sample event. */
    net_condition_add(&adc_condition, &adc_suspend, &adc_sample_process, (void *)NULL);

    /* Initialize control task. */
    task_create(&control_cb, "CONTROL", control_stack, CONTROL_TASK_STACK_SIZE, &control_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&control_cb, 0);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
