/*
 * mem_test.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
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
#include <mem_stats.h>
#include <serial.h>
#include <string.h>
#include <MK40DZ10.h>
#include <sll.h>
#include <stdlib.h>
#include <path.h>

#define NUM_DEMO_TASK       5
#define DEMO_STACK_SIZE     300
#define MAX_ISR_CYCLES      10000
#define MAX_TIMER_TICKS     (NUM_DEMO_TASK * MAX_ISR_CYCLES)

#define MEM_MAX_ALLOC       (0x6EC4 / 2)
#define MAX_ALLOC           0x10
#define MEM_PER_TASK        (MEM_MAX_ALLOC / NUM_DEMO_TASK)

typedef struct _mem_test_node MEM_TEST_NODE;
struct _mem_test_node
{
    MEM_TEST_NODE   *next;
    uint32_t        size;
};

typedef struct _mem_test_list
{
    MEM_TEST_NODE   *head;
    MEM_TEST_NODE   *tail;
} MEM_TEST_LIST;

void    ctx_sample_task(void *argv);
void    stat_task(void *argv);

uint8_t pop_n(void *node, void *param)
{
    uint8_t match = FALSE;

    UNUSED_PARAM(node);

    if ( *((uint32_t *)param) == 0 )
    {
        match = TRUE;
    }
    else
    {
        *((uint32_t *)param) = *((uint32_t *)param) - 1;
    }

    return (match);

} /* pop_n */

void ctx_sample_task(void *argv)
{
    UNUSED_PARAM(argv);
    uint32_t last_tick;
    uint32_t n = 0;
    uint32_t com = 0;
    uint32_t *ret_time = (uint32_t *)argv;
    MEM_TEST_NODE *mem;
    uint32_t mem_size;
    uint32_t total_allocated;
    uint32_t num_allocated, mem_num;
    MEM_TEST_LIST mem_test_list;

    ret_time[0] = 0;

    num_allocated = 0;
    total_allocated = 0;
    mem_test_list.head =
    mem_test_list.tail = NULL;

    /* Initialize random seed. */
    srand(current_system_tick64());

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

        last_tick = current_system_tick64() - last_tick;
        if (last_tick < MAX_TIMER_TICKS)
        {
            com += last_tick;
            ret_time[0] = (com / n);
        }

        mem_size = (rand() % MAX_ALLOC) + sizeof(MEM_TEST_NODE);
        mem = (MEM_TEST_NODE *)mem_dynamic_alloc(mem_size);

        if (mem)
        {
            total_allocated += mem_size;
            mem->size = mem_size;
            sll_append(&mem_test_list, mem, OFFSETOF(MEM_TEST_NODE, next));
            num_allocated++;
        }

        while ( (total_allocated > MEM_PER_TASK) || (mem == NULL) )
        {
            mem_num = rand() % num_allocated;
            mem = sll_search_pop(&mem_test_list, pop_n, &mem_num, OFFSETOF(MEM_TEST_NODE, next));

            if (mem)
            {
                total_allocated -= mem->size;
                num_allocated --;
                mem_dynamic_dealloc((char *)mem);
            }
            else
            {
                break;
            }
        }
    }
}

void stat_task(void *argv)
{
    UNUSED_PARAM(argv);
    uint32_t *t = (uint32_t *)argv, i;
    extern MEM_DYNAMIC mem_dynamic;

    while(1)
    {
        util_print_sys_info();
        mem_dynamic_print_usage(&mem_dynamic, (STAT_MEM_GENERAL));

        /* Print time slices for the memory tasks. */
        for (i = 0; i < NUM_DEMO_TASK; i++)
        {
            printf("T%d: %lu ", (i+1), t[i]);
        }
        printf("\r\n");

        task_yield();
    }
}

int main(void)
{
    TASK        *ctx_task_cb[NUM_DEMO_TASK], *stat_task_cb;
    uint32_t    *ctx_time;
    int         i;

    SIM_SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;
    PORTE_PCR0 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
    PORTE_PCR1 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;

    /* Initialize file system. */
    fs_init();

    /* Initialize serial console. */
    serial_init(PCLK_FREQ, 115200);

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize memory. */
    mem_init();

    /* Allocate memory for stats. */
    ctx_time = (uint32_t *)mem_static_alloc((sizeof(uint32_t) * NUM_DEMO_TASK));

    /* Create memory tasks. */
    for (i = 0; i < NUM_DEMO_TASK; i++)
    {
        ctx_task_cb[i] = (TASK *)mem_static_alloc(sizeof(TASK) + DEMO_STACK_SIZE);
        task_create(ctx_task_cb[i], "CTX_TSK", (char *)(ctx_task_cb[i] + 1), DEMO_STACK_SIZE, &ctx_sample_task, (void *)(&ctx_time[i]));
        scheduler_task_add(ctx_task_cb[i], TASK_APERIODIC, 2, 0);
    }

    /* Create a stat task. */
    stat_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(stat_task_cb, "STATS", (char *)(stat_task_cb + 1), 4096, &stat_task, (void *)(ctx_time));
    scheduler_task_add(stat_task_cb, TASK_PERIODIC, 1, 100);

    /* Run scheduler. */
    os_run();

    return (0);

}
