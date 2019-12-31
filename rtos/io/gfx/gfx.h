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
#ifdef CMAKE_BUILD
#include <gfx_config.h>
#else
#define GFX_TAB_SIZE            (3)
#endif /* CMAKE_BUILD */

/* Error code definitions. */
#define GFX_CHAR_NOT_SUPPORTED  (-1800)
#define GFX_ROW_FULL            (-1801)
#define GFX_COLUMN_FULL         (-1802)

/* Graphics device flags. */
#define GFX_FLAG_DEBUG          (0x01)

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
    uint8_t         font_width;
    uint8_t         font_height;
    uint8_t         font_char_start;
    uint8_t         font_char_end;

    /* Device flags. */
    uint8_t         flags;

    /* Structure padding. */
    uint8_t         pad[3];
};

/* Function prototype. */
void graphics_register(GFX *);

#endif /* CONFIG_GFX */
