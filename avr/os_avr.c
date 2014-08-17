#include <avr/interrupt.h>
#include <os.h>
#include <os_avr.h>

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

void system_tick_Init()
{
    /* Using 16bit timer 1 to generate the system tick. */
    OCR1AH = (((SYS_FREQ / OS_TICKS_PER_SEC / 64) - 1) & 0xFF00) >> 8;
    OCR1AL = (((SYS_FREQ / OS_TICKS_PER_SEC / 64) - 1) & 0x00FF);

    /* Setup clock source and compare match behaviour. */
    TCCR1B =  0x03 | 0x08;

    /* Enable the compare A match interrupt. */
    TIMSK1 |= 0x02;

} /* system_tick_Init */
