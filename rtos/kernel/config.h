/*
 * config.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* If we are building from cmake. */
#ifdef CMAKE_BUILD
#include <config_cmake.h>
#else

/* Helper API configuration. */
#define CONFIG_SLEEP
#define CONFIG_SEMAPHORE

/* Memory manger. */
//#define CONFIG_MEMGR

/* File system. */
//#define CONFIG_FS

/* Networking stack. */
//#define CONFIG_NET

/* Device IO. */
//#define CONFIG_SERIAL
//#define CONFIG_I2C
//#define CONFIG_PCF8574
//#define CONFIG_SPI
//#define CONFIG_MMC
//#define CONFIG_ADC
//#define CONFIG_LCD_AN
//#define CONFIG_LCD_PCF8574
//#define CONFIG_PPP
//#define CONFIG_ETHERNET

/* API's. */
//#define CONFIG_WEIRD_VIEW
//#define CONFIG_TFTPS
//#define CONFIG_BOOTLOAD

#endif /* CMAKE_BUILD */
#endif /* _CONFIG_H_ */
