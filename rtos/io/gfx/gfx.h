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

#ifndef _GFX_H_
#define _GFX_H_

#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */

#ifdef FS_CONSOLE
#include <console.h>
#endif /* FS_CONSOLE */

#ifdef IO_GFX
#include <gfx_config.h>

/* Error code definitions. */
#define GFX_CHAR_NOT_SUPPORTED  (-1800)
#define GFX_ROW_FULL            (-1801)
#define GFX_COLUMN_FULL         (-1802)

/* Graphics device flags. */
#define GFX_FLAG_DEBUG          (0x1)

/* Debug graphics descriptor. */
extern FD gfx_fd;

/* Graphics data. */
typedef struct _gfx GFX;
struct _gfx
{
    /* Console data. */
    CONSOLE         console;

    /* Display manipulation hooks. */
    int32_t         (*display) (GFX *, const uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);
    int32_t         (*power) (GFX *, uint8_t);
    int32_t         (*clear) (GFX *);
    int32_t         (*invert) (GFX *, uint8_t);

    /* Display dimensions. */
    uint32_t        height;
    uint32_t        width;

    /* Cursor position. */
    uint32_t        cur_row;
    uint32_t        cur_column;

    /* Font data. */
    const uint8_t   *font;

    /* Font configuration. */
    int16_t         font_char_start;
    int16_t         font_char_end;
    uint8_t         font_width;
    uint8_t         font_height;

    /* Device flags. */
    uint8_t         flags;

    /* Structure padding. */
    uint8_t         pad[1];
};

/* Function prototype. */
void graphics_register(GFX *);
int32_t gfx_display(void *, const uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);

#endif /* IO_GFX */
#endif /* _GFX_H_ */
