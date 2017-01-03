/*
 * lcd_an.h
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#ifndef _LCD_AN_H_
#define _LCD_AN_H_

#include <os.h>

#ifdef CONFIG_FS
#include <fs.h>
#endif

#ifdef CONFIG_LCD_AN
#ifdef FS_CONSOLE
#include <console.h>
#else
#error "Console driver is required for LCD_AN driver."
#endif

#define LCD_AN_DEBUG

/* Alphanumeric LCD configurations */
#define LCD_AN_BUSY_TIMEOUT     (500)
#define LCD_AN_TAB_SIZE         (3)
#define LCD_AN_8_BIT_DELAY      (10)

/* LCD delay configurations. */
#define LCD_AN_INIT_DELAY       (15)
#define LCD_AN_CLEAR_DELAY      (10)
#define LCD_AN_READ_DELAY       (5)

/* LCD error code definitions. */
#define LCD_AN_TIME_OUT         -1300
#define LCD_AN_COLUMN_FULL      -1301
#define LCD_AN_ROW_FULL         -1302
#define LCD_AN_INTERNAL_ERROR   -1303

/* LCD IOCTL commands. */
#define LCD_AN_CUSTOM_CHAR     (1)

/* Alphanumeric LCD driver data. */
typedef struct _lcd_an
{
    /* Console data for this driver. */
    CONSOLE    console;

    /* Private data. */
    void       *priv_data;

    /* Number of rows and column on the LCD. */
    uint16_t   row;
    uint16_t   column;

    /* Current location of cursor. */
    uint16_t   cur_row;
    uint16_t   cur_column;

} LCD_AN;

/* Alphanumeric LCD ioctl data. */
typedef struct _lcd_an_ioctl_data
{
    /* LCD IOCTL data. */
    uint32_t    index;
    void        *param;
} LCD_AN_IOCTL_DATA;

/* Include LCD target configuration. */
#include <lcd_an_target.h>

/* Function prototypes. */
void lcd_an_init();
void lcd_an_register(LCD_AN *);
int32_t lcd_an_wait_8bit(LCD_AN *);
void lcd_an_send_nibble(LCD_AN *, uint8_t);
int32_t lcd_an_write_register(LCD_AN *, uint8_t, uint8_t);
int32_t lcd_an_read_register(LCD_AN *, uint8_t, uint8_t *);
int32_t lcd_an_is_busy(LCD_AN *);
int32_t lcd_an_create_custom_char(LCD_AN *, uint8_t, uint8_t *);
int32_t lcd_an_write(void *, uint8_t *, int32_t);
int32_t lcd_an_ioctl(void *, uint32_t, void *);

#endif /* CONFIG_LCD_AN */

#endif /* _LCD_AN_H_ */
