/*
 * bootload_atmega644p.c
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
#include <kernel.h>

#ifdef CONFIG_BOOTLOAD
#include <bootload_atmega644p.h>
#include <stk500v1.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#define BAUD        BOOT_BAUD_RATE
#define BAUD_TOL    BOOT_BAUD_TOL
#include <util/setbaud.h>
#include <avr/wdt.h>

#ifndef BOOTLOADER_LOADED

/* Internal function definitions. */
static void bootload_application() BOOTLOAD_SECTION;
#ifdef BOOTLOAD_MMC
static int32_t bootload_mmc() BOOTLOAD_SECTION;
static uint8_t bootload_chrtodec(uint8_t) BOOTLOAD_SECTION;
static uint8_t bootload_hextoint8(uint8_t *) BOOTLOAD_SECTION;
#ifdef BOOTLOAD_MMC_HEX_NONLINEAR
static uint16_t bootload_hextoint16(uint8_t *) BOOTLOAD_SECTION;
#endif /* BOOTLOAD_MMC_HEX_NONLINEAR */
#endif /* BOOTLOAD_MMC */
#ifdef BOOTLOAD_STK
static int32_t bootload_stk() BOOTLOAD_SECTION;
static void stk500_reply(uint8_t) BOOTLOAD_SECTION;
static void stk500_empty_reply() BOOTLOAD_SECTION;
static void bootload_atmega644p_putc(volatile uint8_t) BOOTLOAD_SECTION;
static uint8_t bootload_atmega644p_getc() BOOTLOAD_SECTION;
#endif /* BOOTLOAD_STK */
#if (defined(BOOTLOAD_MMC) || defined(BOOTLOAD_STK))
static void bootload_atmega644p_flush_buffer(uint8_t *, uint32_t, uint32_t) BOOTLOAD_SECTION;
#endif /* (defined(BOOTLOAD_MMC) || defined(BOOTLOAD_STK)) */
#endif /* BOOTLOADER_LOADED */

#ifdef BOOTLOADER_LOADED

/*
 * bootload_atmega644p
 * This function will jump to already flashed boot loader's reset vector
 * causing boot loader to execute.
 */
void bootload_atmega644p()
{
    /* Jump to boot loader reset vector. */
    asm volatile (
                  "jmp  %[boot_vector]      \n\t"
                  :: [boot_vector] "g" (BOOTLOAD_RESET)
                  );
} /* bootload_atmega644p */

/* Stubbed vector table definition. */
void bootvector_table() STACK_LESS BOOTVECTOR_SECTION;
void bootvector_table()
{
    ;
} /* bootvector_table */

#else

/* Boot loader vector table definition. */
void bootvector_table() STACK_LESS BOOTVECTOR_SECTION;
void bootvector_table()
{
    asm volatile (
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  "jmp    bootload_atmega644p   \n\t"
                  ::
                  );
}

/*
 * bootload_atmega644p
 * This function is responsible for performing boot loader operation if
 * required.
 */
void bootload_atmega644p()
{
    int32_t status = SUCCESS;

    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* Switch to boot loader vector table. */
    MCUCR = (1 << IVCE);
    MCUCR = (1 << IVSEL);

#ifdef BOOTLOAD_STK
    /* Initialize the serial interface. */
    /* Set the configured baud-rate. */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    /* Enable RX and TX. */
    UCSR0B = (1 << RXEN0)  | (1 << TXEN0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
#endif /* BOOTLOAD_STK */

    /* Configure the progress and error LEDs. */
    DDRC |= (1 << 3);
    PORTC |= (1 << 3);

#ifdef BOOTLOAD_MMC
    /* Perform MMC boatload. */
    status = bootload_mmc();
#endif
#if (defined(BOOTLOAD_MMC) || defined(BOOTLOAD_STK))
    /* If MMC was not successful. */
    if (status != BOOTLOAD_COMPLETE)
#endif /* (defined(BOOTLOAD_MMC) || defined(BOOTLOAD_STK)) */
    {
#ifdef BOOTLOAD_STK
        /* Perform STK boatload. */
        status = bootload_stk();
#endif
    }

    /* Turn off the progress LED. */
    PORTC &= (uint8_t)~(1 << 3);

    /* If boot loader is now complete. */
    if (status == BOOTLOAD_COMPLETE)
    {
        /* Start the application. */
        bootload_application();
    }

} /* bootload_atmega644p */

/*
 * bootload_application
 * This function will reset the target to start the application.
 */
static void bootload_application()
{
    /* Trigger a soft reset using watch dog timer. */
    wdt_enable(WDTO_15MS);

} /* bootload_application */

#ifdef BOOTLOAD_MMC
/*
 * bootload_mmc
 * @return: BOOTLOAD_COMPLETE will be returned if boatload process was
 *  successfully completed, otherwise success will be returned.
 * This function will perform MMC boot-load process.
 */
static int32_t bootload_mmc()
{
    int32_t status = SUCCESS;
    uint32_t i, offset, page_offset = 0, start_offset;
    uint8_t type, boot_page_buffer[256], hex_buffer[12], data_size = 0;
#ifdef BOOTLOAD_MMC_HEX_NONLINEAR
    uint32_t data_addr = 0;
    int32_t current_page_address = -1;
#else
    int32_t current_page_address = 0;
#endif

    /* Initialize MMC card. */
    if (bootload_disk_initialize(&type) == SUCCESS)
    {
        /* Read the HEX offset. */
        offset = 0;
        bootload_disk_read(type, hex_buffer, 0, 9, &offset);
        start_offset = ((uint32_t)hex_buffer[0] << 24) + ((uint32_t)hex_buffer[1] << 16) + ((uint32_t)hex_buffer[2] << 8) + ((uint32_t)hex_buffer[3] << 0);
        bootload_disk_read(type, NULL, 0, 0, &offset);

        /* If we do have a valid sector offset. */
        offset = 0;
        if ((hex_buffer[4] == 'w') && (hex_buffer[5] == 'E') && (hex_buffer[6] == 'i') && (hex_buffer[7] == 'r') && (hex_buffer[8] == 'D'))
        {
            /* Read a byte. */
            bootload_disk_read(type, hex_buffer, start_offset, 1, &offset);

            /* If we have expected data. */
            while ((status == SUCCESS) && (hex_buffer[0] == ':'))
            {
                /* Read a chunk of HEX. */
                bootload_disk_read(type, hex_buffer, start_offset, 8, &offset);

                /* Process the record type. */
                switch (bootload_hextoint8(&hex_buffer[6]))
                {
                /* If this is a data record. */
                case 0:

#ifdef BOOTLOAD_MMC_HEX_NONLINEAR
                    /* If we are switching to a new location. */
                    if ((current_page_address != -1) && ((data_addr + data_size) != ((data_addr & 0xFFFF0000) + bootload_hextoint16(&hex_buffer[2]))))
                    {
                        /* Flush the current page buffer. */
                        bootload_atmega644p_flush_buffer(boot_page_buffer, current_page_address, page_offset);

                        /* Invalidate the current page address. */
                        current_page_address = -1;
                    }

                    /* Save the load address. */
                    data_addr = ((data_addr & (0xFFFF0000)) + bootload_hextoint16(&hex_buffer[2]));

                    /* If a new page address is required. */
                    if (current_page_address == -1)
                    {
                        /* Calculate the offset in the page buffer. */
                        page_offset = (data_addr % 256);

                        /* Calculate the current page address. */
                        current_page_address = (data_addr / 256) * 256;
                    }
#endif

                    /* Save the number of bytes we are expecting in the data. */
                    data_size = bootload_hextoint8(&hex_buffer[0]);

                    /* Read and fill the page buffer. */
                    for (i = 0; (i < data_size); i++)
                    {
                        /* Read 2 bytes in the hex buffer. */
                        bootload_disk_read(type, hex_buffer, start_offset, 2, &offset);

                        /* Save a byte in the page buffer. */
                        boot_page_buffer[page_offset] = bootload_hextoint8(hex_buffer);

                        /* Go  to next offset in the page buffer. */
                        page_offset ++;

                        /* If we are at the end of a page. */
                        if (page_offset == 256)
                        {
                            /* Flush the page buffer. */
                            bootload_atmega644p_flush_buffer(boot_page_buffer, current_page_address, page_offset);

                            /* Reset the page offset. */
                            page_offset = 0;

                            /* Update the current page address. */
                            current_page_address += 256;
                        }
                    }

                    break;

                /* If this is the end of HEX file. */
                case 1:

                    /* Flush the page buffer. */
                    bootload_atmega644p_flush_buffer(boot_page_buffer, current_page_address, page_offset);

                    /* Boot loader is now complete. */
                    status = BOOTLOAD_COMPLETE;

                    break;

                /* If this is an extended segment address. */
                case 2:

#ifdef BOOTLOAD_MMC_HEX_NONLINEAR
                    /* Flush the current page buffer. */
                    bootload_atmega644p_flush_buffer(boot_page_buffer, current_page_address, page_offset);

                    /* Invalidate the current page address. */
                    current_page_address = -1;
#endif

                    /* Read extended segment address. */
                    bootload_disk_read(type, hex_buffer, start_offset, 4, &offset);

#ifdef BOOTLOAD_MMC_HEX_NONLINEAR
                    /* Save the extended segment address. */
                    data_addr = (((uint32_t)bootload_hextoint16(hex_buffer)) << 4);
#endif

                    break;

                /* Unknown HEX record. */
                default:

                    /* Boot loader encountered an error. */
                    status = BOOTLOAD_ERROR;

                    break;
                }

                /* Read and discard the CRC. */
                bootload_disk_read(type, hex_buffer, start_offset, 2, &offset);

                /* Goto the start of next record. */
                while (TRUE)
                {
                    /* Read a byte. */
                    bootload_disk_read(type, hex_buffer, start_offset, 1, &offset);

                    /* If we have a line terminator. */
                    if ((hex_buffer[0] != '\r') && (hex_buffer[0] != '\n'))
                    {
                        break;
                    }
                }
            }
        }

        /* Terminate the read. */
        bootload_disk_read(type, NULL, 0, 0, &offset);
    }

    /* Return status to the caller. */
    return (status);

} /* bootload_mmc */

/*
 * bootload_chrtodec
 * @c: HEX character to be converted.
 * @return: Decimal equivalent of the given character.
 * This function will convert a HEX character to it's decimal equivalent.
 */
static uint8_t bootload_chrtodec(uint8_t c)
{
    if (c >= '0' && c <= '9') c = c - '0';
    else if (c >= 'a' && c <='f') c = c - 'a' + 10;
    else if (c >= 'A' && c <='F') c = c - 'A' + 10;

    /* Return the decimal equivalent of the given character. */
    return (c);

} /* bootload_chrtodec */

/*
 * bootload_hextoint8
 * @hex: HEX string to be converted.
 * @return: Decimal equivalent of the given string.
 * This function will convert a HEX string to decimal equivalent.
 */
static uint8_t bootload_hextoint8(uint8_t *hex)
{
    /* Convert each byte and return the decimal value. */
    return ((bootload_chrtodec(hex[0]) << 4) + bootload_chrtodec(hex[1]));

} /* bootload_hextoint8 */

#ifdef BOOTLOAD_MMC_HEX_NONLINEAR
/*
 * bootload_hextoint16
 * @hex: HEX string to be converted.
 * @return: Decimal equivalent of the given string.
 * This function will convert a HEX string to decimal equivalent.
 */
static uint16_t bootload_hextoint16(uint8_t *hex)
{
    /* Convert each byte and return the decimal value. */
    return ((((uint16_t)bootload_hextoint8(&hex[0])) << 8) + ((uint16_t)bootload_hextoint8(&hex[2])));

} /* bootload_hextoint16 */
#endif /* BOOTLOAD_MMC_HEX_NONLINEAR */
#endif /* BOOTLOAD_MMC */

#ifdef BOOTLOAD_STK
/*
 * bootload_stk
 * @return: BOOTLOAD_COMPLETE will be returned if boatload process was
 *  successfully completed.
 * This function will perform STK boot-load process.
 */
static int32_t bootload_stk()
{
    int32_t status = SUCCESS;
    uint32_t load_address = 0,  i;
    uint16_t size, page_size = 256;
    uint8_t byte, boot_page_buffer[256];

    /* Process programmer commands. */
    while (status != BOOTLOAD_COMPLETE)
    {
        /* Get a byte from the serial. */
        byte = bootload_atmega644p_getc();

        /* Process incoming data. */
        switch (byte)
        {
        /* Get sync command. */
        case STK_GET_SYNC:

            /* Send an empty reply. */
            stk500_empty_reply();

            break;

        /* Signing on. */
        case STK_GET_SIGN_ON:

            /* If we have a valid byte. */
            if (bootload_atmega644p_getc() == STK_CRC_EOP)
            {
                /* Send the version to the other end. */
                bootload_atmega644p_putc(STK_INSYNC);
                bootload_atmega644p_putc('A');
                bootload_atmega644p_putc('V');
                bootload_atmega644p_putc('R');
                bootload_atmega644p_putc(' ');
                bootload_atmega644p_putc('I');
                bootload_atmega644p_putc('S');
                bootload_atmega644p_putc('P');
                bootload_atmega644p_putc(STK_OK);
            }
            else
            {
                /* We are not in sync. */
                bootload_atmega644p_putc(STK_NOSYNC);
            }

            break;

        /* Need to fetch a parameter information. */
        case STK_GET_PARAMETER:

            /* Get the information other end require. */
            switch (bootload_atmega644p_getc())
            {

            /* Need hardware revision. */
            case STK_READ_HWVER:

                /* Send hardware revision. */
                stk500_reply(STK_HWVER);

                break;

            /* Need software revision major. */
            case STK_READ_SWMAJ:

                /* Send software revision major. */
                stk500_reply(STK_SWMAJ);

                break;

            /* Need software revision minor. */
            case STK_READ_SWMIN:

                /* Send software revision minor. */
                stk500_reply(STK_SWMIN);

                break;

            /* Need programmer type. */
            case STK_READ_PGTYPE:

                /* Send that we are serial programmer. */
                stk500_reply('S');

                break;

            default:

                /* Unknown parameter. */
                stk500_reply(0);

                break;
            }

            break;

        /* Set device information. */
        case STK_SET_DEVICE:

            /* Read and discard first 12 bytes. */
            for (i = 0; i < 12; i ++)
            {
                byte = bootload_atmega644p_getc();
            }

            /* Just save the page size for the device. */
            page_size = ((uint16_t)bootload_atmega644p_getc() << 8);
            page_size |= ((uint16_t)bootload_atmega644p_getc());

            /* Read and discard next 6 bytes. */
            for (i = 0; i < 6; i ++)
            {
                byte = bootload_atmega644p_getc();
            }

            /* Send an empty reply. */
            stk500_empty_reply();

            break;

        /* Enable programmer. */
        case STK_ENTER_PROGMODE:

            /* Send empty reply. */
            stk500_empty_reply();

            break;

        /* Disable programmer. */
        case STK_LEAVE_PROGMODE:

            /* Send empty reply. */
            stk500_empty_reply();

            /* Boot loader is now complete. */
            status = BOOTLOAD_COMPLETE;

            break;

        /* Extended parameters. */
        case STK_SET_DEVICE_EXT:

            /* Read and discard 5 bytes. */
            for (i = 0; i < 5; i ++)
            {
                byte = bootload_atmega644p_getc();
            }

            /* Send empty reply. */
            stk500_empty_reply();

            break;

        /* Load address. */
        case STK_LOAD_ADDRESS:

            /* Save the load address. */
            load_address = bootload_atmega644p_getc();
            load_address |= bootload_atmega644p_getc() << 8;
            load_address += load_address;

            /* Send empty reply. */
            stk500_empty_reply();

            break;

        /* Universal command. */
        case STK_UNIVERSAL:

            /* Read and discard 4 bytes. */
            for (i = 0; i < 4; i ++)
            {
                byte = bootload_atmega644p_getc();
            }

            /* Send a zero byte in response. */
            stk500_reply(0x00);

            break;

        /* Flash program. */
        case STK_PROG_FLASH:

            /* Read and discard 2 bytes. */
            for (i = 0; i < 2; i ++)
            {
                byte = bootload_atmega644p_getc();
            }

            /* Send empty reply. */
            stk500_empty_reply();

            break;

        /* Data program. */
        case STK_PROG_DATA:

            /* Read and discard 1 byte. */
            byte = bootload_atmega644p_getc();

            /* Send empty reply. */
            stk500_empty_reply();

            break;

        /* Page program. */
        case STK_PROG_PAGE:

            /* Receive the size of data we need to program. */
            size = bootload_atmega644p_getc() << 8;
            size |= bootload_atmega644p_getc();

            /* Get the region needed to be programmed. */
            switch (bootload_atmega644p_getc())
            {
            /* Flash is being programmed. */
            case 'F':

                /* Receive page data. */
                for (i = 0; i < size; i++)
                {
                    boot_page_buffer[i] = bootload_atmega644p_getc();
                }

                /* If we have expected byte. */
                if (bootload_atmega644p_getc() == STK_CRC_EOP)
                {
                    /* Send that we are in sync. */
                    bootload_atmega644p_putc(STK_INSYNC);

                    /* Flush program buffer. */
                    bootload_atmega644p_flush_buffer(boot_page_buffer, load_address, size);

                    /* Page write complete. */
                    bootload_atmega644p_putc(STK_OK);
                }
                else
                {
                    /* We are not in sync. */
                    bootload_atmega644p_putc(STK_NOSYNC);
                }

                break;

            /* Unsupported region. */
            default:
                bootload_atmega644p_putc(STK_FAILED);
                break;
            }

            break;

        /* Read a page. */
        case STK_READ_PAGE:

            /* Receive the size of data we need to read. */
            size = bootload_atmega644p_getc() << 8;
            size |= bootload_atmega644p_getc();

            /* Get the region needed to be read. */
            switch (bootload_atmega644p_getc())
            {
            /* Flash is being read. */
            case 'F':

                /* If we have expected byte. */
                if (bootload_atmega644p_getc() == STK_CRC_EOP)
                {
                    /* Send that we are in sync. */
                    bootload_atmega644p_putc(STK_INSYNC);

                    /* Send back the read memory. */
                    for (i = 0; i < size; i ++)
                    {
                        /* Put a byte on serial. */
                        bootload_atmega644p_putc(pgm_read_byte_far(load_address + i));
                    }

                    /* Page read complete. */
                    bootload_atmega644p_putc(STK_OK);
                }
                else
                {
                    /* We are not in sync. */
                    bootload_atmega644p_putc(STK_NOSYNC);
                }

                break;

            /* Unsupported region. */
            default:
                bootload_atmega644p_putc(STK_FAILED);
                break;
            }

            break;

        /* Need to read signature of the device. */
        case STK_READ_SIGN:

            /* If we have a valid byte. */
            if (bootload_atmega644p_getc() == STK_CRC_EOP)
            {
                /* We are in sync. */
                bootload_atmega644p_putc(STK_INSYNC);

                /* Send device signature bytes. */
                bootload_atmega644p_putc(boot_signature_byte_get(0x00));
                bootload_atmega644p_putc(boot_signature_byte_get(0x02));
                bootload_atmega644p_putc(boot_signature_byte_get(0x04));

                /* Send okay response. */
                bootload_atmega644p_putc(STK_OK);
            }
            else
            {
                /* We are not in sync. */
                bootload_atmega644p_putc(STK_NOSYNC);
            }

            break;

        /* EOP should not come here. */
        case STK_CRC_EOP:

            /* We are not in sync. */
            bootload_atmega644p_putc(STK_NOSYNC);

            break;

        /* Unknown command. */
        default:

            /* If we have a valid byte. */
            if (bootload_atmega644p_getc() == STK_CRC_EOP)
            {
                /* Unknown command was given. */
                bootload_atmega644p_putc(STK_UNKNOWN);
            }
            else
            {
                /* We are not in sync. */
                bootload_atmega644p_putc(STK_NOSYNC);
            }

            break;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* bootload_stk */

/*
 * stk500_reply
 * @byte: Byte to be sent to the other end.
 * This function sends a byte reply to the other end.
 */
static void stk500_reply(uint8_t byte)
{
    /* If we have a valid byte. */
    if (bootload_atmega644p_getc() == STK_CRC_EOP)
    {
        /* Send that we are in sync. */
        bootload_atmega644p_putc(STK_INSYNC);
        bootload_atmega644p_putc(byte);
        bootload_atmega644p_putc(STK_OK);
    }
    else
    {
        /* We are not in sync. */
        bootload_atmega644p_putc(STK_NOSYNC);
    }

} /* stk500_reply */

/*
 * stk500_empty_reply
 * This function sends an empty reply to the other end.
 */
static void stk500_empty_reply()
{
    /* If we have a valid byte. */
    if (bootload_atmega644p_getc() == STK_CRC_EOP)
    {
        /* Send that we are in sync. */
        bootload_atmega644p_putc(STK_INSYNC);
        bootload_atmega644p_putc(STK_OK);
    }
    else
    {
        /* We are not in sync. */
        bootload_atmega644p_putc(STK_NOSYNC);
    }

} /* stk500_empty_reply */

/*
 * bootload_atmega644p_putc
 * @byte: Byte to be sent on console.
 * This function sends a character on the serial.
 */
static void bootload_atmega644p_putc(volatile uint8_t byte)
{
    /* Wait for last byte to be sent. */
    while ((UCSR0A & (1 << UDRE0)) == 0) ;

    /* Put a byte on the serial. */
    UDR0 = byte;

} /* bootload_atmega644p_putc */

/*
 * bootload_atmega644p_getc
 * This function will read and return a byte from the serial.
 */
static uint8_t bootload_atmega644p_getc()
{
    volatile uint8_t byte;

    /* Wait for some data to become available. */
    while ((UCSR0A & (1 << RXC0)) == 0);

    /* Read the incoming byte. */
    byte = UDR0;

    /* Return the received byte. */
    return (byte);

} /* bootload_atmega644p_getc */
#endif /* BOOTLOAD_STK */

#if (defined(BOOTLOAD_MMC) || defined(BOOTLOAD_STK))

/*
 * bootload_atmega644p_flush_buffer
 * @buffer: Page buffer to be converted.
 * @load_address: Page start address.
 * @num_bytes: Bytes loaded in the page buffer.
 * This function will flush program buffer on the given address.
 */
static void bootload_atmega644p_flush_buffer(uint8_t *buffer, uint32_t load_address, uint32_t num_bytes)
{
    uint32_t i;
    uint16_t word;

    /* If we have some data to flush. */
    if (num_bytes > 0)
    {
        /* Toggle the progress LED. */
        PORTC ^= (1 << 3);

        /* Erase the page we are going to write. */
        boot_page_erase_safe(load_address);

        /* Enable read while write. */
        boot_spm_busy_wait();
        boot_rww_enable();

        /* Read and write data on the flash. */
        for (i = 0; i < num_bytes; i += 2)
        {
            /* Fill data for the page. */
            word = (uint16_t)(buffer[i]);
            word |= (uint16_t)(buffer[i + 1] << 8);
            boot_page_fill_safe(load_address + i, word);
        }

        /* Write the page buffer. */
        boot_page_write_safe(load_address);

        /* Enable read while write. */
        boot_spm_busy_wait();
        boot_rww_enable();
    }

} /* bootload_atmega644p_flush_buffer */
#endif /* (defined(BOOTLOAD_MMC) || defined(BOOTLOAD_STK)) */

#endif /* BOOTLOADER_LOADED */
#endif /* CONFIG_BOOTLOAD */
