/*
 * ds182x.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <ds182x.h>

#ifdef GPIO_DS182X
#include <string.h>

/*
 * ds182x_init
 * This function will initialize DS182X bus.
 */
void ds182x_init(void)
{
#ifdef DS182X_TGT_INIT
    DS182X_TGT_INIT();
#endif
} /* ds182x_init */

/*
 * ds182x_register
 * @bus: DS182X bus needed to be registered.
 * @return: Success will be returned if ds182x bus was successfully registered.
 * This function will register a DS182X bus.
 */
int32_t ds182x_register(DS182X *bus)
{
    int32_t status;

    /* Initialize 1-wire interface for this sensor. */
    status = onewire_init(&bus->onewire);

    /* Return status to the caller. */
    return (status);

} /* ds182x_register */

/*
 * ds182x_get_first
 * @bus: DS182X bus from which we need the reading of first sensor.
 * @temp: Will return the read temperature here in mC.
 * @return: Success will be returned if a reading was successfully returned,
 *  ONEWIRE_NO_DEVICE will be returned if there is no device on the bus.
 * This function will read the sensor reading of the first sensor found on the
 *  ds182x bus.
 */
int32_t ds182x_get_first(DS182X *bus, uint16_t *temp)
{
    int32_t status;

    /* Clear the ROM address. */
    memset(bus->rom_addr, 0, sizeof(bus->rom_addr));

    /* Get reading from next device. */
    status = ds182x_get_next(bus, temp);

    /* Return status to the caller. */
    return (status);

} /* ds182x_get_first */

/*
 * ds182x_get_next
 * @bus: DS182X bus from which we need the reading of next sensor.
 * @temp: Will return the read temperature here in mC.
 * @return: Success will be returned if a reading was successfully returned,
 *  ONEWIRE_NO_DEVICE will be returned if there is no more devices on the bus.
 * This function will read the sensor reading of the next sensor found on the
 *  ds182x bus.
 */
int32_t ds182x_get_next(DS182X *bus, uint16_t *temp)
{
    int32_t status = SUCCESS;
    uint8_t conv_complete, scratch_pad[DS182X_SC_SIZE];

    for (;(status == SUCCESS);)
    {
        /* Search the 1-wire bus for sensors. */
        status = onewire_search(&bus->onewire, ONEWIRE_CMD_DEV_SEARCH, bus->rom_addr);

        if (status == SUCCESS)
        {
            /* Validate if we have a valid device address. */
            /* Validate if we have a valid sensor family. */
            if ((onewire_crc(bus->rom_addr, 7) == bus->rom_addr[7]) && ((bus->rom_addr[0] == DS182X_DS18S20) || (bus->rom_addr[0] == DS182X_DS18B20) || (bus->rom_addr[0] == DS182X_DS1822) || (bus->rom_addr[0] == DS182X_DS1825)))
            {
                /* Reset 1-wire bus */
                status = onewire_reset(&bus->onewire);

                if (status == SUCCESS)
                {
                    /* Select this ROM. */
                    status = onewire_select(&bus->onewire, bus->rom_addr);
                }

                if (status == SUCCESS)
                {
                    /* Start the temperature conversion. */
                    status = onewire_write(&bus->onewire, (uint8_t []){DS182X_STARTCONV}, 1);
                }

                if (status == SUCCESS)
                {
                    /* Wait for the conversion to complete. */
                    POLL_SW_MS(((status = onewire_read_bit(&bus->onewire, &conv_complete)) == SUCCESS) && (conv_complete == FALSE), DS182X_CONV_MAX_TIME, status, DS182X_CONV_TIMEOUT);
                }

                if (status == SUCCESS)
                {
                    /* Reset 1-wire bus */
                    status = onewire_reset(&bus->onewire);
                }

                if (status == SUCCESS)
                {
                    /* Skip ROM. */
                    status = onewire_write(&bus->onewire, (uint8_t []){ONEWIRE_CMD_SKIP_ROM}, 1);
                }

                if (status == SUCCESS)
                {
                    /* Send the read scratch pad command. */
                    status = onewire_write(&bus->onewire, (uint8_t []){DS182X_READSCRATCH}, 1);
                }

                if (status == SUCCESS)
                {
                    /* Read the scratch pad. */
                    status = onewire_read(&bus->onewire, scratch_pad, DS182X_SC_SIZE);
                }

                /* Validate crc. */
                if ((status == SUCCESS) && ((onewire_crc(scratch_pad, DS182X_SC_SIZE - 1) == scratch_pad[DS182X_SC_CRC])))
                {
                    /* Return the calculated temperature. */
                    *temp = (uint16_t)((((uint16_t)scratch_pad[DS182X_SC_TEMP_MSB]) << 8) | scratch_pad[DS182X_SC_TEMP_LSB]);

                    /* Convert the temperature into mC. */
                    *temp = (uint16_t)(((*temp) * 1000) / 16);

                    /* Break out of this loop. */
                    break;
                }
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ds182x_get_next */

#endif /* GPIO_DS182X */
