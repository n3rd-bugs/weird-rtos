#include <avr/io.h>

FUSES = {
    .low = (FUSE_SUT0 & FUSE_SUT1),
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT
};
