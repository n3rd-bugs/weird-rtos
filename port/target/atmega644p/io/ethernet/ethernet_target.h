/*
 * ethernet_target.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#ifndef _ETHERNET_TARGET_H_
#define _ETHERNET_TARGET_H_

#include <os.h>

#ifdef CONFIG_ETHERNET
#include <ethernet_atmega644p.h>

/* Hook-up ethernet OS stack. */
#define ETHERENET_TGT_INIT  ethernet_atmega644p_init

#endif /* CONFIG_ETHERNET */

#endif /* _ETHERNET_TARGET_H_ */
