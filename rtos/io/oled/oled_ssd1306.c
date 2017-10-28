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
            status = oled_ssd1306_command(oled, (uint8_t)(oled->height - 1));
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
        /* Turn-on the display. */
        status = oled_ssd1306_command(oled, SSD1306_DISPLAYON);
    }

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_register */

/*
 * oled_ssd1306_display
 * @oled: SSD1306 device data.
 * @buffer: Buffer to display.
 * @col: Starting column.
 * @num_col: Number of columns.
 * @row: Starting row.
 * @num_row: Number of rows.
 * This function will display a buffer on OLED.
 */
int32_t oled_ssd1306_display(SSD1306 *oled, uint8_t *buffer, uint8_t col, uint8_t num_col, uint8_t row, uint8_t num_row)
{
    int32_t status, i, num_bytes, this_bytes;
    uint8_t display_buffer[17];
    I2C_MSG msg;

    /* Set column address. */
    status = oled_ssd1306_command(oled, SSD1306_COLUMNADDR);

    if (status == SUCCESS)
    {
        /* Set the column start address. */
        status = oled_ssd1306_command(oled, col);

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
            status = oled_ssd1306_command(oled, (row / 8));
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
        msg.length = 17;
        msg.flags = I2C_MSG_WRITE;

        /* Number of bytes to transfer. */
        num_bytes = ((num_col * num_row) / 8);

        /* Send 16 bytes of data at a time. */
        for (i = 0; ((status == SUCCESS) && (i < num_bytes)); i += 16)
        {
            /* If we have less than 16 bytes to transfer. */
            if ((num_bytes - i) < 16)
            {
                /* Transfer the remaining number of bytes. */
                this_bytes = num_bytes - i;
                msg.length = this_bytes + 1;
            }
            else
            {
                /* Transfer all the 16 bytes. */
                this_bytes = 16;
            }

            /* Copy buffer data. */
            memcpy(&display_buffer[1], &buffer[i], (uint8_t)this_bytes);

            /* Send this chunk on I2C bus. */
            status = i2c_message(&oled->i2c, &msg, 1);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* oled_ssd1306_display */

/*
 * oled_ssd1306_invert
 * @oled: SSD1306 device data.
 * @invert: Flag to specify if we need to invert the display.
 * This function will invert SSD1306 display.
 */
int32_t oled_ssd1306_invert(SSD1306 *oled, uint8_t invert)
{
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
