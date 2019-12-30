/*
 * config.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
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
//#define CONFIG_OLED

/* API's. */
//#define CONFIG_WEIRD_VIEW
//#define CONFIG_TFTPS
//#define CONFIG_BOOTLOAD

#endif /* CMAKE_BUILD */
#endif /* _CONFIG_H_ */
