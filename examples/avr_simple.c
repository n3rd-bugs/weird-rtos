/*
 * simple.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 *
 * This is an example of how to create the tasks and using different components.
 */
#include <os.h>
#include <sys_info.h>

char    stack_1[192];
TASK    task_cb_1;
char    stack_2[128];
TASK    task_cb_2;
char    stack_3[128];
TASK    task_cb_3;
char    stack_4[128];
TASK    task_cb_4;
char    stack_5[256];
TASK    task_cb_5;
char    stack_6[128];
TASK    task_cb_6;
char    stack_7[128];
TASK    task_cb_7;

SEMAPHORE semaphore;

void    led_task(void *argv);
void    led_task_delayed(void *argv);
void    led_task_periodic(void *argv);
void    print_task(void *argv);
void    semaphore_task(void *argv);

void led_task_delayed(void *argv)
{
    while(1)
    {
        PORTD |= (1 << (uint16_t)argv);
        sleep(5);
        PORTD &= ~(1 << (uint16_t)argv);
        sleep(5);
    }
}

void led_task(void *argv)
{
    while(1)
    {
        DISABLE_INTERRUPTS();
        PORTD |= (1 << (uint16_t)argv);
        PORTD &= ~(1 << (uint16_t)argv);
        ENABLE_INTERRUPTS();
    }
}

void led_task_periodic(void *argv)
{
    while(1)
    {
        PORTD |= (1 << (uint16_t)argv);
        task_yield();
        PORTD &= ~(1 << (uint16_t)argv);
        task_yield();
    }
}

void print_task(void *argv)
{

    while(1)
    {
        util_print_sys_info();
        task_yield();
    }
}

void semaphore_task(void *argv)
{

    while(1)
    {
        semaphore_obtain(&semaphore, (-1));
        PORTB |= (1 << (uint16_t)argv);
        sleep(50);
        PORTB &= ~(1 << (uint16_t)argv);
        sleep(50);
        semaphore_release(&semaphore);
    }
}

int main(void)
{
    PORTD = PORTB = 0x00;
    DDRD  = 0xF0;
    DDRB  = 0xFF;

    stdout = &serial_fd;

    scheduler_init();

    semaphore_create(&semaphore, 1, 1, SEMAPHORE_FIFO);

    task_create(&task_cb_1, "TSK_1", stack_1, 192, &led_task_delayed, (void *)0x07);
    task_create(&task_cb_2, "TSK_2", stack_2, 128, &led_task_periodic, (void *)0x06);
    task_create(&task_cb_3, "TSK_3", stack_3, 128, &led_task, (void *)0x05);
    task_create(&task_cb_4, "TSK_4", stack_4, 128, &led_task, (void *)0x04);
    task_create(&task_cb_5, "TSK_5", stack_5, 256, &print_task, (void *)0x00);
    task_create(&task_cb_6, "TSK_6", stack_6, 128, &semaphore_task, (void *)0x02);
    task_create(&task_cb_7, "TSK_7", stack_7, 128, &semaphore_task, (void *)0x03);

    scheduler_task_add(&task_cb_1, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_2, TASK_PERIODIC, 1, 500);
    scheduler_task_add(&task_cb_3, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_4, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_5, TASK_PERIODIC, 2, 1000);
    scheduler_task_add(&task_cb_6, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_7, TASK_APERIODIC, 2, 0);

    os_run();

    return (0);

}
