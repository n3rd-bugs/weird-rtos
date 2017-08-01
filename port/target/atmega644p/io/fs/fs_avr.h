/*
 * fs_avr.h
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
#ifndef _FS_AVR_H_
#define _FS_AVR_H_

#include <kernel.h>

#ifdef CONFIG_FS

/* Function prototypes. */
void fs_avr_init(void);

#endif /* CONFIG_FS */
#endif /* _FS_AVR_H_ */
