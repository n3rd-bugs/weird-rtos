# Introduction
Weird RTOS provides boot loader for supported AVR devices. This uses audrino style bootloader and can be used to load new programs using UART and MMC.
## Default Configuration for UART interface
Default baudrate is `115200`, that can be updated in bootload_avr.h, please select a one with least error %.
Use `arduino` as programmer for avrdude.
An example command can be:
``avrdude.exe -pm1284p -carduino -PCOM1 -b115200 -Uflash:w:hello.hex:a``
## Preparing MMC card for Bootload
 Use these steps to prepare a MMC card to be used for bootloading a HEX file on AVR:
 1. Format a MMC card leaving some space at the end, around 10MB should be more than enough.
 2. Note down the partition size in bytes.
 3. Divide that partition size by 512 to convert it in to LBA or sector number.
 4. Pick a sector after that e.g. 15720500 (0x00EFE034).
 5. Update the *BOOTLOAD_MMC_BOOTLOAD_MARK_SECTOR_LOCATION* with this address.
 6. Post the bootload marker on that sector like this:
  ``printf '\x00\xEF\xE0\x35wEirD' | sudo dd bs=512 of=/dev/sdd seek=15720500``
 7. Where **\x00\xEF\xE0\x35** represents the 0x00EFE035 the sector at which we will be dumping our HEX image, and **wEirD** is the mark that will be used to validate if this is a valid sector.
 8. Now dump the HEX file at the next sector like this:
 ``sudo dd bs=512 if=hello.hex of=/dev/sdd seek=15720501``
## Flashing Initial Image
*arduino-isp.ino* can be used with an Arduino board to flash an initial image.
For that use baudrate as `115200` and `stk500v1` as programmer for avrdude.
An example command can be:
``avrdude.exe -pm1284p -cstk500v1 -PCOM2 -b115200 -Uflash:w:hello.hex:a``
## Supported MCUs
For now following targets are supported:
 1. atmega644p
 2. atmega1284p
