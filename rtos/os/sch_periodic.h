/*
 * sch_periodic.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _SCH_PERIODIC_H_
#define _SCH_PERIODIC_H_

#include <os.h>

#ifdef CONFIG_INCLUDE_PERIODIC_TASKS

/* Periodic scheduling class definition. */
extern SCHEDULER periodic_scheduler;

#endif /* CONFIG_INCLUDE_PERIODIC_TASKS */

#endif /* _SCH_PERIODIC_H_ */