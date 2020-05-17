/*
 * smart_change_over.c
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
#include <net.h>
#include <net_udp.h>
#include <sys_info.h>
#include <weird_view_server.h>
#include <adc.h>
#include <math.h>
#include <idle.h>
#include <avr/wdt.h>
#ifdef IO_LCD_AN
#include <lcd_an.h>
#endif /* IO_LCD_AN */
#ifdef IO_SERIAL
#include <serial.h>
#endif
#include <rtl.h>
#include <io.h>

/* Definitions to communicate with other side. */
#define DEVICE_NAME         "Smart Change Over"

/* Function prototypes. */
int32_t weird_view_demo_task_stats(uint16_t, FS_BUFFER_LIST *);
int32_t weird_view_demo_analog_data(uint16_t, uint32_t *, uint32_t *, uint32_t *);
static int32_t weird_view_demo_adc_sample(uint16_t, FS_BUFFER_LIST *);

/* Weird view server definitions. */
const char plgn_name_1[] P_STR_MEM = "Main Line Voltage";
const char plgn_name_2[] P_STR_MEM = "Generator Voltage";
const char plgn_name_3[] P_STR_MEM = "Task Statistics";
const char plgn_name_4[] P_STR_MEM = "Main Line Wave";
const char plgn_name_5[] P_STR_MEM = "Generator Wave";
WEIRD_VIEW_SERVER           weird_view;
WEIRD_VIEW_PLUGIN           weird_view_plugins[] =
{
        /* Analog plugin. */
        {
                .id         = 0x1,
                .name       = plgn_name_1,
                .data       = (void *)&weird_view_demo_analog_data,
                .request    = NULL,
                .type       = WV_PLUGIN_ANALOG
        },

        /* Analog plugin. */
        {
                .id         = 0x2,
                .name       = plgn_name_2,
                .data       = (void *)&weird_view_demo_analog_data,
                .request    = NULL,
                .type       = WV_PLUGIN_ANALOG
        },

        /* Task statistics plugin. */
        {
                .id         = 0x3,
                .name       = plgn_name_3,
                .data       = (void *)&weird_view_demo_task_stats,
                .request    = NULL,
                .type       = WV_PLUGIN_LOG
        },

        /* ADC sample plugin. */
        {
                .id         = 0x4,
                .name       = plgn_name_4,
                .data       = (void *)&weird_view_demo_adc_sample,
                .request    = NULL,
                .type       = WV_PLUGIN_WAVE
        },

        /* ADC sample plugin. */
        {
                .id         = 0x5,
                .name       = plgn_name_5,
                .data       = (void *)&weird_view_demo_adc_sample,
                .request    = NULL,
                .type       = WV_PLUGIN_WAVE
        },
};

/* Control task definitions. */
#define CONTROL_TASK_STACK_SIZE         512
uint8_t control_stack[CONTROL_TASK_STACK_SIZE];
TASK control_cb;
void control_entry(void *argv);

/* Log task definitions. */
#define LOG_TASK_STACK_SIZE             512
uint8_t log_stack[LOG_TASK_STACK_SIZE];
TASK log_cb;
void log_entry(void *argv);

/* Time at which last power failure occur. */
volatile uint32_t last_power_failure = 0;

/* ADC configuration and data. */
#define ADC_PRESCALE            ((uint32_t)125)
#define ADC_WAVE_FREQ           ((uint32_t)100)
#define ADC_ATIMER_PRESCALE     ((uint32_t)64)
#define ADC_SAMPLE_PER_WAVE     ((uint32_t)PCLK_FREQ / (ADC_ATIMER_PRESCALE * ADC_PRESCALE * ADC_WAVE_FREQ))
#define ADC_NUM_APPROX_SAMPLES  ((ADC_SAMPLE_PER_WAVE * 3) / 2)

/* Charge controller definitions. */
#define ADC_CHN_MAIN            (1)
#define ADC_CHN_GENERATOR       (3)

#define VOLTAGE_THRESHOLD       (300)
#define POWER_ON_DELAY          (500)
#define LED_TOGGLE_DELAY        (150)
#define LOG_DELAY               (1000)
#define STATE_DELAY             (100)
#define DEBOUNCE_DELAY          (100)
#define KEY_GEN_OFF_DELAY       (750)
#define GENERATOR_SELF_DELAY    (1500)
#define GENERATOR_SELF_DEL_INC  (750)
#define GENERATOR_ON_DELAY      (8000)
#define GENERATOR_SELF_RETRY    (3)
#define GENERATOR_RELAY_DELAY   (3000)
#define CHANGE_OVER_DELAY       (2000)
#define SWITCH_DELAY            (7000)
#define GENERATOR_OFF_DELAY     (8000)
#define ADC_CHANNEL_DELAY       (100)
#define ADC_MAX_WAVES           (20)
#define ADC_NUM_SAMPLES         (ADC_MAX_WAVES * ADC_SAMPLE_PER_WAVE)
#define ENABLE_WDT              TRUE
#define COMPUTE_AVG             FALSE
#define COMPUTE_APPROX          TRUE
#define DEBUG_WAVE              FALSE
#define ENABLE_COUTERMEASURE    TRUE
#define ENFORCE_ZERO_CROSSING   TRUE

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
#define IN_GENPWR_ON            PINB
#define PIN_GENPWR_ON           1

#define DDR_GENSELF_ON          DDRB
#define PORT_GENSELF_ON         PORTB
#define IN_GENSELF_ON           PINB
#define PIN_GENSELF_ON          2

#define DDR_GENERATOR_RELAY     DDRD
#define PORT_GENERATOR_RELAY    PORTD
#define IN_GENERATOR_RELAY      PIND
#define PIN_GENERATOR_RELAY     3

#define DDR_AUTO_SEL            DDRA
#define PORT_AUTO_SEL           PORTA
#define IN_AUTO_SEL             PINA
#define PIN_AUTO_SEL            6

#define DDR_CHANGE_OVER         DDRD
#define PORT_CHANGE_OVER        PORTD
#define IN_CHANGE_OVER          PIND
#define PIN_CHANGE_OVER         6
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
#define IN_GENPWR_ON            PINB
#define PIN_GENPWR_ON           1

#define DDR_GENSELF_ON          DDRB
#define PORT_GENSELF_ON         PORTB
#define IN_GENSELF_ON           PINB
#define PIN_GENSELF_ON          2

#define DDR_GENERATOR_RELAY         DDRD
#define PORT_GENERATOR_RELAY        PORTD
#define IN_GENERATOR_RELAY          PIND
#define PIN_GENERATOR_RELAY         3

#define DDR_AUTO_SEL            DDRA
#define PORT_AUTO_SEL           PORTA
#define IN_AUTO_SEL             PINA
#define PIN_AUTO_SEL            6
#endif

static volatile uint32_t main_volt = 0;
static volatile uint32_t generator_volt = 0;
static uint8_t current_channel = 0;
static uint8_t auto_start = FALSE;
#if (COMPUTE_APPROX == TRUE)
static uint8_t adc_got_wave = FALSE;
static uint8_t adc_num_samples = 0;
static int8_t last_sample_channel = -1;
static uint16_t adc_wave[ADC_NUM_APPROX_SAMPLES];
static uint16_t adc_wave_copy[ADC_NUM_APPROX_SAMPLES];
static uint32_t main_approx, generator_approx;
#endif /* (COMPUTE_APPROX == TRUE) */
static uint32_t adc_sample;
static CONDITION adc_condition;
static SUSPEND adc_suspend;

/* ADC APIs. */
void adc_data_callback(uint32_t);
void adc_sample_process(void *, int32_t);

void generator_self(void)
{
    uint32_t i, loop;
    INT_LVL interrupt_level;
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

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Self the generator. */
        PORT_SELFON_IND |= (1 << PIN_SELFON_IND);
        DDR_GENSELF_ON |= (1 << PIN_GENSELF_ON);
        PORT_GENSELF_ON |= (1 << PIN_GENSELF_ON);

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Wait before releasing the self. */
        sleep_fms(GENERATOR_SELF_DELAY + (GENERATOR_SELF_DEL_INC * loop));

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Release the self. */
        PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
        DDR_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
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

#ifdef ENABLE_COUTERMEASURE
            /* Release the self here in any case. */
            PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
            DDR_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
            PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
#endif /* ENABLE_COUTERMEASURE */

            /* If main has crossed the threshold. */
            if (main_volt > VOLTAGE_THRESHOLD)
            {
                /* Main detected. */
                PORT_MAINON_IND |= (1 << PIN_MAINON_IND);
                main_on = TRUE;
            }
            else
            {
                /* If this is a new power failure. */
                if (main_on == TRUE)
                {
                    /* Main is no longer on. */
                    PORT_MAINON_IND  &= (uint8_t)(~(1 << PIN_MAINON_IND));
                    main_on = FALSE;

                    /* Save the time at which this happened. */
                    last_power_failure = current_system_tick();
                }
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
            sleep_fms(STATE_DELAY);
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
        (*auto_start) ^= 0x1;
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
    INT_LVL interrupt_level;
    uint32_t gen_off_count;
    uint32_t switch_count = 0;
    uint32_t generator_count = 0;
    uint32_t change_over_count = 0;
    uint8_t generator_on;
    uint8_t main_on = FALSE;
    uint8_t generator_selfed = FALSE;
    uint8_t generator_switched_on = FALSE;
    uint8_t change_over_on = FALSE;
    uint8_t supply_gen = FALSE;
    uint8_t do_gen_off = FALSE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Turn off all the indicators. */
    PORT_GENON_IND &= (uint8_t)(~(1 << PIN_GENON_IND));
    PORT_MAINON_IND &= (uint8_t)(~(1 << PIN_MAINON_IND));
    PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));

    /* Turn off generator power and self. */
    DDR_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
    PORT_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
    DDR_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
    PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));

    /* Turn off the generator relay. */
    PORT_GENERATOR_RELAY &= (uint8_t)(~(1 << PIN_GENERATOR_RELAY));

    /* TURN on the connected LED. */
    PORT_CONNECTED |= (1 << PIN_CONNECTED);

    /* Configure the button as input. */
    PORT_AUTO_SEL &= (uint8_t)(~(1 << PIN_AUTO_SEL));
    DDR_AUTO_SEL &= (uint8_t)(~(1 << PIN_AUTO_SEL));

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Wait for system to stabilize. */
    sleep_fms(POWER_ON_DELAY);

    for (;;)
    {
        /* Wait for ADC to take new readings. */
        sleep_fms(STATE_DELAY);

#ifdef ENABLE_COUTERMEASURE
        /* Release the self here in any case. */
        PORT_SELFON_IND &= (uint8_t)(~(1 << PIN_SELFON_IND));
        DDR_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
        PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
#endif /* ENABLE_COUTERMEASURE */

        /* Check if button is pressed. */
        if (!(IN_AUTO_SEL & (1 << PIN_AUTO_SEL)))
        {
            /* Wait for sometime. */
            sleep_fms(DEBOUNCE_DELAY);

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

                    sleep_fms(DEBOUNCE_DELAY);
                }

                break;
            }

            /* Sleep for de-bounce of the key. */
            sleep_fms(DEBOUNCE_DELAY);
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
        if (main_volt > VOLTAGE_THRESHOLD)
        {
            /* Main detected. */
            PORT_MAINON_IND |= (1 << PIN_MAINON_IND);
            main_on = TRUE;
        }
        else
        {
            /* If this is a new power failure. */
            if (main_on == TRUE)
            {
                /* Main is no longer on. */
                PORT_MAINON_IND  &= (uint8_t)(~(1 << PIN_MAINON_IND));
                main_on = FALSE;

                /* Save the time at which this happened. */
                last_power_failure = current_system_tick();
            }
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

        /* Check if generator is no longer on. */
        if (generator_on == FALSE)
        {
            /* If we were on generator. */
            if (supply_gen == TRUE)
            {
                /* Increment generator count. */
                generator_count ++;

                /* If we have waited long enough to turn off the generator relay. */
                if (generator_count >= (SWITCH_DELAY / STATE_DELAY))
                {
                    /* Get system interrupt level. */
                    interrupt_level = GET_INTERRUPT_LEVEL();

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    /* Turn off the generator relay. */
                    PORT_GENERATOR_RELAY &= ((uint8_t)~(1 << PIN_GENERATOR_RELAY));

                    /* Supply is no longer on the generator. */
                    supply_gen = FALSE;

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);
                }
            }
        }
        else
        {
            /* Reset the generator count. */
            generator_count = 0;
        }

        /* Check if main is on or we need to turn off the generator. */
        if ((main_on == TRUE) || (do_gen_off == TRUE))
        {
            /* If generator is still on. */
            if ((generator_on == TRUE) || (generator_switched_on == TRUE))
            {
                /* We have detected an update in state. */
                switch_count ++;

                /* If we were on generator. */
                if (supply_gen == TRUE)
                {
                    /* If we have waited long enough to turn off the switch over. */
                    if (switch_count >= (GENERATOR_RELAY_DELAY / STATE_DELAY))
                    {
                        /* Get system interrupt level. */
                        interrupt_level = GET_INTERRUPT_LEVEL();

                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* Turn off the generator relay. */
                        PORT_GENERATOR_RELAY &= ((uint8_t)~(1 << PIN_GENERATOR_RELAY));

                        /* Supply is no longer on the generator. */
                        supply_gen = FALSE;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);
                    }
                }

                /* If change over is still on. */
                if ((change_over_on == TRUE) && (switch_count > (SWITCH_DELAY / STATE_DELAY)))
                {
                    /* Turn off the change over. */
                    PORT_CHANGE_OVER &= (uint8_t)(~(1 << PIN_CHANGE_OVER));

                    /* Change over is now turned off. */
                    change_over_on = FALSE;
                }

                /* If we have waited long enough to process this state. */
                if (switch_count > (GENERATOR_OFF_DELAY / STATE_DELAY))
                {
                    /* Turn off the generator. */
                    DDR_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
                    PORT_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));

                    /* Generator is no longer on and clear the self-ed flag. */
                    generator_switched_on = FALSE;
                    generator_selfed = FALSE;

                    /* Reset the switch count. */
                    switch_count = 0;

                    /* Clear the generator off flag. */
                    do_gen_off = FALSE;
                }
            }
            else
            {
                /* Reset the switch count. */
                switch_count = 0;

                /* Clear the generator off flag. */
                do_gen_off = FALSE;
            }
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
                    DDR_GENPWR_ON |= (1 << PIN_GENPWR_ON);
                    PORT_GENPWR_ON |= (1 << PIN_GENPWR_ON);

                    /* Generator is turned on. */
                    generator_switched_on = TRUE;

                    /* Reset the switch count. */
                    switch_count = 0;
                }
            }
            else
            {
                /* Reset the switch count. */
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

                /* Turn off the generator relay. */
                PORT_GENERATOR_RELAY &= ((uint8_t)~(1 << PIN_GENERATOR_RELAY));
                supply_gen = FALSE;

                /* Restore old interrupt level. */
                SET_INTERRUPT_LEVEL(interrupt_level);

                /* Self the generator. */
                generator_self();

                /* We have self-ed the generator. */
                generator_selfed = TRUE;

                /* Reset change over count. */
                change_over_count = 0;
            }

            /* Check if generator is now turned on. */
            if ((auto_start == TRUE) && (generator_on == TRUE) && (supply_gen ==  FALSE))
            {
                /* If change over is not yet on. */
                if (change_over_on == FALSE)
                {
                    /* Turn on the change over. */
                    PORT_CHANGE_OVER |= (1 << PIN_CHANGE_OVER);

                    /* Change over is now turned on. */
                    change_over_on = TRUE;

                    /* Reset change over count. */
                    change_over_count = 0;
                }
                else
                {
                    /* Increment change over count. */
                    change_over_count ++;

                    /* If there is enough time for the change over to settle. */
                    if (change_over_count > (CHANGE_OVER_DELAY / STATE_DELAY))
                    {
                        /* Get system interrupt level. */
                        interrupt_level = GET_INTERRUPT_LEVEL();

                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* Turn on the generator relay. */
                        PORT_GENERATOR_RELAY |= (1 << PIN_GENERATOR_RELAY);

                        /* Supply is no longer on the generator. */
                        supply_gen = TRUE;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);
                    }
                }
            }

            if (change_over_on == FALSE)
            {
                /* Reset change over count. */
                change_over_count = 0;
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
int32_t weird_view_demo_task_stats(uint16_t id, FS_BUFFER_LIST *buffer)
{
    int32_t status;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(id);

    /* Need to update the existing data. */
    status = fs_buffer_list_push(buffer, (uint8_t []){ WV_PLUGIN_LOG_UPDATE }, sizeof(uint8_t), 0);

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
    switch (id)
    {
    case 0x1:
        /* (x * 2 * ((10k + 100k) / 10k)) */
        *value = main_volt;
        *value_div = 1;
        *max_value = 1024;
        break;

    case 0x2:
        *value = generator_volt;
        *value_div = 1;
        *max_value = 1024;
        break;
    }

    /* Always return success. */
    return (SUCCESS);

} /* weird_view_demo_analog_data */

/*
 * weird_view_demo_adc_sample
 * @id: Plugin id.
 * @buffer: File system buffer in which reply will be populated.
 * This is callback function to populate the given buffer with task statistics.
 */
static int32_t weird_view_demo_adc_sample(uint16_t id, FS_BUFFER_LIST *buffer)
{
    int32_t status = -1;

#if (COMPUTE_APPROX == TRUE)
    switch (id)
    {
    case 0x4:
        /* If we have a sample for main line. */
        if (last_sample_channel == ADC_CHN_MAIN)
        {
            /* Let's send this sample. */
            status = SUCCESS;
        }
        break;

    case 0x5:
        /* If we have a sample for generator. */
        if (last_sample_channel == ADC_CHN_GENERATOR)
        {
            /* Let's send this sample. */
            status = SUCCESS;
        }
        break;
    }

    if (status == SUCCESS)
    {
        /* Add sample size. */
        status = fs_buffer_list_push(buffer, (uint8_t []){ sizeof(uint16_t) }, sizeof(uint8_t), 0);
    }

    if (status == SUCCESS)
    {
        /* Push ADC sample on the buffer. */
        status = fs_buffer_list_push(buffer, (uint8_t *)adc_wave_copy, (ADC_NUM_APPROX_SAMPLES * sizeof(uint16_t)), 0);
    }
#else

    /* Remove some compiler warnings. */
    UNUSED_PARAM(id);
    UNUSED_PARAM(buffer);
#endif

    /* Always status to the caller. */
    return (status);

} /* weird_view_demo_adc_sample */

/*
 * adc_data_callback
 * @data: ADC reading.
 * This is callback function for ADC conversion. This function will be called
 * in the context of a ISR.
 */
void adc_data_callback(uint32_t data)
{
    static int32_t n = 0;
#if (COMPUTE_APPROX == TRUE)
    static int32_t wave_index = 0;
    static uint16_t last_sample = 0;
    static uint8_t edge = 0;
    static uint32_t sample_tick = 0;
#endif /* (COMPUTE_APPROX == TRUE) */

#if (ENABLE_WDT == TRUE)
    /* Reset watch dog timer. */
    WDT_RESET();
#endif

#if COMPUTE_AVG
    /* Add the new reading. */
    adc_sample += (uint32_t)data;
#else
    /* If we have a new max. */
    if (data > adc_sample)
    {
        /* Save the new max. */
        adc_sample = data;
    }
#endif /* COMPUTE_AVG */

#if (COMPUTE_APPROX == TRUE)
    /* If we have not yet found a wave. */
    if (adc_got_wave == FALSE)
    {
        /* If we have not yet found a negative edge. */
        if (edge == 0)
        {
            /* If this is a negative edge. */
            if (last_sample > data)
            {
                /* Lets start waiting for a positive edge. */
                edge = 1;
            }
        }

        /* If we were at a negative edge. */
        if (edge == 1)
        {
            /* If this is a positive edge. */
            if (last_sample < data)
            {
                /* Reset the wave. */
                edge = 0;
                wave_index = 0;

#if ENFORCE_ZERO_CROSSING
                /* We did start from a zero. */
                if (last_sample == 0)
#endif /* ENFORCE_ZERO_CROSSING */
                {
                    /* Let's save starting values. */
                    adc_wave[wave_index++] = last_sample;
                    adc_wave[wave_index++] = data;

                    /* This may be a good wave. */
                    adc_got_wave = TRUE;

                    /* Save the current system tick. */
                    sample_tick = current_system_tick();
                }
#if ENFORCE_ZERO_CROSSING
                else
                {
                    /* This is not a good wave. */
                    adc_got_wave = FALSE;
                }
#endif /* ENFORCE_ZERO_CROSSING */
            }
        }
    }
    else
    {
        /* If we don't have a complete good wave. */
        if (edge != 2)
        {
            /* If we are still at positive edge. */
            if (edge == 0)
            {
                /* If we are now at negative edge. */
                if (last_sample > data)
                {
                    /* Move to next edge. */
                    edge = 1;
                }
            }
            else
            {
                /* If we again encountered a positive edge. */
                if (last_sample < data)
                {
#if ENFORCE_ZERO_CROSSING
                    /* If previous sample was zero. */
                    if (last_sample == 0)
#endif /* ENFORCE_ZERO_CROSSING */
                    {
                        /* We now have the wave. */
                        edge = 2;
                    }
#if ENFORCE_ZERO_CROSSING
                    else
                    {
                        /* This wave is not good. */
                        adc_got_wave = FALSE;

                        /* Reset the wave index. */
                        wave_index = 0;

                        /* Start looking for next negative edge. */
                        edge = 0;
                    }
#endif /* ENFORCE_ZERO_CROSSING */
                }
            }

            /* If we are still acquiring this wave. */
            if ((edge != 2) && (adc_got_wave == TRUE))
            {
                /* Pick a wave sample. */
                adc_wave[wave_index++] = data;
            }

            /* If we processed a system tick while acquiring this sample or we
             * have exhausted the sample buffer. */
            if ((sample_tick != current_system_tick()) || (return_task != idle_task_get()) || (wave_index == ADC_NUM_APPROX_SAMPLES))
            {
                /* This wave is not good. */
                adc_got_wave = FALSE;

                /* Reset the wave index. */
                wave_index = 0;

                /* Start looking for next negative edge. */
                edge = 0;
            }
        }
    }

    /* Save the last sample. */
    last_sample = data;

#endif /* (COMPUTE_APPROX == TRUE) */

    /* We have taken a sample. */
    n++;

    /* If we have processed the maximum number of samples or we do have a
     * stable wave. */
#if (COMPUTE_APPROX == TRUE)
    if ((n == ADC_NUM_SAMPLES) || ((adc_got_wave == TRUE) && (edge == 2)))
#else
    if (n == ADC_NUM_SAMPLES)
#endif
    {
        /* Reset the sample counter. */
        n = 0;

#if (COMPUTE_APPROX == TRUE)
        /* If we did pick a wave. */
        if (edge == 2)
        {
            /* Save the number of samples we picked. */
            adc_num_samples = wave_index;
        }
        else
        {
            /* This wave is not good. */
            adc_got_wave = FALSE;
        }

        /* Reset the wave counters. */
        wave_index = 0;
        edge = 0;
        last_sample = 0;
#endif /* (COMPUTE_APPROX == TRUE) */

        /* Stop ADC sampling. */
        adc_avr_periodic_read_stop();

        /* Set the ping flag for ADC condition. */
        adc_condition.flags |= CONDITION_PING;

        /* Resume any tasks waiting for ADC condition. */
        resume_condition(&adc_condition, NULL, TRUE);
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
    uint32_t v_int = 0;
    INT_LVL interrupt_level;
#if (COMPUTE_APPROX == TRUE)
    uint32_t i;
    double approx_alpha = 0, approx_beta = 0, avrg, sd;
#endif /* (COMPUTE_APPROX == TRUE) */

    /* Remove some compiler warning. */
    UNUSED_PARAM(data);
    UNUSED_PARAM(status);

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
#if COMPUTE_AVG
        /* Compute average of ADC sample. */
        v_int += adc_sample/ADC_SAMPLES;
#else
        /* Use the MAX value as the sample. */
        v_int = adc_sample;
#endif /* COMPUTE_AVG */

#if (COMPUTE_APPROX == TRUE)
        /* If we do have a good wave. */
        if (adc_got_wave == TRUE)
        {
            /* Calculate sum of all the values. */
            avrg = 0;
            for (i = 0; i < adc_num_samples; i++)
            {
                avrg += (double)adc_wave[i];
            }

            /* Compute sample average. */
            avrg = avrg / adc_num_samples;

            /* Calculate standard deviation. */
            sd = 0;
            for (i = 0; i < adc_num_samples; i++)
            {
                sd += pow(((double)adc_wave[i] - avrg), 2);
            }
            sd /= adc_num_samples;
            sd = sqrt(sd);

            /* Alpha is sd(samples) / sd(sin(x)). */
            approx_alpha = sd / 0.308124;

            /* For beta avrg(samples) - (alpha * avrg(sin(x))). */
            approx_beta = avrg - ((approx_alpha * 2) / (3.141592));
        }
#if DEBUG_WAVE
        for (i = 0; i < adc_num_samples; i++)
        {
            printf("%d\r\n", adc_wave[i]);
        }
        io_puts("-------------\r\n");
#endif
#endif /* (COMPUTE_APPROX == TRUE) */

        /* Reset the ADC sample. */
        adc_sample = 0;

        /* Get system interrupt level. */
        interrupt_level = GET_INTERRUPT_LEVEL();

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

#ifdef PORT_SAMPLE_IND
        /* A sample was processed. */
        PORT_SAMPLE_IND ^= (1 << PIN_SAMPLE_IND);
#endif

        switch (current_channel)
        {
        case ADC_CHN_MAIN:

            /* Save the ADC reading. */
            main_volt = v_int;
            current_channel = ADC_CHN_GENERATOR;

#if (COMPUTE_APPROX == TRUE)
            /* If we do have a good wave. */
            if (adc_got_wave == TRUE)
            {
                /* Compute the wave max using the approximation constants. */
                main_approx = approx_alpha - approx_beta;

                /* Mark this as an okay sample. */
                last_sample_channel = ADC_CHN_MAIN;
            }
            else
            {
                /* Approximation is not available. */
                main_approx = 0;
            }
#endif /* (COMPUTE_APPROX == TRUE) */

            break;

        case ADC_CHN_GENERATOR:

            /* Save the ADC reading. */
            generator_volt = v_int;
            current_channel = ADC_CHN_MAIN;

#if (COMPUTE_APPROX == TRUE)
            /* If we do have a good wave. */
            if (adc_got_wave == TRUE)
            {
                /* Compute the wave max using the approximation constants. */
                generator_approx = approx_alpha - approx_beta;

                /* Mark this as an okay sample. */
                last_sample_channel = ADC_CHN_GENERATOR;
            }
            else
            {
                /* Approximation is not available. */
                generator_approx = 0;
            }
#endif /* (COMPUTE_APPROX == TRUE) */

            break;
        }
#if (COMPUTE_APPROX == TRUE)
        /* If we do have a good wave. */
        if (adc_got_wave == TRUE)
        {
            /* Reset the wave flag. */
            adc_got_wave = FALSE;

            /* Make a copy of this sample. */
            memcpy(adc_wave_copy, adc_wave, sizeof(uint16_t) * ADC_NUM_APPROX_SAMPLES);
            memset(adc_wave, 0, sizeof(uint16_t) * ADC_NUM_APPROX_SAMPLES);
        }
#endif /* (COMPUTE_APPROX == TRUE) */

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Select required channel. */
        adc_channel_select(current_channel);

        /* Before actually starting the sampling, wait for channel to switch. */
        adc_suspend.timeout = (current_system_tick() + MS_TO_TICK(ADC_CHANNEL_DELAY));
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

/*
 * log_entry
 * @argv: Task argument.
 * This is main entry function for smart switch control task.
 */
void log_entry(void *argv)
{
    uint32_t systick, ip_address;
    static FD *enc28j60_fd = NULL;
    uint8_t day, hour, min, sec, milisec;
    char str[10];

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

#ifdef IO_LCD_AN
    /* Initialize LCD AN. */
    lcd_an_init();
#endif

    /* Open ethernet device. */
    enc28j60_fd = fs_open("\\ethernet\\enc28j60", 0);

    for (;;)
    {
        /* Calculate time since last power failure. */
        systick = current_system_tick() - last_power_failure;

        /* Reset LCD AN interface. */
        fs_ioctl(lcd_an_fd, LCD_AN_RESET, NULL);

        /* Try to get the assigned IP address to the ethernet controller. */
        ip_address = IPV4_ADDR_UNSPEC;
        ipv4_get_device_address(enc28j60_fd, &ip_address, NULL);

        /* Calculate time. */
        day = (uint8_t)(systick / (86400LU * SOFT_TICKS_PER_SEC));
        hour = (uint8_t)((systick / (3600LU * SOFT_TICKS_PER_SEC)) - ((uint32_t)day * 24LU));
        min = (uint8_t)((systick / (60LU * SOFT_TICKS_PER_SEC)) - (((uint32_t)day * 1440LU) + ((uint32_t)hour * 60LU)));
        sec = (uint8_t)((systick / (SOFT_TICKS_PER_SEC)) - (((uint32_t)day * 86400LU) + ((uint32_t)hour * 3600LU) + ((uint32_t)min * 60LU)));
        milisec = (uint8_t)((systick) - ((((uint32_t)day * 86400LU) + ((uint32_t)hour * 3600LU) + ((uint32_t)min * 60LU) + ((uint32_t)sec)) * SOFT_TICKS_PER_SEC));
        P_STR_CPY(str, P_STR("\f\t"));
        io_puts(str, -1);
        rtl_ultoa(day, (uint8_t*)str, 99, RTL_ULTOA_LEADING_ZEROS);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR(":"));
        io_puts(str, -1);
        rtl_ultoa(hour, (uint8_t*)str, 24, RTL_ULTOA_LEADING_ZEROS);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR(":"));
        io_puts(str, -1);
        rtl_ultoa(min, (uint8_t*)str, 59, RTL_ULTOA_LEADING_ZEROS);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR(":"));
        io_puts(str, -1);
        rtl_ultoa(sec, (uint8_t*)str, 59, RTL_ULTOA_LEADING_ZEROS);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR("."));
        io_puts(str, -1);
        rtl_ultoa((TICK_TO_MS(milisec) / 10), (uint8_t*)str, 99, RTL_ULTOA_LEADING_ZEROS);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR("\r\n"));
        io_puts(str, -1);

        rtl_ultoa_b10((uint32_t)((ip_address >> 24) & 0xFF), (uint8_t*)str);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR("."));
        io_puts(str, -1);
        rtl_ultoa_b10((uint32_t)((ip_address >> 16) & 0xFF), (uint8_t*)str);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR("."));
        io_puts(str, -1);
        rtl_ultoa_b10((uint32_t)((ip_address >> 8) & 0xFF), (uint8_t*)str);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR("."));
        io_puts(str, -1);
        rtl_ultoa_b10((uint32_t)(ip_address & 0xFF), (uint8_t*)str);
        io_puts(str, -1);
        P_STR_CPY(str, P_STR(" "));
        io_puts(str, -1);

        /* If generator power is on. */
        if (IN_GENPWR_ON & (1 << PIN_GENPWR_ON))
        {
            P_STR_CPY(str, P_STR("P"));
            io_puts(str, -1);
        }

        /* If generator self is on. */
        if (IN_GENSELF_ON & (1 << PIN_GENSELF_ON))
        {
            P_STR_CPY(str, P_STR("S"));
            io_puts(str, -1);
        }

        /* If generator relay is on. */
        if (IN_GENERATOR_RELAY & (1 << PIN_GENERATOR_RELAY))
        {
            P_STR_CPY(str, P_STR("R"));
            io_puts(str, -1);
        }

        P_STR_CPY(str, P_STR("\r\n"));
        io_puts(str, -1);

        P_STR_CPY(str, P_STR("V(M): "));
        io_puts(str, -1);
        rtl_ultoa_b10(main_volt, (uint8_t*)str);
        io_puts(str, -1);
#if (COMPUTE_APPROX == TRUE)
        P_STR_CPY(str, P_STR(", "));
        io_puts(str, -1);
        rtl_ultoa_b10(main_approx, (uint8_t*)str);
        io_puts(str, -1);
#endif /* (COMPUTE_APPROX == TRUE) */
        P_STR_CPY(str, P_STR("\r\n"));
        io_puts(str, -1);

        P_STR_CPY(str, P_STR("V(G): "));
        io_puts(str, -1);
        rtl_ultoa_b10(generator_volt, (uint8_t*)str);
        io_puts(str, -1);
#if (COMPUTE_APPROX == TRUE)
        P_STR_CPY(str, P_STR(", "));
        io_puts(str, -1);
        rtl_ultoa_b10(generator_approx, (uint8_t*)str);
        io_puts(str, -1);
#endif /* (COMPUTE_APPROX == TRUE) */

#if (defined(TASK_STATS) && defined(TASK_USAGE))
        /* Reset CPU usage. */
        usage_reset();
#endif /* (defined(TASK_STATS) && defined(TASK_USAGE)) */

        /* Sleep for some time. */
        sleep_fms(LOG_DELAY);
    }

} /* log_entry */

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
    DDR_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
    PORT_GENPWR_ON &= (uint8_t)(~(1 << PIN_GENPWR_ON));
    DDR_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));
    PORT_GENSELF_ON &= (uint8_t)(~(1 << PIN_GENSELF_ON));

    /* Configure and turn off the generator relay. */
    DDR_GENERATOR_RELAY |= (1 << PIN_GENERATOR_RELAY);
    PORT_GENERATOR_RELAY &= (uint8_t)(~(1 << PIN_GENERATOR_RELAY));

    /* Configure connected LED. */
    DDR_CONNECTED |= (1 << PIN_CONNECTED);
    PORT_CONNECTED &= (uint8_t)(~(1 << PIN_CONNECTED));

    /* Configure change over. */
    DDR_CHANGE_OVER |= (1 << PIN_CHANGE_OVER);
    PORT_CHANGE_OVER &= (uint8_t)(~(1 << PIN_CHANGE_OVER));

#if (ENABLE_WDT == TRUE)
    /* Reset watch dog timer. */
    WDT_RESET();

    /* Enable WDT with watch interval of 2 second. */
    wdt_enable(WDTO_2S);
#endif

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize networking stack. */
    net_init();

#ifdef IO_SERIAL
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

    /* Initialize condition data and trigger it now. */
    adc_suspend.timeout = current_system_tick() + MS_TO_TICK(ADC_CHANNEL_DELAY);
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

    /* Initialize log task. */
    task_create(&log_cb, P_STR("LOG"), log_stack, LOG_TASK_STACK_SIZE, &log_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&log_cb, 254);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
