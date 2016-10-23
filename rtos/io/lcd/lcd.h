/*
 * lcd.h
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

#ifndef _LCD_H_
#define _LCD_H_

#include <os.h>

#ifdef CONFIG_FS
#include <fs.h>
#endif

#ifdef CONFIG_LCD
#ifdef FS_CONSOLE
#include <console.h>
#else
#error "Console driver is required for LCD driver."
#endif

#define LCD_DEBUG

/* LCD configurations */
#define LCD_BUSY_TIMEOUT    (500)
#define LCD_TAB_SIZE        (3)

/* LCD delay configurations. */
#define LCD_INIT_DELAY      (35)
#define LCD_CLEAR_DELAY     (10)
#define LCD_READ_DELAY      (5)

/* LCD error code definitions. */
#define LCD_TIME_OUT        -1300
#define LCD_COLUMN_FULL     -1301
#define LCD_ROW_FULL        -1302
#define LCD_INTERNAL_ERROR  -1303

/* LCD driver. */
 typedef struct _lcd
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

} LCD;

/* Include LCD target configuration. */
#include <lcd_target.h>

/* Function prototypes. */
void lcd_init();
void lcd_register(LCD *);
int32_t lcd_wait_8bit(LCD *);
void lcd_send_nibble(LCD *, uint8_t);
int32_t lcd_write_register(LCD *, uint8_t, uint8_t);
int32_t lcd_read_register(LCD *, uint8_t, uint8_t *);
int32_t lcd_is_busy(LCD *);
int32_t lcd_create_custom_char(LCD *, uint8_t, uint8_t *);
int32_t lcd_write(void *, uint8_t *, int32_t);

#endif /* CONFIG_LCD */

#endif /* _LCD_H_ */
