/*
 * interrupts.c
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#include <avr/interrupt.h>
#include <os.h>
#include <os_avr.h>

/* Helper macro to stub an interrupt. */
#define AVR_STUB_IRQ(num)   \
    ISR(_VECTOR(num))       \
    {                       \
        ;                   \
    }

/* These needs to be configured according to the required interrupts 
* in the system. */
//#define AVR_STUB_IRQ_1
#define AVR_STUB_IRQ_2
#define AVR_STUB_IRQ_3
#define AVR_STUB_IRQ_4
#define AVR_STUB_IRQ_5
#define AVR_STUB_IRQ_6
#define AVR_STUB_IRQ_7
#define AVR_STUB_IRQ_8
#define AVR_STUB_IRQ_9
#define AVR_STUB_IRQ_10
#define AVR_STUB_IRQ_11
#define AVR_STUB_IRQ_12
//#define AVR_STUB_IRQ_13
#define AVR_STUB_IRQ_14
#define AVR_STUB_IRQ_15
//#define AVR_STUB_IRQ_16
#define AVR_STUB_IRQ_17
#define AVR_STUB_IRQ_18
#define AVR_STUB_IRQ_19
#define AVR_STUB_IRQ_20
#define AVR_STUB_IRQ_21
#define AVR_STUB_IRQ_22
#define AVR_STUB_IRQ_23
//#define AVR_STUB_IRQ_24
#define AVR_STUB_IRQ_25
#define AVR_STUB_IRQ_26
#define AVR_STUB_IRQ_27
#define AVR_STUB_IRQ_28
#define AVR_STUB_IRQ_29
#define AVR_STUB_IRQ_30

/* Stub all the required interrupts. */
#ifdef AVR_STUB_IRQ_1
AVR_STUB_IRQ(1);
#endif

#ifdef AVR_STUB_IRQ_2
AVR_STUB_IRQ(2);
#endif

#ifdef AVR_STUB_IRQ_3
AVR_STUB_IRQ(3);
#endif

#ifdef AVR_STUB_IRQ_4
AVR_STUB_IRQ(4);
#endif

#ifdef AVR_STUB_IRQ_5
AVR_STUB_IRQ(5);
#endif

#ifdef AVR_STUB_IRQ_6
AVR_STUB_IRQ(6);
#endif

#ifdef AVR_STUB_IRQ_7
AVR_STUB_IRQ(7);
#endif

#ifdef AVR_STUB_IRQ_8
AVR_STUB_IRQ(8);
#endif

#ifdef AVR_STUB_IRQ_9
AVR_STUB_IRQ(9);
#endif

#ifdef AVR_STUB_IRQ_10
AVR_STUB_IRQ(10);
#endif

#ifdef AVR_STUB_IRQ_11
AVR_STUB_IRQ(11);
#endif

#ifdef AVR_STUB_IRQ_12
AVR_STUB_IRQ(12);
#endif

#ifdef AVR_STUB_IRQ_13
AVR_STUB_IRQ(13);
#endif

#ifdef AVR_STUB_IRQ_14
AVR_STUB_IRQ(14);
#endif

#ifdef AVR_STUB_IRQ_15
AVR_STUB_IRQ(15);
#endif

#ifdef AVR_STUB_IRQ_16
AVR_STUB_IRQ(16);
#endif

#ifdef AVR_STUB_IRQ_17
AVR_STUB_IRQ(17);
#endif

#ifdef AVR_STUB_IRQ_18
AVR_STUB_IRQ(18);
#endif

#ifdef AVR_STUB_IRQ_19
AVR_STUB_IRQ(19);
#endif

#ifdef AVR_STUB_IRQ_20
AVR_STUB_IRQ(20);
#endif

#ifdef AVR_STUB_IRQ_21
AVR_STUB_IRQ(21);
#endif

#ifdef AVR_STUB_IRQ_22
AVR_STUB_IRQ(22);
#endif

#ifdef AVR_STUB_IRQ_23
AVR_STUB_IRQ(23);
#endif

#ifdef AVR_STUB_IRQ_24
AVR_STUB_IRQ(24);
#endif

#ifdef AVR_STUB_IRQ_25
AVR_STUB_IRQ(25);
#endif

#ifdef AVR_STUB_IRQ_26
AVR_STUB_IRQ(26);
#endif

#ifdef AVR_STUB_IRQ_27
AVR_STUB_IRQ(27);
#endif

#ifdef AVR_STUB_IRQ_28
AVR_STUB_IRQ(28);
#endif

#ifdef AVR_STUB_IRQ_29
AVR_STUB_IRQ(29);
#endif

#ifdef AVR_STUB_IRQ_30
AVR_STUB_IRQ(30);
#endif
