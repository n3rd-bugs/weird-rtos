/*
 * ethernet.h
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
#ifndef _ETHERNET_H_
#define _ETHERNET_H_
#include <os.h>

#ifdef CONFIG_ETHERNET

/* Include ethernet target configurations. */
#include <ethernet_target.h>

/* Function prototypes. */
void ethernet_init();

#endif /* CONFIG_ETHERNET */

#endif /* _ETHERNET_H_ */
