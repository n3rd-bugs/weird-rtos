/*
 * ppp_stm32f407.c
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

/* PPP device. */
static PPP usart1_ppp;

/*
 * ppp_stm32f407_init
 * This will initialize PPP interface(s) for this target.
 */
void ppp_stm32f407_init(void)
{
    FD fd;

    /* Open USART1 to be registered with PPP. */
    fd = fs_open("\\console\\usart1", 0);

    /* Register this PPP device. */
    ppp_register_fd(&usart1_ppp, fd, FALSE);

} /* ppp_stm32f407_init */

#endif /* CONFIG_PPP */
