/*
 * fuses.c
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#include <avr/io.h>

/* Fuse configuration for AVR (atmegaXXX4). */
FUSES = {
    .low = (FUSE_SUT0 & FUSE_SUT1),
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT
};
