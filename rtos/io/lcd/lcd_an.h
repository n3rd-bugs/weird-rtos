/*
 * lcd_an.h
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
#ifndef _LCD_AN_H_
#define _LCD_AN_H_

#include <kernel.h>

#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */

#ifdef CONFIG_LCD_AN
#ifdef FS_CONSOLE
#include <console.h>
#else
#error "Console driver is required for LCD_AN driver."
#endif /* CONFIG_LCD_AN */
#include <lcd_an_config.h>

/* LCD error code definitions. */
#define LCD_AN_TIME_OUT         -1300
#define LCD_AN_COLUMN_FULL      -1301
#define LCD_AN_ROW_FULL         -1302
#define LCD_AN_INTERNAL_ERROR   -1303

/* LCD register flags. */
#define LCD_DATA_REG            (0x01)
#define LCD_IGNORE_WAIT         (0x02)

/* LCD device flags. */
#define LCD_FLAG_DEBUG          (0x01)

/* LCD IOCTL commands. */
#define LCD_AN_CUSTOM_CHAR     (1)
#define LCD_AN_RESET           (2)

/* LCD structure. */
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

    /* Device flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];
};

/* Alphanumeric LCD ioctl data. */
typedef struct _lcd_an_ioctl_data
{
    /* LCD IOCTL data. */
    uint32_t    index;
    void        *param;
} LCD_AN_IOCTL_DATA;

/* Alphanumeric LCD debug file descriptor. */
extern FD lcd_an_fd;

/* Function prototypes. */
void lcd_an_init(void);
void lcd_an_register(LCD_AN *);
int32_t lcd_an_reset(LCD_AN *);

#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_H_ */
