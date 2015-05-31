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

/* Global variables. */
SPI_DEVICE spi_device;
SPI_MSG spi_mesg;
uint8_t spi_data[10];

/* Demo task stack. */
uint8_t demo_stack[256];

void spi_demo_task(void *argv)
{
    UNUSED_PARAM(argv);

    /* Clear the device structures. */
    memset(&spi_device, 0, sizeof(SPI_DEVICE));
    memset(&spi_data, 'A', 10);

    /* Initialize SPI device configuration. */
    spi_device.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);
    spi_device.baudrate = 10000000;

    /* Initialize this SPI device. */
    SPI_TGT_INIT(&spi_device);

    /* Initialize SPI message. */
    spi_mesg.buffer = spi_data;
    spi_mesg.length = 10;
    spi_mesg.flags = SPI_MSG_WRITE;

    /* Send a write message on SPI. */
    spi_message(&spi_device, &spi_mesg, 1);
}

int main(void)
{
    TASK spi_demo_task_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Create a task for SPI demo. */
    task_create(&spi_demo_task_cb, "SPI", demo_stack, 256, &spi_demo_task, (void *)(NULL), 0);
    scheduler_task_add(&spi_demo_task_cb, TASK_APERIODIC, 5, 0);

    /* Run scheduler. */
    os_run();

    return (0);

}
