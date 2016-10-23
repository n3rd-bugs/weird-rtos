/*
 * avr_simple.h
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
#include <semaphore.h>
#include <fs.h>
#include <sys_info.h>

uint8_t stack_1[192];
TASK    task_cb_1;
uint8_t stack_2[128];
TASK    task_cb_2;
uint8_t stack_3[128];
TASK    task_cb_3;
uint8_t stack_4[128];
TASK    task_cb_4;
uint8_t stack_5[320];
TASK    task_cb_5;
uint8_t stack_6[192];
TASK    task_cb_6;
uint8_t stack_7[192];
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
        sleep_ticks(5);
        PORTD &= ~(1 << (uint16_t)argv);
        sleep_ticks(5);
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
        sleep_ticks(10);
        PORTB &= ~(1 << (uint16_t)argv);
        sleep_ticks(10);
        semaphore_release(&semaphore);

        task_yield();
    }
}

int main(void)
{
    PORTD = PORTB = 0x00;
    DDRD  = 0xF0;
    DDRB  = 0xFF;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    semaphore_create(&semaphore, 1, 1, 0);

    task_create(&task_cb_1, "TSK_1", stack_1, 192, &led_task_delayed, (void *)0x07, TASK_NO_RETURN);
    task_create(&task_cb_2, "TSK_2", stack_2, 128, &led_task_periodic, (void *)0x06, TASK_NO_RETURN);
    task_create(&task_cb_3, "TSK_3", stack_3, 128, &led_task, (void *)0x05, TASK_NO_RETURN);
    task_create(&task_cb_4, "TSK_4", stack_4, 128, &led_task, (void *)0x04, TASK_NO_RETURN);
    task_create(&task_cb_5, "TSK_5", stack_5, 320, &print_task, (void *)0x00, TASK_NO_RETURN);
    task_create(&task_cb_6, "TSK_6", stack_6, 192, &semaphore_task, (void *)0x02, TASK_NO_RETURN);
    task_create(&task_cb_7, "TSK_7", stack_7, 192, &semaphore_task, (void *)0x03, TASK_NO_RETURN);

    scheduler_task_add(&task_cb_1, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_2, TASK_PERIODIC, 1, OS_TICKS_PER_SEC/8);
    scheduler_task_add(&task_cb_3, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_4, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_5, TASK_PERIODIC, 2, OS_TICKS_PER_SEC/4);
    scheduler_task_add(&task_cb_6, TASK_APERIODIC, 2, 0);
    scheduler_task_add(&task_cb_7, TASK_APERIODIC, 2, 0);

    os_run();

    return (0);

}
