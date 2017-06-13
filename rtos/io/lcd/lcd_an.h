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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
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

/* LCD strcuture. */
typedef struct _lcd_an LCD_AN;

/* Target APIs. */
typedef void (LCD_SET_EN)(LCD_AN *);
typedef void (LCD_CLR_EN)(LCD_AN *);
typedef void (LCD_SET_RS)(LCD_AN *);
typedef void (LCD_CLR_RS)(LCD_AN *);
typedef void (LCD_SET_RW)(LCD_AN *);
typedef void (LCD_CLR_RW)(LCD_AN *);
typedef void (LCD_PUT_DATA)(LCD_AN *, uint8_t);
typedef uint8_t (LCD_READ_DATA)(LCD_AN *);

/* Alphanumeric LCD driver data. */
struct _lcd_an
{
    /* Console data for this driver. */
    CONSOLE     console;

    /* Private data. */
    void        *priv_data;

    /* Target hooks. */
    LCD_SET_EN  *set_en;
    LCD_CLR_EN  *clr_en;
    LCD_SET_RS  *set_rs;
    LCD_CLR_RS  *clr_rs;
    LCD_SET_RW  *set_rw;
    LCD_CLR_RW  *clr_rw;
    LCD_PUT_DATA    *put_data;
    LCD_READ_DATA   *read_data;

    /* Number of rows and column on the LCD. */
    uint16_t    row;
    uint16_t    column;

    /* Current location of cursor. */
    uint16_t    cur_row;
    uint16_t    cur_column;
};

/* Alphanumeric LCD ioctl data. */
typedef struct _lcd_an_ioctl_data
{
    /* LCD IOCTL data. */
    uint32_t    index;
    void        *param;
} LCD_AN_IOCTL_DATA;

/* Function prototypes. */
void lcd_an_init();
void lcd_an_register(LCD_AN *);

#endif /* CONFIG_LCD_AN */

#endif /* _LCD_AN_H_ */
