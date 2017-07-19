/*
 * lcd_an.c
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
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

#ifdef CONFIG_LCD_AN
#include <lcd_an.h>
#include <lcd_an_target.h>

#ifdef LCD_AN_DEBUG
/* Alphanumeric LCD debug file descriptor. */
FD lcd_an_fd;
#endif

/* Internal function prototypes. */
static int32_t lcd_an_wait_8bit(LCD_AN *);
static void lcd_an_send_nibble(LCD_AN *, uint8_t);
static int32_t lcd_an_write_register(LCD_AN *, uint8_t, uint8_t);
static int32_t lcd_an_read_register(LCD_AN *, uint8_t, uint8_t *);
static int32_t lcd_an_create_custom_char(LCD_AN *, uint8_t, uint8_t *);
static int32_t lcd_an_write(void *, const uint8_t *, int32_t);
static int32_t lcd_an_ioctl(void *, uint32_t, void *);

/*
 * lcd_an_init
 * This function is responsible for initializing LCD subsystem.
 */
void lcd_an_init()
{
#ifdef LCD_AN_TGT_INIT
    /* Initialize LCD subsystem. */
    LCD_AN_TGT_INIT();
#endif

} /* lcd_an_init */

/*
 * lcd_an_register
 * @lcd: LCD data.
 * This function will register a LCD driver.
 */
void lcd_an_register(LCD_AN *lcd)
{
    int32_t status = SUCCESS;

    /* Initialize LCD. */
    lcd->clr_rs(lcd);
    lcd->set_rw(lcd);
    lcd->clr_en(lcd);

#if (LCD_AN_INIT_DELAY > 0)
    /* Need to wait at least 15ms on power up. */
    sleep_ms(LCD_AN_INIT_DELAY);
#endif

    /* Send first 0x3. */
    lcd_an_send_nibble(lcd, 0x3);

    /* Controller still think that we are using 8bit mode so we can still read
     * the status bit. */
    status = lcd_an_wait_8bit(lcd);

    if (status == SUCCESS)
    {
        /* Send second 0x3. */
        lcd->clr_rw(lcd);
        lcd_an_send_nibble(lcd, 0x3);

        /* Wait for LCD to process the command in 8 bit mode. */
        status = lcd_an_wait_8bit(lcd);
    }

    if (status == SUCCESS)
    {
        /* Send third 0x3. */
        lcd->clr_rw(lcd);
        lcd_an_send_nibble(lcd, 0x3);

        /* Wait for LCD to process the command in 8 bit mode. */
        status = lcd_an_wait_8bit(lcd);
    }

    if (status == SUCCESS)
    {
        /* Switch to 4-bit mode. */
        lcd->clr_rw(lcd);
        lcd_an_send_nibble(lcd, 0x2);

        /* Wait for LCD to process the command in 8 bit mode. */
        status = lcd_an_wait_8bit(lcd);
    }

    /* LCD configuration. */
    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd, FALSE, 0x28);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd, FALSE, 0x28);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd, FALSE, 0x08);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd, FALSE, 0x01);

#if (LCD_AN_CLEAR_DELAY > 0)
        /* Wait for sometime before writing any more data. */
        sleep_ms(LCD_AN_CLEAR_DELAY);
#endif
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd, FALSE, 0x06);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd, FALSE, 0x0C);
    }

    if (status == SUCCESS)
    {
        /* Reset the cursor location. */
        lcd->cur_column = lcd->cur_row = 0;

        /* Register this LCD driver with console. */
        lcd->console.fs.write = &lcd_an_write;
        lcd->console.fs.ioctl = &lcd_an_ioctl;
        console_register(&lcd->console);

        /* There is always some space available for data to be sent. */
        lcd->console.fs.flags |= FS_SPACE_AVAILABLE;

#ifdef LCD_AN_DEBUG
        lcd_an_fd = fs_open("\\console\\lcd1", 0);
#endif
    }

} /* lcd_an_register */

/*
 * lcd_an_wait_8bit
 * @lcd: LCD driver on which a command is needed to be sent.
 * @return: Success will be returned if LCD came out of busy successfully
 *  LCD_TIME_OUT will be returned if we timed out waiting for LCD.
 * This function sends a nibble to the LCD.
 */
static int32_t lcd_an_wait_8bit(LCD_AN *lcd)
{
#if (LCD_AN_8_BIT_DELAY == 0)
    uint32_t sys_time;
#endif
    int32_t status = SUCCESS;

#if (LCD_AN_8_BIT_DELAY > 0)
    UNUSED_PARAM(lcd);

    /* Rather waiting on status bit just busy wait here. */
    sleep_ms(LCD_AN_8_BIT_DELAY);
#else

    /* Read the command register. */
    lcd->set_rw(lcd);
    lcd->set_en(lcd);

    /* Save current system time. */
    sys_time = current_system_tick();

    /* Read the first 4 bit and wait for the busy bit. */
    while ((INT32CMP(current_system_tick(), sys_time) < (MS_TO_TICK(LCD_AN_BUSY_TIMEOUT))) &&
           (lcd->read_data(lcd) & (1 << 3)))
    {
        lcd->clr_en(lcd);
        task_yield();
        lcd->set_en(lcd);
    }

    /* If we timed out waiting for the LCD. */
    if (lcd->read_data(lcd) & (1 << 3))
    {
        /* Return error to the caller. */
        status = LCD_AN_TIME_OUT;
    }

    /* Clear the enable pin. */
    lcd->clr_en(lcd);
#endif

    /* Return status to the caller. */
    return (status);

} /* lcd_an_wait_8bit */

/*
 * lcd_an_send_nibble
 * @lcd: LCD driver on which a nibble is needed to be sent.
 * @cmd: Nibble needed to be sent.
 * This function sends a nibble to the LCD.
 */
static void lcd_an_send_nibble(LCD_AN *lcd, uint8_t nibble)
{
    /* Put nibble value. */
    lcd->put_data(lcd, nibble);

    /* Latch the data on the register. */
    lcd->set_en(lcd);

    /* Clear enable. */
    lcd->clr_en(lcd);

} /* lcd_an_send_nibble */

/*
 * lcd_an_write_register
 * @lcd: LCD driver on which a byte is needed to be written.
 * @rs: Flag to specify if we are writing a data register or command register.
 *  TRUE: If we need to write a data register.
 *  FALSE: If we need to write a command register.
 * @byte: Byte needed to be written.
 * @return: Success will be returned if bytes was successfully written
 *  LCD_AN_TIME_OUT will be returned if timed out waiting for LCD.
 * This function write a register to the LCD.
 */
static int32_t lcd_an_write_register(LCD_AN *lcd, uint8_t rs, uint8_t byte)
{
    uint8_t cmd_byte;
    uint32_t sys_time = current_system_tick();
    int32_t status = SUCCESS;

    /* Wait for LCD. */
    do
    {
        /* Read command register. */
        lcd_an_read_register(lcd, FALSE, &cmd_byte);

        /* If we are still busy. */
        if (cmd_byte & (1 << 7))
        {
            /* Yield the task. */
            task_yield();
        }

    } while ((current_system_tick() - sys_time) < (MS_TO_TICK(LCD_AN_BUSY_TIMEOUT)) && (cmd_byte & (1 << 7)));

    /* If we did not timeout waiting for the LCD. */
    if ((cmd_byte & (1 << 7)) == 0)
    {
        /* Select required register. */
        if (rs == TRUE)
        {
            /* Select data register. */
            lcd->set_rs(lcd);
        }
        else
        {
            /* Select command register. */
            lcd->clr_rs(lcd);
        }

        /* We are writing data. */
        lcd->clr_rw(lcd);

        /* Disable the LCD data line. */
        lcd->clr_en(lcd);

        /* Put byte on the LCD. */
        lcd_an_send_nibble(lcd, ((byte >> 4) & 0x0F));
        lcd_an_send_nibble(lcd, (byte & 0x0F));

    }
    else
    {
        /* Return error to the caller. */
        status = LCD_AN_TIME_OUT;
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_write_register */

/*
 * lcd_an_read_register
 * @lcd: LCD driver from which a byte is needed to be read.
 * @rs: Flag to specify if we are writing a data register or command register.
 *  TRUE: If we need to write a data register.
 *  FALSE: If we need to write a command register.
 * @byte: Read data will be returned here.
 * @return: Always return success.
 * This function send a command to the LCD.
 */
static int32_t lcd_an_read_register(LCD_AN *lcd, uint8_t rs, uint8_t *byte)
{
    uint8_t ret_byte;

    /* Select required register. */
    if (rs == TRUE)
    {
        /* Select data register. */
        lcd->set_rs(lcd);
    }
    else
    {
        /* Select command register. */
        lcd->clr_rs(lcd);
    }

    /* We are reading data. */
    lcd->set_rw(lcd);

    /* Enable the LCD data line. */
    lcd->set_en(lcd);

    /* Wait before reading back from the LCD. */
    sleep_us(LCD_AN_READ_DELAY);

    /* Read first 4 bits. */
    ret_byte = lcd->read_data(lcd) << 4;

    /* Clear the LCD data line. */
    lcd->clr_en(lcd);

    /* Enable the LCD data line. */
    lcd->set_en(lcd);

    /* Wait before reading back from the LCD. */
    sleep_us(LCD_AN_READ_DELAY);

    /* Read last 4 bits. */
    ret_byte |= lcd->read_data(lcd);

    /* Clear the LCD data line. */
    lcd->clr_en(lcd);

    /* Return the read byte. */
    *byte = ret_byte;

    /* Always return success. */
    return (0);

} /* lcd_an_read_register */

/*
 * lcd_an_create_custom_char
 * @lcd: LCD driver for which a custom character is needed to be written.
 * @index: Custom character index.
 * @bitmap: Character bitmap array.
 * This function creates/overwrites an custom character on the given LCD.
 */
static int32_t lcd_an_create_custom_char(LCD_AN *lcd, uint8_t index, uint8_t *bitmap)
{
    int32_t status;
    uint8_t i;
    uint8_t ddram_addr;

    /* Get the DDRAM address. */
    status = lcd_an_read_register(lcd, FALSE, &ddram_addr);

    if (status == SUCCESS)
    {
        /* Move to required index in the CGRAM. */
        status = lcd_an_write_register(lcd, FALSE, 0x40 + (index << 3));

        /* Write the bitmap of the character. */
        for (i = 0; ((status == SUCCESS) && (i < 8)); i++)
        {
            /* Write the bitmap in the CGRAM. */
            status = lcd_an_write_register(lcd, TRUE, bitmap[i]);
        }

        /* Revert to old DDRAM address. */
        (void)lcd_an_write_register(lcd, FALSE, (ddram_addr | (1 << 7)));
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_create_custom_char */

/*
 * lcd_an_write
 * @priv_data: LCD data for which this was called.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * @return: Number of bytes will be returned if write was successful,
 *  LCD_AN_TIME_OUT will be returned if timed out waiting for LCD,
 *  LCD_AN_ROW_FULL will be returned if there are no more rows left on the LCD,
 *  LCD_AN_COLUMN_FULL will be returned if there is more column left on the LCD.
 * This function prints a string on the LCD.
 */
static int32_t lcd_an_write(void *priv_data, const uint8_t *buf, int32_t nbytes)
{
    LCD_AN *lcd = (LCD_AN *)priv_data;
    int32_t to_print = nbytes;
    int32_t status = SUCCESS;
    uint8_t address = 0, indent_size;

    /* While we have some data to be printed. */
    while ((status == SUCCESS) && (nbytes > 0))
    {
        /* Put this character on the LCD. */
        switch (*buf)
        {

        /* Handle clear screen. */
        case '\f':

            /* Clear display. */
            status = lcd_an_write_register(lcd, FALSE, 0x01);

            /* If display was successfully cleared. */
            if (status == SUCCESS)
            {
                /* Reset the cursor location. */
                lcd->cur_column = lcd->cur_row = 0;
            }

#if (LCD_AN_CLEAR_DELAY > 0)
            /* Wait for sometime before writing any more data. */
            sleep_ms(LCD_AN_CLEAR_DELAY);
#endif

            break;

        /* Handle new line, return carriage and tab. */
        case '\n':
        case '\r':
        case '\t':
            switch (*buf)
            {
            /* If this was new line. */
            case '\n':

                /* If we are not at the last row. */
                if (lcd->cur_row < lcd->row)
                {
                    /* Move cursor to next row. */
                    lcd->cur_row ++;
                }
                else
                {
                    /* No more rows on the LCD. */
                    status = LCD_AN_ROW_FULL;
                }

                break;

            /* If this was a return carriage. */
            case '\r':

                /* Reset the cursor column. */
                lcd->cur_column = 0;

                break;

            /* If this was a tab. */
            case '\t':

                /* Calculate the indent size. */
                indent_size = LCD_AN_TAB_SIZE - ((lcd->cur_column) % LCD_AN_TAB_SIZE);

                /* Check if we can add required indentation. */
                if ((lcd->cur_column + indent_size) < lcd->column)
                {
                    /* Move the cursor to required column. */
                    lcd->cur_column += indent_size;
                }
                else
                {
                    /* No more space in the column for indentation. */
                    status = LCD_AN_COLUMN_FULL;
                }

                break;
            }

            if (status == SUCCESS)
            {
                /* Update the cursor position. */
                switch (lcd->cur_row)
                {
                case 0:
                    address = 0x80 + lcd->cur_column;
                    break;
                case 1:
                    address = 0xc0 + lcd->cur_column;
                    break;
                case 2:
                    address = 0x94 + lcd->cur_column;
                    break;
                case 3:
                    address = 0xd4 + lcd->cur_column;
                    break;
                default:
                    /* Unsupported row. */
                    status = LCD_AN_INTERNAL_ERROR;
                    break;
                }
            }

            if (status == SUCCESS)
            {
                /* Update cursor location on the LCD. */
                status = lcd_an_write_register(lcd, FALSE, address);
            }

            break;

        /* Normal ASCII character. */
        default:

            /* If we are not at the last column. */
            if (lcd->cur_column < lcd->column)
            {
                /* Write a data register. */
                status = lcd_an_write_register(lcd, TRUE, *buf);

                /* Move LCD cursor to next column. */
                lcd->cur_column ++;
            }
            else
            {
                /* No more space in the column for new characters. */
                status = LCD_AN_COLUMN_FULL;
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

} /* lcd_an_write */

/*
 * lcd_an_ioctl
 * @priv_data: LCD data for which this was called.
 * @cmd: IOCTL command needed to be processed.
 * @param: IOCTL data.
 * @return: Returns success if the command was successful,
 *  FS_INVALID_COMMAND will be returned if an unknown command was requested.
 * This function executes a special command on the LCD.
 */
static int32_t lcd_an_ioctl(void *priv_data, uint32_t cmd, void *param)
{
    LCD_AN *lcd = (LCD_AN *)priv_data;
    LCD_AN_IOCTL_DATA *data;
    int32_t status;

    /* Process the requested command. */
    switch (cmd)
    {

    /* Need to create a custom character. */
    case LCD_AN_CUSTOM_CHAR:

        /* Pick the IOCTL data. */
        data = (LCD_AN_IOCTL_DATA *)param;

        /* Create a custom LCD character. */
        status = lcd_an_create_custom_char(lcd, data->index,
                                           (uint8_t *)data->param);

        break;

    default:

        /* Unknown command was requested. */
        status = FS_INVALID_COMMAND;

        break;
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_ioctl */

#endif /* CONFIG_LCD_AN */
