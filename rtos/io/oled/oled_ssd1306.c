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

/* Internal function prototypes. */
static int32_t oled_ssd1306_display(GFX *, uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);
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
 * This function will register a OLED-SSD1306 driver.
 */
int32_t oled_ssd1306_register(SSD1306 *oled)
{
    int32_t status;

    /* Initialize I2C device. */
    i2c_init(&oled->i2c);

    /* Initialize SSD1306. */

    /* Turn off display. */
    status = oled_ssd1306_command(oled, SSD1306_DISPLAYOFF);

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
            status = oled_ssd1306_command(oled, 0x00);
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
 * This function will display a buffer on OLED.
 */
static int32_t oled_ssd1306_display(GFX *gfx, uint8_t *buffer, uint32_t col, uint32_t num_col, uint32_t row, uint32_t num_row)
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
 * This function will execute a command on SSD1306.
 */
static int32_t oled_ssd1306_command(SSD1306 *oled, uint8_t command)
{
    int32_t status;
    I2C_MSG msg;
    uint8_t command_buffer[2];

    /* Initialize command buffer. */
    command_buffer[0] = 0x00;
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
