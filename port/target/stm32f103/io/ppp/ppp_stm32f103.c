/*
 * ppp_stm32f103.c
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

#ifdef CONFIG_PPP
#include <ppp.h>
#include <ppp_target.h>
#ifdef CONFIG_SERIAL
#include <usart_stm32f103.h>

/* PPP over serial device data. */
static PPP ppp_usart2;
static STM32_USART usart2;
FS_BUFFER_DATA usart2_buffer_data;
static uint8_t usart2_buffer_space[PPP_MAX_BUFFER_SIZE * PPP_NUM_BUFFERS];
static FS_BUFFER usart2_buffer_ones[PPP_NUM_BUFFERS];
static FS_BUFFER_LIST usart2_buffer_lists[PPP_NUM_BUFFER_LIST];
#endif /* CONFIG_SERIAL */

/*
 * ppp_stm32f103_init
 * This will initialize PPP interface(s) for this target.
 */
void ppp_stm32f103_init(void)
{
#ifdef CONFIG_SERIAL
    FD fd;

    /* Register this serial device. */
    usart2_buffer_data.buffer_space = usart2_buffer_space;
    usart2_buffer_data.buffer_size = PPP_MAX_BUFFER_SIZE;
    usart2_buffer_data.buffer_ones = usart2_buffer_ones;
    usart2_buffer_data.num_buffer_ones = PPP_NUM_BUFFERS;
    usart2_buffer_data.buffer_lists = usart2_buffer_lists;
    usart2_buffer_data.num_buffer_lists = PPP_NUM_BUFFER_LIST;
    usart2_buffer_data.threshold_buffers = PPP_THRESHOLD_BUFFER;
    usart2_buffer_data.threshold_lists = PPP_THRESHOLD_BUFFER_LIST;
    usart_stm32f103_register(&usart2, "usart2", 2, PPP_BAUD_RATE, &usart2_buffer_data, TRUE, FALSE);

    /* Open USART2 to be registered with PPP. */
    fd = fs_open("\\console\\usart2", 0);

    /* Register this PPP device. */
    ppp_register_fd(&ppp_usart2, fd, TRUE);
#endif /* CONFIG_SERIAL */
} /* ppp_stm32f103_init */
#endif /* CONFIG_PPP */
