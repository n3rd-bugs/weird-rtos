/*
 * pstring.h
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
#ifndef P_STR_LEN
#define P_STR_LEN                   strlen
#endif
#ifndef P_MEM_CPY
#define P_MEM_CPY                   memcpy
#endif
#ifndef PROGMEM
#define PROGMEM
#endif

#endif /* _PSTRING_H_ */
