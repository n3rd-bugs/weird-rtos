/*
 * usb.c
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

#ifdef CONFIG_USB
#include <usb.h>

/*
 * usb_init
 * This function will initialize USB stack.
 */
void usb_init()
{
#ifdef USB_TGT_INIT
    /* Initialize USB target. */
    USB_TGT_INIT();
#endif
} /* usb_init */

#endif /* CONFIG_USB */
