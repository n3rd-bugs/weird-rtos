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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _NET_CONDITION_H_
#define _NET_CONDITION_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <condition.h>

/* Networking condition definitions. */
#ifndef BUILD_CMAKE
#define NET_COND_NUM_DEVICES    2
#define NET_COND_NUM_INTERNAL   7
#endif /* BUILD_CMAKE */
#define NET_COND_NUM_TOTAL      (NET_COND_NUM_DEVICES + NET_COND_NUM_INTERNAL)

/* Networking condition task data. */
#ifndef BUILD_CMAKE
#define NET_COND_STACK_SIZE     512
#endif /* BUILD_CMAKE */

/* Networking condition process function. */
typedef void NET_CONDITION_PROCESS (void *, int32_t);

/* Networking condition global data. */
typedef struct _net_condition
{
    /* Condition list. */
    SUSPEND         *suspend[NET_COND_NUM_TOTAL];
    CONDITION       *condition[NET_COND_NUM_TOTAL];
    NET_CONDITION_PROCESS *process[NET_COND_NUM_TOTAL];
    void            *data[NET_COND_NUM_TOTAL];

    /* Number of conditions on which network stack is listening. */
    uint8_t         num;

    /* Structure padding. */
    uint8_t         pad[3];

} NET_CONDITION;

/* Exported definitions. */
extern TASK net_condition_tcb;

/* Function prototypes. */
void net_condition_init(void);
void net_condition_updated(void);
void net_condition_add(CONDITION *, SUSPEND *, NET_CONDITION_PROCESS *, void *);
void net_condition_remove(CONDITION *);

#endif /* CONFIG_NET */
#endif /* _NET_CONDITION_H_ */
