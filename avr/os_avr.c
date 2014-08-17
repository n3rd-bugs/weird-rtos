/*
 * os_avr.c
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
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

/*
 * ISR(TIMER1_COMPA_vect, ISR_NAKED)
 * This is timer interrupt that will be called at each system tick.
 */
ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    /* Save the context on the current task's stack. */
    /* This will also disable global interrupts. */
    SAVE_CONTEXT();

    /* Process system tick. */
    os_process_system_tick();

    /* Restore the previous task's context. */
    RESTORE_CONTEXT();

    /* Return and enable global interrupts. */
    RETURN_ENABLING_INTERRUPTS();

} /* ISR(TIMER1_COMPA_vect, ISR_NAKED) */

/*
 * system_tick_Init
 * This initializes system tick.
 * On AVR we are using Timer1 with CCP1 module to reload the timer when expired.
 */
void system_tick_Init()
{
    /* Using 16bit timer 1 to generate the system tick. */
    OCR1AH = (((SYS_FREQ / OS_TICKS_PER_SEC / 64) - 1) & 0xFF00) >> 8;
    OCR1AL = (((SYS_FREQ / OS_TICKS_PER_SEC / 64) - 1) & 0x00FF);

    /* Setup clock source and compare match behavior. */
    TCCR1B =  0x03 | 0x08;

    /* Enable the compare A match interrupt. */
    TIMSK1 |= 0x02;

} /* system_tick_Init */
