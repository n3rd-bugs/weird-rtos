/*
 * assert.h
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

#include <os.h>

#define ASSERT_FILE_INFO

/* Error handling. */
#ifdef ASSERT_FILE_INFO
#define OS_ASSERT(raise)        {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      __FILE__, __LINE__,       \
                                                      current_task);            \
                                    }                                           \
                                }
#else
#define OS_ASSERT(raise)        {                                               \
                                    if (raise != FALSE)                         \
                                    {                                           \
                                        system_assert(raise, "", 0,             \
                                                      current_task);            \
                                    }                                           \
                                }
#endif

/* Function prototypes. */
void system_assert(int32_t, char *, uint32_t, TASK *);