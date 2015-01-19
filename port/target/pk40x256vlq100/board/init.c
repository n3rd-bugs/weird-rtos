/*
 * init.c
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

#include <isr.h>
#include <os.h>

/*
 * wdt_disbale
 * This function disables the WDT.
 */
void wdt_disbale()
{
    /* Disable the WDOG module */
    WDOG_UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xC520);
    WDOG_UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xD928);
    WDOG_STCTRLH = WDOG_STCTRLH_BYTESEL(0x00) |
                   WDOG_STCTRLH_WAITEN_MASK |
                   WDOG_STCTRLH_STOPEN_MASK |
                   WDOG_STCTRLH_ALLOWUPDATE_MASK |
                   WDOG_STCTRLH_CLKSRC_MASK |
                   0x0100U;

} /* wdt_disbale */

/*
 * sysclock_init
 * This function initializes system clock.
 */
void sysclock_init()
{
    /* If the internal load capacitors are being used, they should be selected
     * before enabling the oscillator. Application specific. 16pF and 8pF
     * selected in this example */
    OSC_CR = OSC_CR_SC16P_MASK | OSC_CR_SC8P_MASK;

    /* Enabling the oscillator for 8 MHz crystal
     * RANGE=1, should be set to match the frequency of the crystal being used.
     * HGO=1, high gain is selected, provides better noise immunity but
     * does draw higher current.
     * EREFS=1, enable the external oscillator.
     * LP=0, low power mode not selected (not actually part of osc setup)
     * IRCS=0, slow internal ref clock selected (not actually part of osc setup) */
    MCG_C2 = MCG_C2_RANGE(1) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK;

    /* Select ext oscillator, reference divider and clear IREFS to start ext osc
     * CLKS=2, select the external clock source
     * FRDIV=3, set the FLL ref divider to keep the ref clock in range
     *      (even if FLL is not being used) 8 MHz / 256 = 31.25 kHz
     * IREFS=0, select the external clock.
     * IRCLKEN=0, disable IRCLK (can enable it if desired).
     * IREFSTEN=0, disable IRC in stop mode.
     *      (can keep it enabled in stop if desired) */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);

    /* Wait for oscillator to initialize. */
    while (!(MCG_S & MCG_S_OSCINIT_MASK))
    {
        ;
    }

    /* Wait for Reference clock to switch to external reference. */
    while (MCG_S & MCG_S_IREFST_MASK)
    {
        ;
    }

    /* Wait for MCGOUT to switch over to the external reference clock. */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2)
    {
        ;
    }

    /* Now configure the PLL and move to PBE mode set the PRDIV field to
     * generate a 4MHz reference clock (8MHz / 2).
     * PRDIV=1 selects a divide by 2. */
    MCG_C5 = MCG_C5_PRDIV(1);

    /* Set the VDIV field to 0, which is x24, giving 4 x 24  = 96 MHz
     * the PLLS bit is set to enable the PLL the clock monitor is enabled,
     * CME=1 to cause a reset if crystal fails LOLIE can be optionally set
     * to enable the loss of lock interrupt */
    MCG_C6 = MCG_C6_CME_MASK | MCG_C6_PLLS_MASK | MCG_C6_VDIV(0);

    /* Wait until the source of the PLLS clock has switched to the PLL. */
    while (!(MCG_S & MCG_S_PLLST_MASK))
    {
        ;
    }

    /* Wait until the PLL has achieved lock. */
    while (!(MCG_S & MCG_S_LOCK_MASK))
    {
        ;
    }

    /* Set up the SIM clock dividers BEFORE switching to the PLL to ensure the
     * system clock speeds are in spec.
     * core = PLL (96MHz).
     * bus = PLL/2 (48MHz).
     * flexbus = PLL/2 (48MHz).
     * flash = PLL/4 (12MHz). */
    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) |
                  SIM_CLKDIV1_OUTDIV2(1) |
                  SIM_CLKDIV1_OUTDIV3(1) |
                  SIM_CLKDIV1_OUTDIV4(2);

    /* Transition into PEE by setting CLKS to 0
     * previous MCG_C1 settings remain the same, just need to set CLKS to 0. */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for MCGOUT to switch over to the PLL. */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3)
    {
        ;
    }

    /* The USB clock divider in the System Clock Divider Register 2
     * (SIM_CLKDIV2) should be configured to generate the 48 MHz
     * USB clock before configuring the USB module.
     * Set USB divider to PLL/2 assuming reset.
     * Set state of the SIM_CLKDIV2 register. */
    SIM_CLKDIV2 |= SIM_CLKDIV2_USBDIV(1) | SIM_CLKDIV2_I2SDIV(7);

    /* Set USB clock gating. */
    SIM_SCGC4 |= (SIM_SCGC4_USBOTG_MASK);

} /* sysclock_init */

/*
 * system_entry
 * This is system entry function, this will initialize the hardware and then
 * call the user initializer.
 */
void system_entry(void)
{
    extern uint64_t current_tick;

    /* Set the interrupt vector table position. */
    SCB_VTOR = (uint32_t)(&system_isr_table);

    __asm (
    "   MOV    R0, %[sp]    \r\n"
    "   CMP    R0, #0       \r\n"
    "   BEQ    skip_sp      \r\n"
    "   mov    SP, R0       \r\n"
    "   sub    SP, #4       \r\n"
    "   mov    R0, #0       \r\n"
    "   mvn    R0, R0       \r\n"
    "   str    R0, [SP,#0]  \r\n"
    "   add    SP, #4       \r\n"
    " skip_sp:              \r\n"
    ::
    [sp] "r" (&os_stack_end));

    /* Disable watch dog timer. */
    wdt_disbale();

    /* Initialize system clock. */
    sysclock_init();

    /* Initialize system clock. */
    current_tick = 0;

    /* We are not running any task until OS initializes. */
    set_current_task(NULL);

    ENABLE_INTERRUPTS();

    /* Call application initializer. */
    (void) main();

} /* system_entry */
