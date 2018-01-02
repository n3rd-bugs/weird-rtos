/*
 * condition.h
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
#ifndef _CONDITION_H_
#define _CONDITION_H_

/* Error codes. */
#define CONDITION_TIMEOUT           -600

/* Condition flags. */
#define CONDITION_PING              0x01

/* Suspend definitions. */
#define SUSPEND_INVALID_PRIORITY    (255)
#define SUSPEND_MIN_PRIORITY        (254)

/* User call back to check if this task satisfy the criteria. */
typedef uint8_t CONDITION_DO_RESUME (void *, void *);
typedef uint8_t CONDITION_DO_SUSPEND (void *, void *);
typedef void CONDITION_LOCK (void *);
typedef void CONDITION_UNLOCK (void *);

/* Resume data. */
typedef struct _resume
{
    /* Function that will be called to see if we need to resume. */
    CONDITION_DO_RESUME     *do_resume;

    void        *param;     /* User defined criteria. */
    int32_t     status;     /* Status needed to be returned to the task. */
} RESUME;

/* Suspend data. */
typedef struct _suspend SUSPEND;
struct _suspend
{
    /* Suspend link list member. */
    SUSPEND     *next;

    /* System tick at which we will resume this condition. */
    uint32_t    timeout;

    TASK        *task;      /* Task suspended on this. */
    void        *param;     /* User defined criteria for the tasks. */

    /* Flag to specify if the timer is enabled. */
    uint8_t     timeout_enabled;

    /* Priority for this suspend. */
    uint8_t     priority;

    /* Priority for this suspend. */
    uint8_t     may_resume;

    /* Structure padding. */
    uint8_t     pad[1];
};

/* Link list of suspend. */
typedef struct _suspend_list
{
    /* Link-list of the suspend on this condition. */
    SUSPEND     *head;
    SUSPEND     *tail;

} SUSPEND_LIST;

/* Condition data. */
typedef struct _condition
{
    /* Link-list of the suspend on this condition. */
    SUSPEND_LIST            suspend_list;

    /* Function that will be called to see if we need to suspend. */
    CONDITION_DO_SUSPEND    *do_suspend;

    /* Function that will be called to get lock for condition. */
    CONDITION_LOCK          *lock;

    /* Function that will be called to release lock for this condition. */
    CONDITION_UNLOCK        *unlock;

    /* Private data that will be passed to the lock, unlock and suspend check
     * APIs.  */
    void                    *data;

    /* Condition status, tasks resuming from this condition will have this
     * status returned to them. */
    int32_t                 status;

    /* Condition flags. */
    uint8_t                 flags;

    /* Structure padding. */
    uint8_t                 pad[3];

} CONDITION;

/* Function prototypes. */
int32_t suspend_condition(CONDITION **, SUSPEND **, uint8_t *, uint8_t);
void resume_condition(CONDITION *, RESUME *, uint8_t);

#endif /* _CONDITION_H_ */
