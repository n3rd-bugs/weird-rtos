/*
 * oled_ssd1306.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_OLED
#include <oled_ssd1306.h>
#include <oled_target.h>
#include <string.h>

/* OLED SSD1306 font data. */
static const uint8_t oled_font_data[95 * 6] PROGMEM =
{
     0x0,  0x0,  0x0,  0x0,  0x0,  0x0, /* 0x20 */
     0x0,  0x6, 0x5F,  0x6,  0x0,  0x0, /* 0x21 */
     0x7,  0x3,  0x0,  0x7,  0x3,  0x0, /* 0x22 */
    0x24, 0x7E, 0x24, 0x7E, 0x24,  0x0, /* 0x23 */
    0x24, 0x2B, 0x6A, 0x12,  0x0,  0x0, /* 0x24 */
    0x63, 0x13,  0x8, 0x64, 0x63,  0x0, /* 0x25 */
    0x36, 0x49, 0x56, 0x20, 0x50,  0x0, /* 0x26 */
     0x0,  0x7,  0x3,  0x0,  0x0,  0x0, /* 0x27 */
     0x0, 0x3E, 0x41,  0x0,  0x0,  0x0, /* 0x28 */
     0x0, 0x41, 0x3E,  0x0,  0x0,  0x0, /* 0x29 */
     0x8, 0x3E, 0x1C, 0x3E,  0x8,  0x0, /* 0x2A */
     0x8,  0x8, 0x3E,  0x8,  0x8,  0x0, /* 0x2B */
     0x0, 0xE0, 0x60,  0x0,  0x0,  0x0, /* 0x2C */
     0x8,  0x8,  0x8,  0x8,  0x8,  0x0, /* 0x2D */
     0x0, 0x60, 0x60,  0x0,  0x0,  0x0, /* 0x2E */
    0x20, 0x10,  0x8,  0x4,  0x2,  0x0, /* 0x2F */
    0x3E, 0x51, 0x49, 0x45, 0x3E,  0x0, /* 0x30 */
     0x0, 0x42, 0x7F, 0x40,  0x0,  0x0, /* 0x31 */
    0x62, 0x51, 0x49, 0x49, 0x46,  0x0, /* 0x32 */
    0x22, 0x49, 0x49, 0x49, 0x36,  0x0, /* 0x33 */
    0x18, 0x14, 0x12, 0x7F, 0x10,  0x0, /* 0x34 */
    0x2F, 0x49, 0x49, 0x49, 0x31,  0x0, /* 0x35 */
    0x3C, 0x4A, 0x49, 0x49, 0x30,  0x0, /* 0x36 */
     0x1, 0x71,  0x9,  0x5,  0x3,  0x0, /* 0x37 */
    0x36, 0x49, 0x49, 0x49, 0x36,  0x0, /* 0x38 */
     0x6, 0x49, 0x49, 0x29, 0x1E,  0x0, /* 0x39 */
     0x0, 0x6C, 0x6C,  0x0,  0x0,  0x0, /* 0x3A */
     0x0, 0xEC, 0x6C,  0x0,  0x0,  0x0, /* 0x3B */
     0x8, 0x14, 0x22, 0x41,  0x0,  0x0, /* 0x3C */
    0x24, 0x24, 0x24, 0x24, 0x24,  0x0, /* 0x3D */
     0x0, 0x41, 0x22, 0x14,  0x8,  0x0, /* 0x3E */
     0x2,  0x1, 0x59,  0x9,  0x6,  0x0, /* 0x3F */
    0x3E, 0x41, 0x5D, 0x55, 0x1E,  0x0, /* 0x40 */
    0x7E, 0x11, 0x11, 0x11, 0x7E,  0x0, /* 0x41 */
    0x7F, 0x49, 0x49, 0x49, 0x36,  0x0, /* 0x42 */
    0x3E, 0x41, 0x41, 0x41, 0x22,  0x0, /* 0x43 */
    0x7F, 0x41, 0x41, 0x41, 0x3E,  0x0, /* 0x44 */
    0x7F, 0x49, 0x49, 0x49, 0x41,  0x0, /* 0x45 */
    0x7F,  0x9,  0x9,  0x9,  0x1,  0x0, /* 0x46 */
    0x3E, 0x41, 0x49, 0x49, 0x7A,  0x0, /* 0x47 */
    0x7F,  0x8,  0x8,  0x8, 0x7F,  0x0, /* 0x48 */
     0x0, 0x41, 0x7F, 0x41,  0x0,  0x0, /* 0x49 */
    0x30, 0x40, 0x40, 0x40, 0x3F,  0x0, /* 0x4A */
    0x7F,  0x8, 0x14, 0x22, 0x41,  0x0, /* 0x4B */
    0x7F, 0x40, 0x40, 0x40, 0x40,  0x0, /* 0x4C */
    0x7F,  0x2,  0x4,  0x2, 0x7F,  0x0, /* 0x4D */
    0x7F,  0x2,  0x4,  0x8, 0x7F,  0x0, /* 0x4E */
    0x3E, 0x41, 0x41, 0x41, 0x3E,  0x0, /* 0x4F */
    0x7F,  0x9,  0x9,  0x9,  0x6,  0x0, /* 0x50 */
    0x3E, 0x41, 0x51, 0x21, 0x5E,  0x0, /* 0x51 */
    0x7F,  0x9,  0x9, 0x19, 0x66,  0x0, /* 0x52 */
    0x26, 0x49, 0x49, 0x49, 0x32,  0x0, /* 0x53 */
     0x1,  0x1, 0x7F,  0x1,  0x1,  0x0, /* 0x54 */
    0x3F, 0x40, 0x40, 0x40, 0x3F,  0x0, /* 0x55 */
    0x1F, 0x20, 0x40, 0x20, 0x1F,  0x0, /* 0x56 */
    0x3F, 0x40, 0x3C, 0x40, 0x3F,  0x0, /* 0x57 */
    0x63, 0x14,  0x8, 0x14, 0x63,  0x0, /* 0x58 */
     0x7,  0x8, 0x70,  0x8,  0x7,  0x0, /* 0x59 */
    0x71, 0x49, 0x45, 0x43,  0x0,  0x0, /* 0x5A */
     0x0, 0x7F, 0x41, 0x41,  0x0,  0x0, /* 0x5B */
     0x2,  0x4,  0x8, 0x10, 0x20,  0x0, /* 0x5C */
     0x0, 0x41, 0x41, 0x7F,  0x0,  0x0, /* 0x5D */
     0x4,  0x2,  0x1,  0x2,  0x4,  0x0, /* 0x5E */
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, /* 0x5F */
     0x0,  0x3,  0x7,  0x0,  0x0,  0x0, /* 0x60 */
    0x20, 0x54, 0x54, 0x54, 0x78,  0x0, /* 0x61 */
    0x7F, 0x44, 0x44, 0x44, 0x38,  0x0, /* 0x62 */
    0x38, 0x44, 0x44, 0x44, 0x28,  0x0, /* 0x63 */
    0x38, 0x44, 0x44, 0x44, 0x7F,  0x0, /* 0x64 */
    0x38, 0x54, 0x54, 0x54,  0x8,  0x0, /* 0x65 */
     0x8, 0x7E,  0x9,  0x9,  0x0,  0x0, /* 0x66 */
    0x18, 0xA4, 0xA4, 0xA4, 0x7C,  0x0, /* 0x67 */
    0x7F,  0x4,  0x4, 0x78,  0x0,  0x0, /* 0x68 */
     0x0,  0x0, 0x7D, 0x40,  0x0,  0x0, /* 0x69 */
    0x40, 0x80, 0x84, 0x7D,  0x0,  0x0, /* 0x6A */
    0x7F, 0x10, 0x28, 0x44,  0x0,  0x0, /* 0x6B */
     0x0,  0x0, 0x7F, 0x40,  0x0,  0x0, /* 0x6C */
    0x7C,  0x4, 0x18,  0x4, 0x78,  0x0, /* 0x6D */
    0x7C,  0x4,  0x4, 0x78,  0x0,  0x0, /* 0x6E */
    0x38, 0x44, 0x44, 0x44, 0x38,  0x0, /* 0x6F */
    0xFC, 0x44, 0x44, 0x44, 0x38,  0x0, /* 0x70 */
    0x38, 0x44, 0x44, 0x44, 0xFC,  0x0, /* 0x71 */
    0x44, 0x78, 0x44,  0x4,  0x8,  0x0, /* 0x72 */
     0x8, 0x54, 0x54, 0x54, 0x20,  0x0, /* 0x73 */
     0x4, 0x3E, 0x44, 0x24,  0x0,  0x0, /* 0x74 */
    0x3C, 0x40, 0x20, 0x7C,  0x0,  0x0, /* 0x75 */
    0x1C, 0x20, 0x40, 0x20, 0x1C,  0x0, /* 0x76 */
    0x3C, 0x60, 0x30, 0x60, 0x3C,  0x0, /* 0x77 */
    0x6C, 0x10, 0x10, 0x6C,  0x0,  0x0, /* 0x78 */
    0x9C, 0xA0, 0x60, 0x3C,  0x0,  0x0, /* 0x79 */
    0x64, 0x54, 0x54, 0x4C,  0x0,  0x0, /* 0x7A */
     0x8, 0x3E, 0x41, 0x41,  0x0,  0x0, /* 0x7B */
     0x0,  0x0, 0x77,  0x0,  0x0,  0x0, /* 0x7C */
     0x0, 0x41, 0x41, 0x3E,  0x8,  0x0, /* 0x7D */
     0x2,  0x1,  0x2,  0x1,  0x0,  0x0, /* 0x7E */
};

/* Internal function prototypes. */
static int32_t oled_ssd1306_display(GFX *, const uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);
static int32_t oled_ssd1306_power(GFX *, uint8_t);
static int32_t oled_ssd1306_clear_display(GFX *);
static int32_t oled_ssd1306_invert(GFX *, uint8_t);
static int32_t oled_ssd1306_command(SSD1306 *, uint8_t);
/*
 * oled_ssd1306_init
 * This function is responsible for initializing OLED-SSD1306 subsystem.
 */
void oled_ssd1306_init(void)
{
#ifdef OLED_TGT_INIT
    /* Initialize OLED subsystem */
    OLED_TGT_INIT();
#endif
} /* oled_ssd1306_init */

/*
 * oled_ssd1306_register
 * @oled: SSD1306 device data.
 * @return: Success will be returned if OLED was successfully initialized,
 *  OLED_
 * This function will register a OLED-SSD1306 driver.
 */
int32_t oled_ssd1306_register(SSD1306 *oled)
{
    int32_t status = SUCCESS;

    /* Initialize I2C device. */
    i2c_init(&oled->i2c);

    /* Initialize SSD1306. */

    /* Try to turn off the OLED, as this is first command we will poll for
     * the command to complete successfully. */
    POLL_SW_MS((oled_ssd1306_command(oled, SSD1306_DISPLAYOFF) != SUCCESS), OLED_SSD1306_INIT_DELAY, status, SSD1306_INIT_ERROR);

    /* Configure OLED clock. */
    if (status == SUCCESS)
    {
        /* Send clock command. */
        status = oled_ssd1306_command(oled, SSD1306_SETDISPLAYCLOCKDIV);

        if (status == SUCCESS)
        {
            /* Set clock divider to 0x80. */
            status = oled_ssd1306_command(oled, 0x80);
        }
    }

    /* Set multiplexer. */
    if (status == SUCCESS)
    {
        /* Send multiplexer command. */
        status = oled_ssd1306_command(oled, SSD1306_SETMULTIPLEX);

        if (status == SUCCESS)
        {
            /* Set multiplex to the height of OLED. */
            status = oled_ssd1306_command(oled, (uint8_t)(oled->gfx.height - 1));
        }
    }

    /* Set display offset and starting line.. */
    if (status == SUCCESS)
    {
        /* Send display offset command. */
        status = oled_ssd1306_command(oled, SSD1306_SETDISPLAYOFFSET);

        if (status == SUCCESS)
        {
            /* Reset the display offset. */
            status = oled_ssd1306_command(oled, 0x0);
        }
    }

    if (status == SUCCESS)
    {
        /* Reset the start line. */
        status = oled_ssd1306_command(oled, SSD1306_SETSTARTLINE | 0x0);
    }

    /* Configure charge pump. */
    if (status == SUCCESS)
    {
        /* Send the charge pump command. */
        status = oled_ssd1306_command(oled, SSD1306_CHARGEPUMP);

        if (status == SUCCESS)
        {
            /* Set the charge pump according to power source. */
            if (oled->flags & SSD1306_EXTERNAL_VCC)
            {
                status = oled_ssd1306_command(oled, 0x10);
            }
            else
            {
                status = oled_ssd1306_command(oled, 0x14);
            }
        }
    }

    /* Configure memory mode. */
    if (status == SUCCESS)
    {
        /* Send the memory mode command. */
        status = oled_ssd1306_command(oled, SSD1306_MEMORYMODE);

        if (status == SUCCESS)
        {
            /* Set horizontal addressing mode. */
            status = oled_ssd1306_command(oled, 0x0);
        }
    }

    if (status == SUCCESS)
    {
        /* Configure the segment re-map. */
        status = oled_ssd1306_command(oled, SSD1306_SEGREMAP | 0x1);
    }

    if (status == SUCCESS)
    {
        /* Set COM output scan direction. */
        status = oled_ssd1306_command(oled, SSD1306_COMSCANDEC);
    }

    /* Configure COM pins. */
    if (status == SUCCESS)
    {
        /* Send COM pin command. */
        status = oled_ssd1306_command(oled, SSD1306_SETCOMPINS);

        if (status == SUCCESS)
        {
            /* Set COM pin configuration. */
            status = oled_ssd1306_command(oled, 0x12);
        }
    }

    /* Set contrast. */
    if (status == SUCCESS)
    {
        /* Send contrast command. */
        status = oled_ssd1306_command(oled, SSD1306_SETCONTRAST);

        if (status == SUCCESS)
        {
            /* Configure contrast according to voltage source. */
            if (oled->flags & SSD1306_EXTERNAL_VCC)
            {
                status = oled_ssd1306_command(oled, 0x9F);
            }
            else
            {
                status = oled_ssd1306_command(oled, 0xCF);
            }
        }
    }

    /* Configure pre-charge. */
    if (status == SUCCESS)
    {
        /* Send pre-charge command. */
        status = oled_ssd1306_command(oled, SSD1306_SETPRECHARGE);

        if (status == SUCCESS)
        {
            /* Configure pre-charge according to voltage source. */
            if (oled->flags & SSD1306_EXTERNAL_VCC)
            {
                status = oled_ssd1306_command(oled, 0x22);
            }
            else
            {
                status = oled_ssd1306_command(oled, 0xF1);
            }
        }
    }

    /* Configure VCOM de-select level. */
    if (status == SUCCESS)
    {
        /* Send VCOM de-select level command. */
        status = oled_ssd1306_command(oled, SSD1306_SETVCOMDESELECT);

        if (status == SUCCESS)
        {
            /* Adjust de-select level to 0x40. */
            status = oled_ssd1306_command(oled, 0x40);
        }
    }

    if (status == SUCCESS)
    {
        /* Turn on the entire display. */
        status = oled_ssd1306_command(oled, SSD1306_DISPLAYALLON_RESUME);
    }

    if (status == SUCCESS)
    {
        /* Show display normally. */
        status = oled_ssd1306_command(oled, SSD1306_NORMALDISPLAY);
    }

    if (status == SUCCESS)
    {
        /* Deactivate scrolling. */
        status = oled_ssd1306_command(oled, SSD1306_DEACTIVATE_SCROLL);
    }

    if (status == SUCCESS)
    {
        /* Hook up graphics for this OLED. */
        oled->gfx.display = &oled_ssd1306_display;
        oled->gfx.power = &oled_ssd1306_power;
        oled->gfx.clear = &oled_ssd1306_clear_display;
        oled->gfx.invert = &oled_ssd1306_invert;

        /* Set the font information. */
        oled->gfx.font = oled_font_data;
        oled->gfx.font_width = 6;
        oled->gfx.font_height = 8;
        oled->gfx.font_char_start = ' ';
        oled->gfx.font_char_end = '~';


        /* Register this with graphics. */
        graphics_register(&oled->gfx);
    }

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_register */

/*
 * oled_ssd1306_display
 * @gfx: Graphics data.
 * @buffer: Buffer to display.
 * @col: Starting column.
 * @num_col: Number of columns.
 * @row: Starting row.
 * @num_row: Number of rows.
 * @return: Success will be returned if the given data was successfully displayed.
 * This function will display a buffer on OLED.
 */
static int32_t oled_ssd1306_display(GFX *gfx, const uint8_t *buffer, uint32_t col, uint32_t num_col, uint32_t row, uint32_t num_row)
{
    SSD1306 *oled = (SSD1306 *)gfx;
    int32_t i, this_bytes, num_bytes;
    int32_t status;
    uint8_t display_buffer[OLED_I2C_CHUNK_SIZE + 1];
    I2C_MSG msg;

    /* Set column address. */
    status = oled_ssd1306_command(oled, SSD1306_COLUMNADDR);

    if (status == SUCCESS)
    {
        /* Set the column start address. */
        status = oled_ssd1306_command(oled, (uint8_t)col);

        if (status == SUCCESS)
        {
            /* Set the column end address. */
            status = oled_ssd1306_command(oled, (uint8_t)((col + num_col) - 1));
        }
    }

    /* If column address was successfully set. */
    if (status == SUCCESS)
    {
        /* Set the page address. */
        status = oled_ssd1306_command(oled, SSD1306_PAGEADDR);

        if (status == SUCCESS)
        {
            /* Set the page start address. */
            status = oled_ssd1306_command(oled, (uint8_t)(row / 8));
        }

        if (status == SUCCESS)
        {
            /* Set the page end address. */
            status = oled_ssd1306_command(oled, (uint8_t)(((row + num_row) / 8) - 1));
        }
    }

    if (status == SUCCESS)
    {
        /* Initialize command buffer. */
        display_buffer[0] = 0x40;

        /* Initialize I2C message. */
        msg.buffer = display_buffer;
        msg.length = OLED_I2C_CHUNK_SIZE + 1;
        msg.flags = I2C_MSG_WRITE;

        /* Number of bytes to transfer. */
        num_bytes = (int32_t)((num_col * num_row) / 8);

        /* If a buffer was not given. */
        if (!buffer)
        {
            /* Just fill with zeros. */
            memset(&display_buffer[1], 0, OLED_I2C_CHUNK_SIZE);
        }

        /* Send configured bytes of data at a time. */
        for (i = 0; ((status == SUCCESS) && (i < num_bytes)); i += OLED_I2C_CHUNK_SIZE)
        {
            /* If we have less than configured bytes to transfer. */
            if ((num_bytes - i) < OLED_I2C_CHUNK_SIZE)
            {
                /* Transfer the remaining number of bytes. */
                this_bytes = num_bytes - i;
                msg.length = this_bytes + 1;
            }
            else
            {
                /* Transfer all the bytes. */
                this_bytes = OLED_I2C_CHUNK_SIZE;
            }

            /* If a buffer was given. */
            if (buffer)
            {
                /* Copy buffer data. */
                memcpy(&display_buffer[1], &buffer[i], (uint8_t)this_bytes);
            }

            /* Send this chunk on I2C bus. */
            status = i2c_message(&oled->i2c, &msg, 1);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_display */

/*
 * oled_ssd1306_power
 * @gfx: Graphics data.
 * @turn_on: If we are needed to turn on the display.
 * @return: Success will be returned if power state was successfully updated.
 * This function will clear the display.
 */
static int32_t oled_ssd1306_power(GFX *gfx, uint8_t turn_on)
{
    SSD1306 *oled = (SSD1306 *)gfx;
    int32_t status;

    /* Update the display power. */
    status = oled_ssd1306_command(oled, (turn_on == TRUE) ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_power */

/*
 * oled_ssd1306_clear_display
 * @gfx: Graphics data.
 * @return: Success will be returned if display was successfully cleared.
 * This function will clear the display.
 */
static int32_t oled_ssd1306_clear_display(GFX *gfx)
{
    SSD1306 *oled = (SSD1306 *)gfx;
    int32_t status;

    /* Clear the display. */
    status = oled_ssd1306_display(&oled->gfx, NULL, 0, oled->gfx.width, 0, oled->gfx.height);

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_clear_display */

/*
 * oled_ssd1306_invert
 * @gfx: Graphics data.
 * @invert: Flag to specify if we need to invert the display.
 * @return: Success will be returned if display was successfully inverted.
 * This function will invert SSD1306 display.
 */
static int32_t oled_ssd1306_invert(GFX *gfx, uint8_t invert)
{
    SSD1306 *oled = (SSD1306 *)gfx;
    int32_t status;

    /* If we need to invert the display. */
    if (invert)
    {
        /* Invert the display. */
        status = oled_ssd1306_command(oled, SSD1306_INVERTDISPLAY);
    }
    else
    {
        /* Normalize the display. */
        status = oled_ssd1306_command(oled, SSD1306_NORMALDISPLAY);
    }

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_invert */

/*
 * oled_ssd1306_command
 * @oled: SSD1306 device data.
 * @command: Command to be executed.
 * @return: Success will be returned if command was successfully executed.
 * This function will execute a command on SSD1306.
 */
static int32_t oled_ssd1306_command(SSD1306 *oled, uint8_t command)
{
    int32_t status;
    I2C_MSG msg;
    uint8_t command_buffer[2];

    /* Initialize command buffer. */
    command_buffer[0] = 0x0;
    command_buffer[1] = command;

    /* Initialize I2C message. */
    msg.buffer = command_buffer;
    msg.length = 2;
    msg.flags = I2C_MSG_WRITE;

    /* Send a message on I2C bus. */
    status = i2c_message(&oled->i2c, &msg, 1);

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_command */

#endif /* CONFIG_OLED */
