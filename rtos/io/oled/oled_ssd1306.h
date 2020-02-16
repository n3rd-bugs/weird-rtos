/*
 * oled_ssd1306.h
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
#ifndef _OLED_SSD1306_H_
#define _OLED_SSD1306_H_
#include <kernel.h>

#ifdef CONFIG_FS
#include <fs.h>
#endif

#ifdef IO_OLED
#ifndef IO_I2C
#error "I2C is required for OLED."
#endif /* IO_I2C */
#include <i2c.h>
#ifndef IO_GFX
#error "Graphics is needed to be enabled for OLED."
#endif /* IO_GFX */
#include <gfx.h>
#include <oled_config.h>

/* Error code definitions. */
#define SSD1306_INIT_ERROR          -2400

/* SSD1306 definitions. */
#define SSD1306_MEMORYMODE          (0x20)
#define SSD1306_COLUMNADDR          (0x21)
#define SSD1306_PAGEADDR            (0x22)
#define SSD1306_DEACTIVATE_SCROLL   (0x2E)
#define SSD1306_SETSTARTLINE        (0x40)
#define SSD1306_SETCONTRAST         (0x81)
#define SSD1306_CHARGEPUMP          (0x8D)
#define SSD1306_SEGREMAP            (0xA0)
#define SSD1306_DISPLAYALLON_RESUME (0xA4)
#define SSD1306_NORMALDISPLAY       (0xA6)
#define SSD1306_INVERTDISPLAY       (0xA7)
#define SSD1306_SETMULTIPLEX        (0xA8)
#define SSD1306_DISPLAYOFF          (0xAE)
#define SSD1306_DISPLAYON           (0xAF)
#define SSD1306_COMSCANDEC          (0xC8)
#define SSD1306_SETDISPLAYOFFSET    (0xD3)
#define SSD1306_SETDISPLAYCLOCKDIV  (0xD5)
#define SSD1306_SETPRECHARGE        (0xD9)
#define SSD1306_SETCOMPINS          (0xDA)
#define SSD1306_SETVCOMDESELECT     (0xDB)

/* OLED configuration flags. */
#define SSD1306_EXTERNAL_VCC        (0x1)

/* OLED driver data. */
typedef struct _ssd1306
{
    /* Graphics data for this driver. */
    GFX         gfx;

    /* I2C device associated with this OLED. */
    I2C_DEVICE  i2c;

    /* Flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];

} SSD1306;

/* Function prototypes. */
void oled_ssd1306_init(void);
int32_t oled_ssd1306_register(SSD1306 *);

#endif /* IO_OLED */
#endif /* _OLED_SSD1306_H_ */
