/*
 * lcd_an_demo.c
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
#include <sys_info.h>
#include <math.h>
#include <lcd_an.h>
#include <serial.h>

/* LCD Demo task definitions. */
#define LCD_DEMO_TASK_STACK_SIZE        384
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
    extern FD lcd_an_fd;
    LCD_AN_IOCTL_DATA ioctl_data;

    /* Initialize alphanumeric LCD. */
    lcd_an_init();

    /* Create custom characters. */
    ioctl_data.index = 0;
    ioctl_data.param = (uint8_t []){ 0b01010, 0b10101, 0b10001, 0b01010,
                                     0b00100, 0b01010, 0b01110, 0b01010 };
    fs_ioctl(lcd_an_fd, LCD_AN_CUSTOM_CHAR, &ioctl_data);
    ioctl_data.index = 1;
    ioctl_data.param = (uint8_t []){ 0b01100, 0b01010, 0b01100, 0b01010,
                                     0b01000, 0b01100, 0b01010, 0b01010 };
    fs_ioctl(lcd_an_fd, LCD_AN_CUSTOM_CHAR, &ioctl_data);
    ioctl_data.index = 2;
    ioctl_data.param = (uint8_t []){ 0b00100, 0b01010, 0b01110, 0b01010,
                                     0b00000, 0b01010, 0b01110, 0b01010 };
    fs_ioctl(lcd_an_fd, LCD_AN_CUSTOM_CHAR, &ioctl_data);
    ioctl_data.index = 3;
    ioctl_data.param = (uint8_t []){ 0b01010, 0b01010, 0b00100, 0b01010,
                                     0b10101, 0b10001, 0b01010, 0b00100 };
    fs_ioctl(lcd_an_fd, LCD_AN_CUSTOM_CHAR, &ioctl_data);

    /* Move to next 500ms boundary. */
    sleep_ms(500 - (TICK_TO_MS(current_system_tick()) % 500));

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
        /* Save current system tick. */
        systick = current_system_tick();

        /* Print "Hello World" with current system tick. */
        printf("\f\10\tWeird RTOS\r\n\1\tLCD Demo\r\n");
        printf("\2SysTick\t: %lu", (uint32_t)systick);
        printf("\r\n\3Stack\t: %lu", get_current_task()->stack_size - (uint32_t)util_task_calc_free_stack(get_current_task()));

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

    /* Initialize serial. */
    serial_init();

    /* Initialize LCD demo task. */
    task_create(&lcd_demo_cb, P_STR("LCD"), lcd_demo_stack, LCD_DEMO_TASK_STACK_SIZE, &lcd_demo_entry, (void *)0, TASK_NO_RETURN);
    scheduler_task_add(&lcd_demo_cb, 0);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
