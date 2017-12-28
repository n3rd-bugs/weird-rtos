/*
 * weird_view_client_demo.c
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
#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include <string.h>
#include <fs.h>
#include <net.h>
#include <net_udp.h>
#include <sys_info.h>
#include <weird_view_server.h>
#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
#include <adc.h>
#endif
#include <math.h>
#include <serial.h>

/* Definitions to communicate with other side. */
#define DEVICE_NAME         "Smart Switch 1"

#define DEMO_SAMPLES        200

/* Function prototypes. */
int32_t weird_view_demo_adc_sample(uint16_t, FS_BUFFER *);
int32_t weird_view_demo_task_stats(uint16_t, FS_BUFFER *);
int32_t weird_view_demo_switch_data(uint16_t, uint8_t *);
void weird_view_demo_switch_req(uint16_t, uint8_t);
int32_t weird_view_demo_analog_data(uint16_t, uint32_t *, uint32_t *, uint32_t *);

/* Battery task stack. */
const char plgn_name_1[] P_STR_MEM = "Switch 1";
const char plgn_name_2[] P_STR_MEM = "Analog 1";
const char plgn_name_3[] P_STR_MEM = "Analog 2";
const char plgn_name_4[] P_STR_MEM = "Samples";
const char plgn_name_5[] P_STR_MEM = "Task Statistics";
WEIRD_VIEW_SERVER           weird_view;
WEIRD_VIEW_PLUGIN           weird_view_plugins[] =
{
        /* Switch plugin. */
        {
                .id         = 0x01,
                .name       = plgn_name_1,
                .data       = (void *)&weird_view_demo_switch_data,
                .request    = (void *)&weird_view_demo_switch_req,
                .type       = WV_PLUGIN_SWITCH
        },

        /* Analog plugin. */
        {
                .id         = 0x02,
                .name       = plgn_name_2,
                .data       = (void *)&weird_view_demo_analog_data,
                .request    = NULL,
                .type       = WV_PLUGIN_ANALOG
        },

        /* Analog plugin. */
        {
                .id         = 0x03,
                .name       = plgn_name_3,
                .data       = (void *)&weird_view_demo_analog_data,
                .request    = NULL,
                .type       = WV_PLUGIN_ANALOG
        },

        /* ADC sample plugin. */
        {
                .id         = 0x04,
                .name       = plgn_name_4,
                .data       = (void *)&weird_view_demo_adc_sample,
                .request    = NULL,
                .type       = WV_PLUGIN_WAVE
        },

        /* Task statistics plugin. */
        {
                .id         = 0x05,
                .name       = plgn_name_5,
                .data       = (void *)&weird_view_demo_task_stats,
                .request    = NULL,
                .type       = WV_PLUGIN_LOG
        },
};

#ifdef HEART_BEAT
/* Heart beat task. */
#define HEARTBEAT_STACK_SIZE        320
uint8_t heartbeat_stack[HEARTBEAT_STACK_SIZE];
TASK    heartbeat_cb;
void heartbeat_entry(void *argv);
#endif

/* ADC configuration and data. */
#define ADC_PRESCALE            ((uint32_t)25)
#define ADC_WAVE_FREQ           ((uint32_t)100)
#define ADC_ATIMER_PRESCALE     ((uint32_t)64)
#define ADC_SAMPLE_PER_WAVE     ((uint32_t)PCLK_FREQ / (ADC_ATIMER_PRESCALE * ADC_PRESCALE * ADC_WAVE_FREQ))

#define NUM_ADC_CHANNELS        (2)
#define ADC_CHANNEL_DELAY       (SOFT_TICKS_PER_SEC / 100)

static uint16_t adc_sample[DEMO_SAMPLES];
static uint16_t adc_sample_log[DEMO_SAMPLES];
static CONDITION adc_condition;
static SUSPEND adc_suspend;

static uint8_t adc_channel;
static uint32_t vavg[NUM_ADC_CHANNELS];
static uint32_t vsamples[NUM_ADC_CHANNELS];

/* ADC APIs. */
void adc_data_callback(uint32_t);
void adc_sample_process(void *, int32_t);


#ifdef HEART_BEAT
/*
 * heartbeat_entry
 * @argv: Task argument.
 * This a heart beat task.
 */
void heartbeat_entry(void *argv)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
        /* Toggle LED. */
        PORTC ^= (1 << 3);

        /* Print system information. */
        util_print_sys_info();

        /* Sleep for some time. */
        sleep_ms(1000);
#endif
    }

} /* heartbeat_entry */
#endif

/*
 * weird_view_demo_adc_sample
 * @id: Plugin id.
 * @buffer: File system buffer in which reply will be populated.
 * This is callback function to populate the given buffer with task statistics.
 */
int32_t weird_view_demo_adc_sample(uint16_t id, FS_BUFFER *buffer)
{
    int32_t status;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(id);

    /* Add sample size. */
    status = fs_buffer_push(buffer, (uint8_t []){ sizeof(uint16_t) }, sizeof(uint8_t), 0);

    if (status == SUCCESS)
    {
        /* Push ADC sample on the buffer. */
        status = fs_buffer_push(buffer, (uint8_t *)adc_sample_log, (DEMO_SAMPLES * sizeof(uint16_t)) / 2, 0);
    }

    /* Always return success. */
    return (SUCCESS);

} /* weird_view_demo_adc_sample */

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
 * weird_view_demo_switch_data
 * @id: Plugin id.
 * @state: Current switch state will be returned here.
 * This is callback function to populate the given buffer with state of this
 * switch.
 */
int32_t weird_view_demo_switch_data(uint16_t id, uint8_t *state)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(id);

#if (TARGET_PLATFORM == PLAT_STM32F407_DISC)
    /* If switch is active. */
    if (GPIOA->IDR & 0x01)
    {
        /* Switch is on. */
        *state = TRUE;
    }
    else
    {
        /* Switch is off. */
        *state = FALSE;
    }
#elif (TARGET_PLATFORM == PLAT_ATMEGAXX4)
    /* If LED is active. */
    if (PORTC & (1 << 2))
    {
        /* Switch is on. */
        *state = TRUE;
    }
    else
    {
        /* Switch is off. */
        *state = FALSE;
    }
#endif

    /* Always return success. */
    return (SUCCESS);

} /* weird_view_demo_switch_data */

/*
 * weird_view_demo_switch_req
 * @id: Plugin id.
 * @state: New state we need to set.
 * This is callback function to process a request for the demo switch.
 */
void weird_view_demo_switch_req(uint16_t id, uint8_t state)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(id);

#if (TARGET_PLATFORM == PLAT_STM32F407_DISC)
    /* If on requested. */
    if (state == TRUE)
    {
        /* Turn ON the LED. */
        GPIOD->ODR |= (1 << 12);
    }
    else
    {
        /* Turn OFF the LED. */
        GPIOD->ODR &= (uint32_t)(~(1 << 12));
    }
#elif (TARGET_PLATFORM == PLAT_ATMEGAXX4)
    /* If on requested. */
    if (state == TRUE)
    {
        /* Turn ON the LED. */
        PORTC |= (1 << 2);
    }
    else
    {
        /* Turn OFF the LED. */
        PORTC &= (uint8_t)~(1 << 2);
    }
#endif

} /* weird_view_demo_switch_req */

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
    case 0x02:
        *value = (vavg[0] * 256);
        *value_div = ((uint32_t)vsamples[0] * 102400);
        break;

    case 0x03:
        *value = (vavg[1] * 256);
        *value_div = ((uint32_t)vsamples[1] * 102400);
        break;
    }

    *max_value = 3;

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
    INT_LVL interrupt_level;

#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
    /* Toggle an LED show sample rate. */
    PORTC ^= (1 << 4);
#endif

    /* Put data on the ADC sample. */
    adc_sample[n] = (uint16_t)data;

    /* We have taken a sample. */
    n++;

    /* If we have required number of samples. */
    if (n == DEMO_SAMPLES)
    {
        /* Reset the sample counter. */
        n = 0;

#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
        /* Stop ADC sampling. */
        adc_atmega644_periodic_read_stop();
#endif

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
    uint32_t i, j, v_int = 0, num_samples;

    /* Remove some compiler warning. */
    UNUSED_PARAM(data);
    UNUSED_PARAM(status);

    /* Were we waiting for ADC channel to stabilize. */
    if (adc_suspend.timeout != MAX_WAIT)
    {
        /* Stop the ADC timer. */
        adc_suspend.timeout = MAX_WAIT;

#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
        /* Start periodic ADC conversion. */
        adc_atmega644_periodic_read_start(&adc_data_callback, (ADC_PRESCALE - 1));
#endif
    }

    else
    {
#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
        /* Toggle an LED show a sample. */
        PORTC ^= (1 << 5);
#endif

        /* Find the first 0 sample. */
        for (i = 0; i < DEMO_SAMPLES; i++)
        {
            /* If this is zero crossing. */
            if (adc_sample[i] == 0)
            {
                break;
            }
        }

        /* If we do have at-least one zero crossing and a wave form. */
        if ((i != DEMO_SAMPLES) && ((DEMO_SAMPLES - i) >= ADC_SAMPLE_PER_WAVE))
        {
            /* Calculate number of remaining samples. */
            num_samples = (DEMO_SAMPLES - i);

#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
            /* Find the last zero crossing. */
            while (num_samples > 0)
            {
                /* Check if this is the last zero crossing. */
                if (adc_sample[i + (num_samples - 1)] == 0)
                {
                    break;
                }
                else
                {
                    /* Move back in samples. */
                    num_samples--;
                }
            }
#endif

            /* If we do have a sample to compute average on. */
            if (num_samples > ADC_SAMPLE_PER_WAVE)
            {
                /* Save of copy of this sample. */
                memcpy(adc_sample_log, &adc_sample[i], num_samples * sizeof(uint16_t));
                memset(&adc_sample_log[num_samples], 0, (DEMO_SAMPLES - num_samples) * sizeof(uint16_t));

                /* Calculate the sum of all the samples. */
                for (j = i, i = 0; i < num_samples; i++, j++)
                {
                    /* Add this sample to total sum. */
                    v_int = v_int + adc_sample[j];
                }

                /* Update the global sample data. */
                vavg[adc_channel] = v_int;
                vsamples[adc_channel] = num_samples;
            }
        }

        /* Move to next ADC channel. */
        adc_channel++;
        if (adc_channel == NUM_ADC_CHANNELS)
        {
            adc_channel = 0;
        }

#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
        switch (adc_channel)
        {
        case 0:
            /* Select ADC channel 4. */
            adc_channel_select(4);
            break;

        case 1:
            /* Select ADC channel 3. */
            adc_channel_select(3);
            break;
        }
#endif

        /* Before actually start sampling wait for some time. */
        adc_suspend.timeout = (current_system_tick() + ADC_CHANNEL_DELAY);
    }

} /* adc_sample_process */

/* Main entry function for AVR. */
int main(void)
{
    SOCKET_ADDRESS  socket_address;

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

#if (TARGET_PLATFORM == PLAT_STM32F407_DISC)
    /* Enable GPIOA and GPIOD clock. */
    RCC->AHB1ENR |= 0x00000009;

    /* Configure GPIO mode input for GPIOA.0. */
    GPIOA->MODER &= ~((GPIO_MODER_MODER0 << (0 * 2)));

    /* Configure GPIO mode output for GPIOD.12. */
    GPIOD->MODER &= ~((GPIO_MODER_MODER0 << (12 * 2)));
    GPIOD->MODER |= (0x01 << (12 * 2));

    /* Configure output type (PP). */
    GPIOA->OTYPER &= (uint32_t)(~((GPIO_OTYPER_OT_0 << (0 * 2))));

    /* Configure output type (PP). */
    GPIOD->OTYPER &= (uint32_t)(~((GPIO_OTYPER_OT_0 << (12 * 2))));

    /* Enable pull-down on GPIOA.0. */
    GPIOA->PUPDR &= (uint32_t)(~(((GPIO_PUPDR_PUPDR0 << (0 * 2)))));
    GPIOA->PUPDR |= (0x02 << (0 * 2));

    /* Enable pull-up on GPIOD.12. */
    GPIOD->PUPDR &= (uint32_t)(~(((GPIO_PUPDR_PUPDR0 << (12 * 2)))));
    GPIOD->PUPDR |= (0x01 << (12 * 2));

    /* Configure GPIO speed (100MHz). */
    GPIOA->OSPEEDR &= (uint32_t)(~((GPIO_OSPEEDER_OSPEEDR0 << (0 * 2))));
    GPIOA->OSPEEDR |= ((0x03 << (0 * 2)));

    /* Configure GPIO speed (50MHz). */
    GPIOD->OSPEEDR &= (uint32_t)(~((GPIO_OSPEEDER_OSPEEDR0 << (12 * 2))));
    GPIOD->OSPEEDR |= ((0x02 << (12 * 2)));

    /* By default Turn ON the LED. */
    GPIOD->ODR |= (1 << 12);
#elif (TARGET_PLATFORM == PLAT_ATMEGAXX4)
    /* Configure PC2 as output. */
    DDRC |= ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));

    /* Initialize ADC. */
    adc_init();

    /* Select ADC channel 4. */
    adc_channel_select(4);
#endif

    /* Initialize a weird view server instance. */
    weird_view_server_init(&weird_view, &socket_address, DEVICE_NAME, weird_view_plugins, sizeof(weird_view_plugins)/sizeof(WEIRD_VIEW_PLUGIN));

    /* Initialize condition data. */
    adc_suspend.timeout = (uint32_t)(current_system_tick() + ADC_CHANNEL_DELAY);
    adc_suspend.priority = NET_USER_PRIORITY;

    /* Add a networking condition for to process ADC sample event. */
    net_condition_add(&adc_condition, &adc_suspend, &adc_sample_process, (void *)NULL);

#ifdef HEART_BEAT
    /* Initialize heart beat task. */
    task_create(&heartbeat_cb, P_STR("HRTBEAT"), heartbeat_stack, HEARTBEAT_STACK_SIZE, &heartbeat_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&heartbeat_cb, 0);
#endif

    /* Run scheduler. */
    kernel_run();

    return (0);

}
