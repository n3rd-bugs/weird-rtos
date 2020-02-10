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

#ifdef IO_GFX
#include <gfx.h>
#include <string.h>

/* Graphics debug file descriptor. */
FD gfx_fd = NULL;

/* Internal function prototypes. */
static int32_t gfx_write(void *, const uint8_t *, int32_t);

/*
 * graphics_register
 * @gfx: Graphics data.
 * This function will register a graphics interface.
 */
void graphics_register(GFX *gfx)
{
    int32_t status;
    char fs_name[64] = "\\console\\";

    /* Clear the display. */
    status = gfx->clear(gfx);

    if (status == SUCCESS)
    {
        /* Power on the display. */
        status = gfx->power(gfx, TRUE);
    }

    if (status == SUCCESS)
    {
        /* Initialize cursor position. */
        gfx->cur_column = gfx->cur_row = 0;

        /* Register a console driver for this. */
        gfx->console.fs.write = &gfx_write;
        console_register(&(gfx->console));

        /* There is always some space available for data to be sent. */
        gfx->console.fs.flags |= FS_SPACE_AVAILABLE;

        /* If this is a debug device. */
        if (gfx->flags & GFX_FLAG_DEBUG)
        {
            /* Open this as debug descriptor. */
            strncat(fs_name, gfx->console.fs.name, (64 - sizeof("\\console\\")));
            gfx_fd = fs_open(fs_name, 0);
        }
    }

} /* graphics_register */

/*
 * gfx_display
 * @priv_data: Graphics data.
 * @buffer: Buffer to display.
 * @col: Starting column.
 * @num_col: Number of columns.
 * @row: Starting row.
 * @num_row: Number of rows.
 * @return: Success will be returned if the given data was successfully displayed.
 * This function will display a buffer on the display.
 */
int32_t gfx_display(void *priv_data, const uint8_t *buffer, uint32_t col, uint32_t num_col, uint32_t row, uint32_t num_row)
{
    int32_t status;
    GFX *gfx = (GFX *)priv_data;

    /* Display the given buffer. */
    status = gfx->display(gfx, buffer, col, num_col, row, num_row);

    /* Return status to the caller. */
    return (status);

} /* gfx_display */

/*
 * gfx_write
 * @priv_data: GFX device data for which this was called.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * @return: Number of bytes will be returned if write was successful,
 *  GFX_ROW_FULL will be returned if there is no more rows to print the buffer,
 *  GFX_COLUMN_FULL will be returned if there is no more columns to print
 *      the buffer,
 *  GFX_CHAR_NOT_SUPPORTED will be returned if an unsupported character was given.
 * This function prints a string on a graphics display.
 */
static int32_t gfx_write(void *priv_data, const uint8_t *buf, int32_t nbytes)
{
    GFX *gfx = (GFX *)priv_data;
    int32_t to_print = nbytes;
    int32_t status = SUCCESS;
    uint32_t indent_size;

    /* While we have some data to be printed. */
    while ((status == SUCCESS) && (nbytes > 0))
    {
        /* Put this character on the display. */
        switch (*buf)
        {

        /* Handle clear screen. */
        case '\f':

            /* Clear display. */
            status = gfx->clear(gfx);

            /* If display was successfully cleared. */
            if (status == SUCCESS)
            {
                /* Reset the cursor location. */
                gfx->cur_column = gfx->cur_row = 0;
            }

            break;

        /* Handle carriage return. */
        case '\r':

            /* Move the current column. */
            gfx->cur_column = 0;

            break;

        /* Handle new line. */
        case '\n':

            /* If we are not at the last row. */
            if ((gfx->cur_row + ((uint32_t)gfx->font_height * 2)) <= gfx->height)
            {
                /* Move cursor to next row. */
                gfx->cur_row += gfx->font_height;
            }
            else
            {
                /* No more rows on the display. */
                status = GFX_ROW_FULL;
            }

            break;

        /* Handle tab. */
        case '\t':

            /* Calculate the indent size. */
            indent_size = (GFX_TAB_SIZE - ((gfx->cur_column / gfx->font_width) % GFX_TAB_SIZE));

            /* Check if we can add required indentation. */
            if ((gfx->cur_column + (indent_size * gfx->font_width) + gfx->font_width) <= gfx->width)
            {
                /* Move the cursor to required column. */
                gfx->cur_column = (uint16_t)(gfx->cur_column + (indent_size * gfx->font_width));
            }
            else
            {
                /* No more space in the column for indentation. */
                status = GFX_COLUMN_FULL;
            }

            break;

        /* Normal ASCII character. */
        default:

            /* If given character is supported. */
            if ((*buf >= gfx->font_char_start) && (*buf <= gfx->font_char_end))
            {
                /* Check if we can add a new character. */
                if ((gfx->cur_column + ((uint32_t)gfx->font_width * 2)) <= gfx->width)
                {
                    /* Display the given character. */
                    status = gfx->display(gfx, &gfx->font[(*buf - gfx->font_char_start) * gfx->font_width * (gfx->font_height / 8)], gfx->cur_column, gfx->font_width, gfx->cur_row, gfx->font_height);

                    /* Move the cursor. */
                    gfx->cur_column += gfx->font_width;
                }
                else
                {
                    /* No more rows on the display. */
                    status = GFX_ROW_FULL;
                }
            }
            else
            {
                /* Character not supported. */
                status = GFX_CHAR_NOT_SUPPORTED;
            }

            break;
        }

        /* Decrement number of bytes remaining. */
        nbytes --;

        /* Move forward in the buffer. */
        buf++;
    }

    /* Return number of bytes printed. */
    return ((status == SUCCESS) ? (to_print - nbytes) : (status));

} /* gfx_write */

#endif /* IO_GFX */
