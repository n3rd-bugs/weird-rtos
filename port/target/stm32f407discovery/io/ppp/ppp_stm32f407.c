/*
 * ppp_stm32f407.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
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
