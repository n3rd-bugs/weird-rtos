/*
 * bootload_atmega644p.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_BOOTLOAD
#include <bootload_atmega644p.h>
#include <stk500v1.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#define BAUD        BOOT_BAUD_RATE
#define BAUD_TOL    BOOT_BAUD_TOL
#include <util/setbaud.h>

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
void bootvector_table() STACK_LESS BOOTLOAD_SECTION;
void bootvector_table()
{
    ;
} /* bootvector_table */

#else

/* Boot loader vector table definition. */
void bootvector_table() STACK_LESS BOOTLOAD_SECTION;
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
    uint32_t load_address = 0;
    uint32_t i;
    uint16_t size, word;
    uint16_t page_size = 256;
    uint8_t byte;
    uint8_t boot_page_buffer[256];

    /* Initialize boot loader condition. */
    BOOTLOAD_COND_INIT;

    /* If boot condition meets. */
    if (BOOTLOAD_COND)
    {
        /* Disable interrupts. */
        DISABLE_INTERRUPTS();

        /* Switch to boot loader vector table. */
        MCUCR = (1 << IVCE);
        MCUCR = (1 << IVSEL);

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

        /* Process programmer commands. */
        for (;;)
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

                        /* Erase the page we are going to write. */
                        boot_page_erase_safe(load_address);

                        /* Enable read while write. */
                        boot_spm_busy_wait();
                        boot_rww_enable();

                        /* Read and write data on the flash. */
                        for (i = 0; i < size; i += 2)
                        {
                            /* Fill data for the page. */
                            word = (uint16_t)(boot_page_buffer[i]);
                            word |= (uint16_t)(boot_page_buffer[i + 1] << 8);
                            boot_page_fill_safe(load_address + i, word);
                        }

                        /* Write the page buffer. */
                        boot_page_write_safe(load_address);

                        /* Enable read while write. */
                        boot_spm_busy_wait();
                        boot_rww_enable();

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
    }

} /* bootload_atmega644p */

/*
 * stk500_reply
 * @byte: Byte to be sent to the other end.
 * This function sends a byte reply to the other end.
 */
void stk500_reply(uint8_t byte)
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
void stk500_empty_reply()
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
void bootload_atmega644p_putc(volatile uint8_t byte)
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
uint8_t bootload_atmega644p_getc()
{
    volatile uint8_t byte;

    /* Wait for some data to become available. */
    while ((UCSR0A & (1 << RXC0)) == 0);

    /* Read the incoming byte. */
    byte = UDR0;

    /* Return the received byte. */
    return (byte);

} /* bootload_atmega644p_getc */

#endif /* BOOTLOADER_LOADED */
#endif /* CONFIG_BOOTLOAD */
