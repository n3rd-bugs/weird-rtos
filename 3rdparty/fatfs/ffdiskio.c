/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffdiskio.h"		/* FatFs lower layer API */

/* List of registered file systems. */
static FF_DEVICE ff_devices[FF_NUM_DEVICES];

/*-----------------------------------------------------------------------*/
/* Returns the device at the given index                                 */
/*-----------------------------------------------------------------------*/

FF_DEVICE *disk_search (
	BYTE pdrv		/* Physical drive number to serch */
)
{
	uint32_t index;
	FF_DEVICE *device = NULL;

	/* Search the device list. */
	for (index = 0; index < FF_NUM_DEVICES; index++)
	{
		/* If this is the required device. */
		if ((ff_devices[index].spi_device != NULL) && (ff_devices[index].phy_index == pdrv))
		{
			/* Return this device. */
			device = &ff_devices[index];
			break;
		}
	}

	/* Return the resolved device. */
	return (device);

}

/*-----------------------------------------------------------------------*/
/* Returns the device at the given index                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_register (
	MMC_SPI *spi_device,	/* SPI device to be registered. */
	BYTE pdrv				/* Physical drive index. */
)
{
	DSTATUS status = RES_OK;
	uint32_t index;

	/* If drive is not already registered. */
	if (disk_search(pdrv) == NULL)
	{
		/* Search the device list for a free slot. */
		for (index = 0; index < FF_NUM_DEVICES; index++)
		{
			/* If this is a free slot. */
			if (ff_devices[index].spi_device == NULL)
			{
				/* Register this device.. */
				ff_devices[index].spi_device = spi_device;
				ff_devices[index].phy_index = pdrv;

				break;
			}
		}
	}
	else
	{
		/* Invalid device was given. */
		status = RES_PARERR;
	}

	/* Return status to the caller. */
	return (status);

}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	FF_DEVICE *device = disk_search(pdrv);

	/* Return if the device is initialized. */
	return (((device != NULL) ? ((device->initialized == TRUE) ? RES_OK : STA_NOINIT) : RES_PARERR));
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	FF_DEVICE *device = disk_search(pdrv);
	DSTATUS status = RES_OK;

	/* If the given device was resolved. */
	if (device != NULL)
	{
		/* Initialize SPI. */
		if (mmc_spi_init(device->spi_device) == SUCCESS)
		{
			/* Device is now initialized. */
			device->initialized = TRUE;
		}
		else
		{
			/* Device is not ready. */
			status = RES_NOTRDY;
		}
	}
	else
	{
		/* Invalid device was given. */
		status = RES_PARERR;
	}

	/* Return status to the caller. */
	return (status);

}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	FF_DEVICE *device = disk_search(pdrv);
	uint64_t offset = 0;
	DRESULT status = RES_OK;

	/* If the given device was resolved. */
	if (device != NULL)
	{
		/* Read and return required sector. */
		if ((mmc_spi_read(device->spi_device, sector, &offset, buff, count * MMC_SPI_SECTOR_SIZE) != SUCCESS) ||
			(mmc_spi_read(device->spi_device, sector, &offset, NULL, 0) != SUCCESS))
		{
			/* Read write error. */
			status = RES_ERROR;
		}
	}
	else
	{
		/* Invalid device was given. */
		status = RES_PARERR;
	}

	/* Return status to the caller. */
	return (status);

}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	FF_DEVICE *device = disk_search(pdrv);
	uint64_t offset = 0;
	DRESULT status = RES_OK;

	/* If the given device was resolved. */
	if (device != NULL)
	{
		/* Read and return required sector. */
		if ((mmc_spi_write(device->spi_device, sector, &offset, (uint8_t *)buff, count * MMC_SPI_SECTOR_SIZE) != SUCCESS) ||
			(mmc_spi_write(device->spi_device, sector, &offset, NULL, 0) != SUCCESS))
		{
			/* Read write error. */
			status = RES_ERROR;
		}
	}
	else
	{
		/* Invalid device was given. */
		status = RES_PARERR;
	}

	/* Return status to the caller. */
	return (status);
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;

	UNUSED_PARAM(pdrv);
	UNUSED_PARAM(buff);

	switch (cmd)
	{
	case CTRL_SYNC:
		res = RES_OK;
		break;
	}

	return (res);
}

