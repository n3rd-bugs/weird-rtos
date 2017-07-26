/*
 * pstring.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _PSTRING_H_
#define _PSTRING_H_

/* In program memory data manipulation APIs. */
#ifndef P_STR
#define P_STR(s)                    (s)
#endif
#ifndef P_STR_T
#define P_STR_T                     const char *
#endif
#ifndef P_STR_MEM
#define P_STR_MEM
#endif
#ifndef P_STR_CPY
#define P_STR_CPY                   strcpy
#endif
#ifndef P_STR_NCPY
#define P_STR_NCPY                  strncpy
#endif

#endif /* _PSTRING_H_ */
