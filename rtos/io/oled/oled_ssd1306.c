/*
 * oled_ssd1306.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
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
void oled_ssd1306_register(SSD1306 *oled)
{
    /* Initialize I2C device. */
    i2c_init(&oled->i2c);

    /* Initialize SSD1306. */

    /* Turn off display. */
    oled_ssd1306_command(oled, SSD1306_DISPLAYOFF);

    /* Configure OLED clock. */
    oled_ssd1306_command(oled, SSD1306_SETDISPLAYCLOCKDIV);
    oled_ssd1306_command(oled, 0x80);

    /* Set multiplexer. */
    oled_ssd1306_command(oled, SSD1306_SETMULTIPLEX);
    oled_ssd1306_command(oled, oled->height - 1);

    /* Set display offset and starting line.. */
    oled_ssd1306_command(oled, SSD1306_SETDISPLAYOFFSET);
    oled_ssd1306_command(oled, 0x0);
    oled_ssd1306_command(oled, SSD1306_SETSTARTLINE | 0x0);

    /* Conifgure charge pump. */
    oled_ssd1306_command(oled, SSD1306_CHARGEPUMP);
    if (oled->flags & SSD1306_EXTERNAL_VCC)
    {
        oled_ssd1306_command(oled, 0x10);
    }
    else
    {
        oled_ssd1306_command(oled, 0x14);
    }

    /* Configure memory mode. */
    oled_ssd1306_command(oled, SSD1306_MEMORYMODE);
    oled_ssd1306_command(oled, 0x00);
    oled_ssd1306_command(oled, SSD1306_SEGREMAP | 0x1);
    oled_ssd1306_command(oled, SSD1306_COMSCANDEC);
    oled_ssd1306_command(oled, SSD1306_SETCOMPINS);
    oled_ssd1306_command(oled, 0x12);

    /* Set contrast. */
    oled_ssd1306_command(oled, SSD1306_SETCONTRAST);
    if (oled->flags & SSD1306_EXTERNAL_VCC)
    {
        oled_ssd1306_command(oled, 0x9F);
    }
    else
    {
        oled_ssd1306_command(oled, 0xCF);
    }

    /* Configure pre-charge. */
    oled_ssd1306_command(oled, SSD1306_SETPRECHARGE);
    if (oled->flags & SSD1306_EXTERNAL_VCC)
    {
        oled_ssd1306_command(oled, 0x22);
    }
    else
    {
        oled_ssd1306_command(oled, 0xF1);
    }

    oled_ssd1306_command(oled, SSD1306_SETVCOMDETECT);
    oled_ssd1306_command(oled, 0x40);

    /* Turn on the display in normal mode. */
    oled_ssd1306_command(oled, SSD1306_DISPLAYALLON_RESUME);
    oled_ssd1306_command(oled, SSD1306_NORMALDISPLAY);
    oled_ssd1306_command(oled, SSD1306_DEACTIVATE_SCROLL);
    oled_ssd1306_command(oled, SSD1306_DISPLAYON);

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

    /* Set column start and end address. */
    status = oled_ssd1306_command(oled, SSD1306_COLUMNADDR);

    if (status == SUCCESS)
    {
        status = oled_ssd1306_command(oled, col);

        if (status == SUCCESS)
        {
            status = oled_ssd1306_command(oled, (col + num_col) - 1);
        }
    }

    /* Set the page address. */
    if (status == SUCCESS)
    {
        status = oled_ssd1306_command(oled, SSD1306_PAGEADDR);

        if (status == SUCCESS)
        {
            status = oled_ssd1306_command(oled, (row / 8));
        }

        if (status == SUCCESS)
        {
            status = oled_ssd1306_command(oled, ((row + num_row) / 8) - 1);
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
            memcpy(&display_buffer[1], &buffer[i], this_bytes);

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
