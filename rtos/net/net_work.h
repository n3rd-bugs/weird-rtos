/*
 * net_work.c
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

#ifdef CONFIG_NET
#include <net.h>

/* Callback function to perform some work. */
typedef int32_t WORK_DO(void *param);

/* This defines a single work. */
typedef struct _work
{
    /* Function to call to perform the work. */
    WORK_DO     *work;

    /* Data to be passed to work. */
    void        *data;

    /* Condition to suspend on this work. */
    CONDITION   condition;

    /* Next work in queue. */
    WORK_DO     *next;

} WORK;

/* Work queue list. */
typedef struct _work_queue
{
#ifdef CONFIG_SEMAPHORE
    /* Lock to protect this work queue. */
    SEMAPHORE   lock;
#endif

    /* Suspend data for the work queue. */
    SUSPEND     suspend;
    CONDITION   condition;

    /* Work list. */
    struct _work_list
    {
        WORK    *head;
        WORK    *tail;
    } list;

} WORK_QUEUE;

/* Function prototypes. */
void net_work_init(WORK_QUEUE *);
int32_t net_work_add(WORK_QUEUE *, WORK *, WORK_DO *, void *, uint32_t);

#endif /* CONFIG_NET */
