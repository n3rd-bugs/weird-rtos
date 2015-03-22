/*
 * ppp_fcs.h
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
#ifndef _PPP_FCS_H_
#define _PPP_FCS_H_

#include <os.h>

#ifdef CONFIG_PPP

/* FCS magic and initial value definitions. */
#define PPP_FCS16_INIT      0xffff
#define PPP_FCS16_MAGIC     0xf0b8

#define PPP_FCS16_IS_VALID(b)    (ppp_fcs16_buffer_calculate(b, PPP_FCS16_INIT) == PPP_FCS16_MAGIC)

/* Function prototypes. */
uint16_t ppp_fcs16_calculate(char *, uint32_t, uint16_t);
uint16_t ppp_fcs16_buffer_calculate(FS_BUFFER *, uint16_t);

#endif /* CONFIG_PPP */

#endif /* _PPP_FCS_H_ */
