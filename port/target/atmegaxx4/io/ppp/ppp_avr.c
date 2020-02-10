/*
 * ppp_avr.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef IO_PPP
#include <ppp.h>
#include <ppp_target.h>
#ifdef IO_SERIAL
#include <usart_avr.h>

/* PPP over serial device data. */
static PPP ppp_usart1;
static AVR_USART usart1;
static FS_BUFFER_DATA usart1_buffer_data;
static uint8_t usart1_buffer_space[PPP_MAX_BUFFER_SIZE * PPP_NUM_BUFFERS];
static FS_BUFFER usart1_buffer_ones[PPP_NUM_BUFFERS];
static FS_BUFFER_LIST usart1_buffer_lists[PPP_NUM_BUFFER_LIST];
#endif /* IO_SERIAL */

/*
 * ppp_avr_init
 * This will initialize PPP interface(s) for this target.
 */
void ppp_avr_init(void)
{
#ifdef IO_SERIAL
    FD fd;

    /* Register this serial device. */
    usart1_buffer_data.buffer_space = usart1_buffer_space;
    usart1_buffer_data.buffer_size = PPP_MAX_BUFFER_SIZE;
    usart1_buffer_data.buffers = usart1_buffer_ones;
    usart1_buffer_data.num_buffers = PPP_NUM_BUFFERS;
    usart1_buffer_data.buffer_lists = usart1_buffer_lists;
    usart1_buffer_data.num_buffer_lists = PPP_NUM_BUFFER_LIST;
    usart1_buffer_data.threshold_buffers = PPP_THRESHOLD_BUFFER;
    usart1_buffer_data.threshold_lists = PPP_THRESHOLD_BUFFER_LIST;
    usart_avr_register(&usart1, "usart1", 1, PPP_BAUD_RATE, &usart1_buffer_data, TRUE, FALSE);

    /* Open USART1 to be registered with PPP. */
    fd = fs_open("\\console\\usart1", 0);

    /* Register this PPP device. */
    ppp_register_fd(&ppp_usart1, fd, TRUE);
#endif /* IO_SERIAL */
} /* ppp_avr_init */
#endif /* IO_PPP */
