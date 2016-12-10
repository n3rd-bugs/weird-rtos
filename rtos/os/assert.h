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

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <os.h>

/* Assert configuration. */
//#define ASSERT_NONE
#define ASSERT_FILE_INFO

/* Error handling. */
#ifdef ASSERT_NONE
#define OS_ASSERT(raise)                    ((void)(raise));
#define OS_ASSERT_INFO(raise, file, line)   ((void)(raise)); ((void)(file)); ((void)(line));
#else
#ifdef ASSERT_FILE_INFO
#define OS_ASSERT(raise)        {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      __FILE__, __LINE__,       \
                                                      get_current_task());      \
                                    }                                           \
                                }
#define OS_ASSERT_INFO(raise, file, line)                                       \
                                {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      file, line,               \
                                                      get_current_task());      \
                                    }                                           \
                                }
#else
#define OS_ASSERT(raise)        {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise), "", 0,           \
                                                      get_current_task());      \
                                    }                                           \
                                }
#define OS_ASSERT_INFO(raise, file, line)                                       \
                                {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      "", 0,                    \
                                                      get_current_task());      \
                                    }                                           \
                                }
#endif /* ASSERT_FILE_INFO */
#endif /* ASSERT_NONE */

/* Function prototypes. */
void system_assert(int32_t, char *, uint32_t, TASK *);

#endif /* _ASSERT_H_ */
