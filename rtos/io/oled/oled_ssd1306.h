/*
 * oled_ssd1306.h
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
#ifndef _OLED_SSD1306_H_
#define _OLED_SSD1306_H_
#include <kernel.h>

#ifdef CONFIG_FS
#include <fs.h>
#endif

#ifdef CONFIG_OLED
#ifndef CONFIG_I2C
#error "I2C is required for OLED."
#endif /* CONFIG_I2C */
#include <i2c.h>

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
#define SSD1306_EXTERNAL_VCC        (0x01)

/* OLED driver data. */
typedef struct _ssd1306
{
    /* I2C device associated with this OLED. */
    I2C_DEVICE  i2c;

    /* OLED dimensions. */
    uint8_t     height;
    uint8_t     width;

    /* Flags. */
    uint8_t     flags;

} SSD1306;

/* Function prototypes. */
void oled_ssd1306_init(void);
int32_t oled_ssd1306_register(SSD1306 *);
int32_t oled_ssd1306_display(SSD1306 *, uint8_t *, uint8_t, uint8_t, uint8_t, uint8_t);
int32_t oled_ssd1306_invert(SSD1306 *, uint8_t);

#endif /* CONFIG_OLED */
#endif /* _OLED_SSD1306_H_ */
