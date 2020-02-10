/*
 * lcd_an.c
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef IO_LCD_AN
#include <lcd_an.h>
#include <lcd_an_target.h>
#include <string.h>

/* Alphanumeric LCD debug file descriptor. */
FD lcd_an_fd = NULL;

/* Internal function prototypes. */
static void lcd_an_send_nibble(LCD_AN *, uint8_t);
static int32_t lcd_an_write_register(LCD_AN *, uint8_t, uint8_t);
static int32_t lcd_an_read_register(LCD_AN *, uint8_t, uint8_t *);
static int32_t lcd_an_create_custom_char(LCD_AN *, uint8_t, uint8_t *);
static int32_t lcd_an_write(void *, const uint8_t *, int32_t);
static int32_t lcd_an_ioctl(void *, uint32_t, void *);

/*
 * lcd_an_init
 * This function is responsible for initializing Alphanumeric LCD subsystem.
 */
void lcd_an_init(void)
{
#ifdef LCD_AN_TGT_INIT
    /* Initialize Alphanumeric LCD subsystem. */
    LCD_AN_TGT_INIT();
#endif

} /* lcd_an_init */

/*
 * lcd_an_register
 * @lcd_an: Alphanumeric LCD data.
 * This function will register a Alphanumeric LCD driver.
 */
void lcd_an_register(LCD_AN *lcd_an)
{
    int32_t status = SUCCESS;
    char fs_name[64] = "\\console\\";

    /* Initialize Alphanumeric LCD. */
    lcd_an->clr_rs(lcd_an);
    lcd_an->set_rw(lcd_an);
    lcd_an->clr_en(lcd_an);

#if (LCD_AN_INIT_DELAY > 0)
    /* Need to wait at least 15ms on power up. */
    sleep_fms(LCD_AN_INIT_DELAY);
#endif

    /* Initialize Alphanumeric LCD in 4-bit mode ignore status from initial commands. */
    lcd_an_write_register(lcd_an, LCD_IGNORE_WAIT, 0x33);
    lcd_an_write_register(lcd_an, LCD_IGNORE_WAIT, 0x32);
    status = lcd_an_write_register(lcd_an, 0, 0x28);

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0x8);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0x1);

#if (LCD_AN_CLEAR_DELAY > 0)
        /* Wait for sometime before writing any more data. */
        sleep_fms(LCD_AN_CLEAR_DELAY);
#endif
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0x6);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0xC);
    }

    if (status == SUCCESS)
    {
        /* Reset the cursor location. */
        lcd_an->cur_column = lcd_an->cur_row = 0;

        /* Register this Alphanumeric LCD driver with console. */
        lcd_an->console.fs.write = &lcd_an_write;
        lcd_an->console.fs.ioctl = &lcd_an_ioctl;
        console_register(&lcd_an->console);

        /* There is always some space available for data to be sent. */
        lcd_an->console.fs.flags |= FS_SPACE_AVAILABLE;

        /* If this is a debug device. */
        if (lcd_an->flags & LCD_FLAG_DEBUG)
        {
            /* Open this as debug descriptor. */
            strncat(fs_name, lcd_an->console.fs.name, (64 - sizeof("\\console\\")));
            lcd_an_fd = fs_open(fs_name, 0);
        }
    }
} /* lcd_an_register */

/*
 * lcd_an_reset
 * @lcd_an: Alphanumeric LCD data.
 * This function will reset the Alphanumeric LCD driver.
 */
int32_t lcd_an_reset(LCD_AN *lcd_an)
{
    int32_t status = SUCCESS;

    /* Initialize Alphanumeric LCD. */
    lcd_an->clr_rs(lcd_an);
    lcd_an->set_rw(lcd_an);
    lcd_an->clr_en(lcd_an);

#if (LCD_AN_INIT_DELAY > 0)
    /* Need to wait at least 15ms on power up. */
    sleep_fms(LCD_AN_INIT_DELAY);
#endif

    /* Initialize Alphanumeric LCD in 4-bit mode ignore status from initial commands. */
    lcd_an_write_register(lcd_an, LCD_IGNORE_WAIT, 0x33);
    lcd_an_write_register(lcd_an, LCD_IGNORE_WAIT, 0x32);
    status = lcd_an_write_register(lcd_an, 0, 0x28);

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0x8);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0x1);

#if (LCD_AN_CLEAR_DELAY > 0)
        /* Wait for sometime before writing any more data. */
        sleep_fms(LCD_AN_CLEAR_DELAY);
#endif
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0x6);
    }

    if (status == SUCCESS)
    {
        status = lcd_an_write_register(lcd_an, 0, 0xC);
    }

    if (status == SUCCESS)
    {
        /* Reset the cursor location. */
        lcd_an->cur_column = lcd_an->cur_row = 0;

        /* There is always some space available for data to be sent. */
        lcd_an->console.fs.flags |= FS_SPACE_AVAILABLE;
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_reset */

/*
 * lcd_an_send_nibble
 * @lcd_an: Alphanumeric LCD driver on which a nibble is needed to be sent.
 * @cmd: Nibble needed to be sent.
 * This function sends a nibble to the Alphanumeric LCD.
 */
static void lcd_an_send_nibble(LCD_AN *lcd_an, uint8_t nibble)
{
    /* Put nibble value. */
    lcd_an->put_data(lcd_an, nibble);

    /* Latch the data on the register. */
    lcd_an->set_en(lcd_an);

    /* Clear enable. */
    lcd_an->clr_en(lcd_an);

} /* lcd_an_send_nibble */

/*
 * lcd_an_write_register
 * @lcd_an: Alphanumeric LCD driver on which a byte is needed to be written.
 * @rs: Flag to specify if we are writing a data register or command register.
 *  TRUE: If we need to write a data register.
 *  FALSE: If we need to write a command register.
 * @byte: Byte needed to be written.
 * @return: Success will be returned if bytes was successfully written
 *  LCD_AN_TIME_OUT will be returned if timed out waiting for Alphanumeric LCD.
 * This function write a register to the Alphanumeric LCD.
 */
static int32_t lcd_an_write_register(LCD_AN *lcd_an, uint8_t rs, uint8_t byte)
{
#ifndef LCD_AN_NO_BUSY_WAIT
    uint8_t cmd_byte;
    uint32_t sys_time = current_system_tick();
#endif /* LCD_AN_NO_BUSY_WAIT */
    int32_t status = SUCCESS;

#ifndef LCD_AN_NO_BUSY_WAIT
    /* Wait for Alphanumeric LCD. */
    do
    {
        /* Read command register. */
        lcd_an_read_register(lcd_an, FALSE, &cmd_byte);

        /* If we are still busy. */
        if (cmd_byte & (1 << 7))
        {
            /* Yield the task. */
            task_yield();
        }

    } while ((current_system_tick() - sys_time) < (MS_TO_TICK(LCD_AN_BUSY_TIMEOUT)) && (cmd_byte & (1 << 7)));

    /* If we did not timeout waiting for the Alphanumeric LCD. */
    if (((cmd_byte & (1 << 7)) == 0) || (rs & LCD_IGNORE_WAIT))
#endif /* LCD_AN_NO_BUSY_WAIT */
    {
        /* Select required register. */
        if (rs & LCD_DATA_REG)
        {
            /* Select data register. */
            lcd_an->set_rs(lcd_an);
        }
        else
        {
            /* Select command register. */
            lcd_an->clr_rs(lcd_an);
        }

        /* We are writing data. */
        lcd_an->clr_rw(lcd_an);

        /* Disable the Alphanumeric LCD data line. */
        lcd_an->clr_en(lcd_an);

        /* Put byte on the Alphanumeric LCD. */
        lcd_an_send_nibble(lcd_an, ((byte >> 4) & 0xF));
        lcd_an_send_nibble(lcd_an, (byte & 0xF));

#ifdef LCD_AN_NO_BUSY_WAIT
        /* Yield the task to put delay in transaction.. */
        task_yield();
#else
    }
    else
    {
        /* Return error to the caller. */
        status = LCD_AN_TIME_OUT;
#endif /* LCD_AN_NO_BUSY_WAIT */
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_write_register */

/*
 * lcd_an_read_register
 * @lcd_an: Alphanumeric LCD driver from which a byte is needed to be read.
 * @rs: Flag to specify if we are writing a data register or command register.
 *  TRUE: If we need to write a data register.
 *  FALSE: If we need to write a command register.
 * @byte: Read data will be returned here.
 * @return: Always return success.
 * This function send a command to the Alphanumeric LCD.
 */
static int32_t lcd_an_read_register(LCD_AN *lcd_an, uint8_t rs, uint8_t *byte)
{
    uint8_t ret_byte;

    /* Select required register. */
    if (rs == TRUE)
    {
        /* Select data register. */
        lcd_an->set_rs(lcd_an);
    }
    else
    {
        /* Select command register. */
        lcd_an->clr_rs(lcd_an);
    }

    /* We are reading data. */
    lcd_an->set_rw(lcd_an);

    /* Enable the Alphanumeric LCD data line. */
    lcd_an->set_en(lcd_an);

    /* Wait before reading back from the Alphanumeric LCD. */
    sleep_us(LCD_AN_READ_DELAY);

    /* Read first 4 bits. */
    ret_byte = (uint8_t)(lcd_an->read_data(lcd_an) << 4);

    /* Clear the Alphanumeric LCD data line. */
    lcd_an->clr_en(lcd_an);

    /* Enable the Alphanumeric LCD data line. */
    lcd_an->set_en(lcd_an);

    /* Wait before reading back from the Alphanumeric LCD. */
    sleep_us(LCD_AN_READ_DELAY);

    /* Read last 4 bits. */
    ret_byte |= lcd_an->read_data(lcd_an);

    /* Clear the Alphanumeric LCD data line. */
    lcd_an->clr_en(lcd_an);

    /* Return the read byte. */
    *byte = ret_byte;

    /* Always return success. */
    return (0);

} /* lcd_an_read_register */

/*
 * lcd_an_create_custom_char
 * @lcd_an: Alphanumeric LCD driver for which a custom character is needed to be written.
 * @index: Custom character index.
 * @bitmap: Character bitmap array.
 * This function creates/overwrites an custom character on the given Alphanumeric LCD.
 */
static int32_t lcd_an_create_custom_char(LCD_AN *lcd_an, uint8_t index, uint8_t *bitmap)
{
    int32_t status;
    uint8_t i;
    uint8_t ddram_addr;

    /* Get the DDRAM address. */
    status = lcd_an_read_register(lcd_an, 0, &ddram_addr);

    if (status == SUCCESS)
    {
        /* Move to required index in the CGRAM. */
        status = lcd_an_write_register(lcd_an, 0, (uint8_t)(0x40 + (index << 3)));

        /* Write the bitmap of the character. */
        for (i = 0; ((status == SUCCESS) && (i < 8)); i++)
        {
            /* Write the bitmap in the CGRAM. */
            status = lcd_an_write_register(lcd_an, LCD_DATA_REG, bitmap[i]);
        }

        /* Revert to old DDRAM address. */
        (void)lcd_an_write_register(lcd_an, 0, (ddram_addr | (1 << 7)));
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_create_custom_char */

/*
 * lcd_an_write
 * @priv_data: Alphanumeric LCD data for which this was called.
 * @buf: String needed to be printed.
 * @nbytes: Number of bytes to be printed from the string.
 * @return: Number of bytes will be returned if write was successful,
 *  LCD_AN_TIME_OUT will be returned if timed out waiting for
 *      Alphanumeric LCD,
 *  LCD_AN_ROW_FULL will be returned if there are no more rows left on the
 *      Alphanumeric LCD,
 *  LCD_AN_COLUMN_FULL will be returned if there is more column left on the
 *      Alphanumeric LCD.
 * This function prints a string on the Alphanumeric LCD.
 */
static int32_t lcd_an_write(void *priv_data, const uint8_t *buf, int32_t nbytes)
{
    LCD_AN *lcd_an = (LCD_AN *)priv_data;
    int32_t to_print = nbytes;
    int32_t status = SUCCESS;
    uint8_t address = 0, indent_size;

    /* While we have some data to be printed. */
    while ((status == SUCCESS) && (nbytes > 0))
    {
        /* Put this character on the Alphanumeric LCD. */
        switch (*buf)
        {

        /* Handle clear screen. */
        case '\f':

            /* Clear display. */
            status = lcd_an_write_register(lcd_an, 0, 0x1);

            /* If display was successfully cleared. */
            if (status == SUCCESS)
            {
                /* Reset the cursor location. */
                lcd_an->cur_column = lcd_an->cur_row = 0;
            }

#if (LCD_AN_CLEAR_DELAY > 0)
            /* Wait for sometime before writing any more data. */
            sleep_fms(LCD_AN_CLEAR_DELAY);
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
                if (lcd_an->cur_row < lcd_an->row)
                {
                    /* Move cursor to next row. */
                    lcd_an->cur_row ++;
                }
                else
                {
                    /* No more rows on the Alphanumeric LCD. */
                    status = LCD_AN_ROW_FULL;
                }

                break;

            /* If this was a return carriage. */
            case '\r':

                /* Reset the cursor column. */
                lcd_an->cur_column = 0;

                break;

            /* If this was a tab. */
            case '\t':

                /* Calculate the indent size. */
                indent_size = (uint8_t)(LCD_AN_TAB_SIZE - ((lcd_an->cur_column) % LCD_AN_TAB_SIZE));

                /* Check if we can add required indentation. */
                if ((lcd_an->cur_column + indent_size) < lcd_an->column)
                {
                    /* Move the cursor to required column. */
                    lcd_an->cur_column = (uint16_t)(lcd_an->cur_column + indent_size);
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
                switch (lcd_an->cur_row)
                {
                case 0:
                    address = (uint8_t)(0x80 + lcd_an->cur_column);
                    break;
                case 1:
                    address = (uint8_t)(0xc0 + lcd_an->cur_column);
                    break;
                case 2:
                    address = (uint8_t)(0x94 + lcd_an->cur_column);
                    break;
                case 3:
                    address = (uint8_t)(0xd4 + lcd_an->cur_column);
                    break;
                default:
                    /* Unsupported row. */
                    status = LCD_AN_INTERNAL_ERROR;
                    break;
                }
            }

            if (status == SUCCESS)
            {
                /* Update cursor location on the Alphanumeric LCD. */
                status = lcd_an_write_register(lcd_an, 0, address);
            }

            break;

        /* Normal ASCII character. */
        default:

            /* If we are not at the last column. */
            if (lcd_an->cur_column < lcd_an->column)
            {
                /* Write a data register. */
                status = lcd_an_write_register(lcd_an, LCD_DATA_REG, *buf);

                /* Move Alphanumeric LCD cursor to next column. */
                lcd_an->cur_column ++;
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
 * @priv_data: Alphanumeric LCD data for which this was called.
 * @cmd: IOCTL command needed to be processed.
 * @param: IOCTL data.
 * @return: Returns success if the command was successful,
 *  FS_INVALID_COMMAND will be returned if an unknown command was requested.
 * This function executes a special command on the Alphanumeric LCD.
 */
static int32_t lcd_an_ioctl(void *priv_data, uint32_t cmd, void *param)
{
    LCD_AN *lcd_an = (LCD_AN *)priv_data;
    LCD_AN_IOCTL_DATA *data;
    int32_t status;

    /* Process the requested command. */
    switch (cmd)
    {

    /* Need to create a custom character. */
    case LCD_AN_CUSTOM_CHAR:

        /* Pick the IOCTL data. */
        data = (LCD_AN_IOCTL_DATA *)param;

        /* Create a custom Alphanumeric LCD character. */
        status = lcd_an_create_custom_char(lcd_an, (uint8_t)data->index,
                                           (uint8_t *)data->param);

        break;

    /* Need to reset the Alphanumeric LCD interface. */
    case LCD_AN_RESET:

#ifdef LCD_AN_TGT_RESET
        /* Reset Alphanumeric LCD subsystem. */
        status = LCD_AN_TGT_RESET();
#else
        status = SUCCESS;
#endif

        break;

    default:

        /* Unknown command was requested. */
        status = FS_INVALID_COMMAND;

        break;
    }

    /* Return status to the caller. */
    return (status);

} /* lcd_an_ioctl */

#endif /* IO_LCD_AN */
