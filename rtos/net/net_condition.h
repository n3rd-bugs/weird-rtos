/*
 * net_condition.h
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
#ifndef _NET_CONDITION_H_
#define _NET_CONDITION_H_
#include <os.h>

#ifdef CONFIG_NET
#include <condition.h>

/* Networking condition definitions. */
#define NET_COND_NUM_DEVICES    1
#define NET_COND_NUM_INTERNAL   1
#define NET_COND_NUM_TOTAL      (NET_COND_NUM_DEVICES + NET_COND_NUM_INTERNAL)

/* Networking condition task data. */
#define NET_COND_STACK_SIZE     1024

typedef void NET_CONDITION_PROCESS ();

/* Networking condition global data. */
typedef struct _net_condition
{
    /* Condition list. */
    SUSPEND         *suspend[NET_COND_NUM_TOTAL];
    CONDITION       *condition[NET_COND_NUM_TOTAL];
    NET_CONDITION_PROCESS *process[NET_COND_NUM_TOTAL];
    void            *data[NET_COND_NUM_TOTAL];

    /* Number of conditions on which network stack is listening. */
    uint32_t    num;

} NET_CONDITION;

/* Function prototypes. */
void net_condition_init();
void net_condition_add(CONDITION *, SUSPEND *, NET_CONDITION_PROCESS *, void *);
void net_condition_remove(CONDITION *);

#endif /* CONFIG_NET */
#endif /* _NET_CONDITION_H_ */
