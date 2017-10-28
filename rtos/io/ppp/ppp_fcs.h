/*
 * ppp_fcs.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _PPP_FCS_H_
#define _PPP_FCS_H_

#include <kernel.h>

#ifdef CONFIG_PPP
#include <fs.h>

/* FCS magic and initial value definitions. */
#define PPP_FCS16_INIT      0xffff
#define PPP_FCS16_MAGIC     0xf0b8

#define PPP_FCS16_IS_VALID(b)    (ppp_fcs16_buffer_calculate(b, PPP_FCS16_INIT) == PPP_FCS16_MAGIC)

/* Function prototypes. */
uint16_t ppp_fcs16_calculate(uint8_t *, uint32_t, uint16_t);
uint16_t ppp_fcs16_buffer_calculate(FS_BUFFER *, uint16_t);

#endif /* CONFIG_PPP */

#endif /* _PPP_FCS_H_ */
