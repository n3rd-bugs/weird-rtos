/*
 * nmea_demo.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>
#include <string.h>
#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */
#ifdef IO_SERIAL
#include <serial.h>
#endif /* IO_SERIAL */
#include <stdio.h>
#include <oled_ssd1306.h>
#include <nmea.h>
#include <nmea_p.h>
#include <rtl.h>
#include <io.h>

/* Function prototypes. */
void nmea_task(void *);

/* Hello task control block. */
/* TODO adjust this if serial is enabled. */
#define NMEA_STACK_SIZE     1024
TASK nmea_task_cb;
uint8_t nmea_task_stack[NMEA_STACK_SIZE];

void nmea_task(void *argv)
{
    uint8_t buffer[16];

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    oled_ssd1306_init();

    NMEA nmea = {.fd = fs_open("\\console\\usart1", 0), .data = {0}};

    for (int i = 0; i < 10; i++)
    {
        io_puts(".", -1);
        sleep_ms(250);
    }

    nmea_ublox_configure(&nmea, (NMEA_MSG_RMC|NMEA_MSG_VTG));

    while(1)
    {
        if (nmea_fetch_data(&nmea, (NMEA_MSG_RMC|NMEA_MSG_VTG)) ==SUCCESS)
        {
            io_puts("\f", -1);
            io_puts("DATE: ", -1);
            rtl_ultoa_b10(nmea.data.date/10000, buffer);
            io_puts(buffer, -1);
            io_puts("/", -1);
            rtl_ultoa_b10((nmea.data.date % 10000) / 100, buffer);
            io_puts(buffer, -1);
            io_puts("/", -1);
            rtl_ultoa_b10(nmea.data.date % 100, buffer);
            io_puts(buffer, -1);
            io_puts("\r\n", -1);
            io_puts("TIME: ", -1);
            rtl_ultoa(nmea.data.utc/10000000, buffer, 23, RTL_ULTOA_LEADING_ZEROS);
            io_puts(buffer, -1);
            io_puts(":", -1);
            rtl_ultoa((nmea.data.utc % 10000000) / 100000, buffer, 59, RTL_ULTOA_LEADING_ZEROS);
            io_puts(buffer, -1);
            io_puts(":", -1);
            rtl_ultoa((nmea.data.utc % 100000) / 1000, buffer, 59, RTL_ULTOA_LEADING_ZEROS);
            io_puts(buffer, -1);
            io_puts(".", -1);
            rtl_ultoa(nmea.data.utc % 1000, buffer, 999, RTL_ULTOA_LEADING_ZEROS);
            io_puts(buffer, -1);
            io_puts("\r\n", -1);
            io_puts("LAT: ", -1);
            rtl_ultoa_b10(nmea.data.latitude, buffer);
            io_puts(buffer, -1);
            io_puts("\r\n", -1);
            io_puts("LON: ", -1);
            rtl_ultoa_b10(nmea.data.longitude, buffer);
            io_puts(buffer, -1);
            io_puts("\r\n", -1);
            io_puts("KPH: ", -1);
            rtl_ultoa_b10(nmea.data.speed_mph/1000, buffer);
            io_puts(buffer, -1);
            io_puts(".", -1);
            rtl_ultoa(nmea.data.speed_mph%1000, buffer, 999, RTL_ULTOA_LEADING_ZEROS);
            io_puts(buffer, -1);
        }
    }
}

int main(void)
{
    /* Initialize scheduler. */
    scheduler_init();

#ifdef IO_SERIAL
#ifdef CONFIG_FS
    /* Initialize file system. */
    fs_init();
#endif /* CONFIG_FS */

    /* Initialize serial. */
    serial_init();
#endif /* IO_SERIAL */

    /* Create a task for NMEA demo. */
    task_create(&nmea_task_cb, P_STR("NMEA"), nmea_task_stack, NMEA_STACK_SIZE, &nmea_task, (void *)(NULL), 0);
    scheduler_task_add(&nmea_task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
