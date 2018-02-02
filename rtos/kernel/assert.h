/*
 * assert.h
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

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <kernel.h>

/* Assert configuration. */
#ifdef CMAKE_BUILD
#include <assert_config.h>
#else
#define ASSERT_NONE
#define ASSERT_FILE_INFO
#endif /* CMAKE_BUILD */

/* Error handling. */
#ifdef ASSERT_NONE
#define ASSERT(raise)                    ((void)(raise));
#define ASSERT_INFO(raise, file, line)   ((void)(raise)); ((void)(file)); ((void)(line));
#else
#ifdef ASSERT_FILE_INFO
#define ASSERT(raise)           {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      __FILE__, __LINE__,       \
                                                      NULL);                    \
                                    }                                           \
                                }
#define ASSERT_INFO(raise, file, line)                                          \
                                {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      file, line,               \
                                                      NULL);                    \
                                    }                                           \
                                }
#else
#define ASSERT(raise)           {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise), "", 0,           \
                                                      NULL);                    \
                                    }                                           \
                                }
#define ASSERT_INFO(raise, file, line)                                          \
                                {                                               \
                                    if ((raise) != FALSE)                       \
                                    {                                           \
                                        system_assert((raise),                  \
                                                      "", 0,                    \
                                                      NULL);                    \
                                    }                                           \
                                }
#endif /* ASSERT_FILE_INFO */
#endif /* ASSERT_NONE */

/* Function prototypes. */
void system_assert(int32_t, char *, uint32_t, TASK *);

#endif /* _ASSERT_H_ */
