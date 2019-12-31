/*
 * gfx.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_GFX
#include <gfx.h>

/*
 * graphics_register
 * @gfx: Graphics data.
 * This function will register a graphics interface.
 */
void graphics_register(GFX *gfx)
{
    int32_t status;

    /* Clear the display. */
    status = gfx->clear(gfx);

    if (status == SUCCESS)
    {
        /* Power on the display. */
        status = gfx->power(gfx, TRUE);
    }

    if (status == SUCCESS)
    {

        /* Register a console driver for this. */
        console_register(&(gfx->console));
    }

} /* graphics_register */

#endif /* CONFIG_GFX */
