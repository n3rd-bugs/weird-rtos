/*
 * lcd_demo.c
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
#include <sys_info.h>
#include <math.h>
#include <lcd.h>

/* LCD Demo task definitions. */
#define LCD_DEMO_TASK_STACK_SIZE        512
uint8_t lcd_demo_stack[LCD_DEMO_TASK_STACK_SIZE];
TASK    lcd_demo_cb;
void lcd_demo_entry(void *argv);

/*
 * lcd_demo_entry
 * @argv: Task argument.
 * This is main entry function for LCD demo task.
 */
void lcd_demo_entry(void *argv)
{
    uint64_t systick;

    /* Initialize LCD. */
    lcd_init();

    /* Move to next 500ms boundary. */
    sleep_ms(500 - (TICK_TO_MS(current_system_tick()) % 500));

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
        /* Save current system tick. */
        systick = current_system_tick();

        /* Print "Hello World" with current system tick. */
        printf("\f\1\tWeird RTOS\r\n\tLCD Demo\r\n");
        printf("SysTick\t: %lu", (uint32_t)systick);
        printf("\r\nStack\t\t: %lu", get_current_task()->stack_size - (uint32_t)util_task_calc_free_stack(get_current_task()));

        /* Sleep for 500ms. */
        sleep_ms(500 - TICK_TO_MS(current_system_tick() - systick));
    }
}

/* Main entry function for AVR. */
int main(void)
{
    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize LCD demo task. */
    task_create(&lcd_demo_cb, "LCD", lcd_demo_stack, LCD_DEMO_TASK_STACK_SIZE, &lcd_demo_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&lcd_demo_cb, TASK_APERIODIC, 0, 0);

    /* Run scheduler. */
    os_run();

    return (0);

}
