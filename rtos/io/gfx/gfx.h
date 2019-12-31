/*
 * gfx.h
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

#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */

#ifdef FS_CONSOLE
#include <console.h>
#endif /* FS_CONSOLE */

#ifdef CONFIG_GFX

/* Graphics data. */
typedef struct _gfx GFX;
struct _gfx
{
    /* Console data. */
    CONSOLE     console;

    /* Display manipulation hooks. */
    int32_t     (*display) (GFX *, uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);
    int32_t     (*power) (GFX *, uint8_t);
    int32_t     (*clear) (GFX *);
    int32_t     (*invert) (GFX *, uint8_t);

    /* Display dimensions. */
    uint32_t    height;
    uint32_t    width;
};

/* Function prototype. */
void graphics_register(GFX *);

#endif /* CONFIG_GFX */
