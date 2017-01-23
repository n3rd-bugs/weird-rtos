/*
 * condition.h
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
#ifndef _CONDITION_H_
#define _CONDITION_H_

/* Error codes. */
#define CONDITION_TIMEOUT               -600

/* Suspend flags. */
#define SUSPEND_PRIORITY                0x01
#define SUSPEND_TIMER                   0x02

/* Condition flags. */
#define CONDITION_LOCK_NO_SUSPEND       0x01
#define CONDITION_PING                  0x02

/* Helper macros. */
#define CONDITION_INVALID               ((uint32_t)-1)

/* User call back to check if this task satisfy the criteria. */
typedef uint8_t CONDITION_DO_RESUME (void *, void *);
typedef uint8_t CONDITION_DO_SUSPEND (void *, void *);
typedef void CONDITION_UNLOCK (void *);
typedef void CONDITION_LOCK (void *);

/* Resume data. */
typedef struct _resume
{
    /* Function that will be called to see if we need to resume. */
    CONDITION_DO_RESUME *do_resume;

    void        *param;     /* User defined criteria. */
    int32_t     status;     /* Status needed to be returned to the task. */
} RESUME;

/* Suspend data. */
typedef struct _suspend SUSPEND;
struct _suspend
{
    /* System tick at which we will resume or number of ticks we would
     * want to wait. */
    uint64_t    timeout;

    /* Suspend link list member. */
    SUSPEND     *next;

    /* Function that will be called to see if we need to suspend. */
    CONDITION_DO_SUSPEND *do_suspend;

    TASK        *task;      /* Task suspended on this. */
    void        *param;     /* User defined criteria for the tasks. */
    uint32_t    flags;      /* Suspend flags. */
    uint8_t     pad[4];     /* Structure padding. */
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
    SUSPEND_LIST        suspend_list;

    /* Function that will be called to get lock for condition. */
    CONDITION_LOCK      *lock;

    /* Function that will be called to release lock for this condition. */
    CONDITION_UNLOCK    *unlock;

    /* Private data that will be passed to the lock, unlock and suspend check
     * APIs.  */
    void                *data;

    /* Condition flags. */
    uint32_t            flags;

} CONDITION;

/* Function prototypes. */
int32_t suspend_condition(CONDITION **, SUSPEND **, uint32_t *, uint8_t);
void resume_condition(CONDITION *, RESUME *, uint8_t);

#endif /* _CONDITION_H_ */
