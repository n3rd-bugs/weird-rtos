/*
 * spi_demo.c
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
#include <os.h>
#include <string.h>
#include <mem.h>
#include <spi.h>

/* Function prototypes. */
void spi_demo_task(void *);

void spi_demo_task(void *argv)
{
    SPI_DEVICE device;
    uint8_t data[10];

    UNUSED_PARAM(argv);

    memset(&device, 0, sizeof(SPI_DEVICE));

    device.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);
    device.baudrate = 1122;

    device.data.device_num = 1;

    /* Initialize this SPI device. */
    SPI_TGT_INIT(&device);

    /* Write some data on the SPI device. */
    data[0] = 0x40 | 0x1B;
    data[1] = 0xAA;
    spi_write_read(&device, data, 2);

    /* Read back the same data. */
    data[0] = 0x00 | 0x1B;
    spi_write_read(&device, data, 2);

}

int main(void)
{
    TASK *spi_demo_task_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize memory. */
    mem_init();

    /* Create a task for SPI demo. */
    spi_demo_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(spi_demo_task_cb, "SPI", (uint8_t *)(spi_demo_task_cb + 1), 4096, &spi_demo_task, (void *)(NULL), 0);
    scheduler_task_add(spi_demo_task_cb, TASK_APERIODIC, 5, 0);

    /* Run scheduler. */
    os_run();

    return (0);

}
