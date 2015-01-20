/*
 * ctx_switch.c
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#include <stdio.h>
#include <os.h>
#include <sys_info.h>
#include <serial.h>
#include <string.h>

#define NUM_DEMO_TASK       5
#define DEMO_STACK_SIZE     128

void    ctx_sample_task(void *argv);
void    stat_task(void *argv);

void ctx_sample_task(void *argv)
{
    UNUSED_PARAM(argv);
    uint32_t last_tick;
    uint32_t n = 0;
    uint32_t com = 0;
    uint32_t *ret_time = (uint32_t *)argv;

    ret_time[0] = 0;

    while(1)
    {
        n= n + 1;
        last_tick = current_system_tick64();
        task_yield();

        if (com > 0x0FFFFFFF)
        {
            com = 0;
            n = 1;
        }

        com += current_system_tick64() - last_tick;
        ret_time[0] = (com / n);
    }
}

void stat_task(void *argv)
{
    UNUSED_PARAM(argv);
    uint32_t *t = (uint32_t *)argv;

    while(1)
    {
        util_print_sys_info();
        printf("T1: %lu, T2: %lu,  T3: %lu, T4: %lu, T5: %lu\r\n", t[0], t[1], t[2], t[3], t[4]);
        task_yield();
    }
}

int main(void)
{
    TASK        *ctx_task_cb[NUM_DEMO_TASK], *stat_task_cb;
    uint32_t    *ctx_time;
    int         i;

    /* System clock initialization */
    SIM_SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;

    PORTE_PCR4 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
    PORTA_PCR8 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
    GPIOE_PDDR |= (1<<4);
    GPIOA_PDDR |= (1<<8);

    serial_init(PCLK_FREQ, 115200);

    PORTE_PCR0 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
    PORTE_PCR1 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;

    scheduler_init();

    mem_init();

    ctx_time = (uint32_t *)mem_static_alloc((sizeof(uint32_t) * NUM_DEMO_TASK));

    for (i = 0; i < NUM_DEMO_TASK; i++)
    {
        ctx_task_cb[i] = (TASK *)mem_static_alloc(sizeof(TASK) + DEMO_STACK_SIZE);
        task_create(ctx_task_cb[i], "CTX_TSK", (char *)(ctx_task_cb[i] + 1), DEMO_STACK_SIZE, &ctx_sample_task, (void *)(&ctx_time[i]));
        scheduler_task_add(ctx_task_cb[i], TASK_APERIODIC, 2, 0);
    }

    stat_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(stat_task_cb, "TSK_6", (char *)(stat_task_cb + 1), 4096, &stat_task, (void *)(ctx_time));
    scheduler_task_add(stat_task_cb, TASK_PERIODIC, 1, 100);

    os_run();

    return (0);

}
