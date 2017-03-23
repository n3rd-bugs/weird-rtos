/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffdiskio.h"		/* FatFs lower layer API */

#ifdef FS_FAT

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
		if ((ff_devices[index].phy_device != NULL) && (ff_devices[index].phy_index == pdrv))
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
	void *pdevice,			/* Device to be registered. */
	FF_INIT *pinit,			/* Function to be called to initialize the device */
	FF_READ *pread,			/* Function to be called to read from device */
	FF_WRITE *pwrite,		/* Function to be called to write on device */
	uint32_t psector_size,	/* Sector size of this device. */
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
			if (ff_devices[index].phy_device == NULL)
			{
				/* Register this device. */
				ff_devices[index].phy_device = pdevice;
				ff_devices[index].init = pinit;
				ff_devices[index].read = pread;
				ff_devices[index].write = pwrite;
				ff_devices[index].sector_size = psector_size;
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
		if (device->init(device->phy_device) == SUCCESS)
		{
			/* Device is idle. */
			device->state = FDEV_IDLE;
			device->offset = FDEV_STRAT_OFFSET;

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
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	FF_DEVICE *device = disk_search(pdrv);
	DRESULT status = RES_OK;

	/* If the given device was resolved. */
	if (device != NULL)
	{
		/* If we need to terminate an old request, */
		if ((device->state != FDEV_READING) || (device->current_sector != sector))
		{
			/* Synchronize updates on the device. */
			disk_ioctl(pdrv, CTRL_SYNC, NULL);
		}

		/* Read and return required sector. */
		if (device->read(device->phy_device, sector, &device->offset, buff, count * device->sector_size) == SUCCESS)
		{
			/* Update the device state. */
			device->current_sector = sector + count;
			device->state = FDEV_READING;
		}
		else
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
	BYTE pdrv,			/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	FF_DEVICE *device = disk_search(pdrv);
	DRESULT status = RES_OK;

	/* If the given device was resolved. */
	if (device != NULL)
	{
		/* If we need to terminate an old request, */
		if ((device->state != FDEV_WRITING) || (device->current_sector != sector))
		{
			/* Synchronize updates on the device. */
			disk_ioctl(pdrv, CTRL_SYNC, NULL);
		}

		/* Read and return required sector. */
		if (device->write(device->phy_device, sector, &device->offset, (uint8_t *)buff, count * device->sector_size) == SUCCESS)
		{
			/* Update the device state. */
			device->current_sector = sector + count;
			device->state = FDEV_WRITING;
		}
		else
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
    FF_DEVICE *device = disk_search(pdrv);

	UNUSED_PARAM(buff);

	switch (cmd)
	{
	case CTRL_SYNC:

		/* Process the device state. */
		switch (device->state)
		{
		/* If we were writing data. */
		case FDEV_WRITING:

			/* Terminate the old write. */
			device->write(device->phy_device, 0, &device->offset, NULL, 0);
			break;

		/* If we were reading data. */
		case FDEV_READING:

			/* Terminate the old read. */
			device->read(device->phy_device, 0, &device->offset, NULL, 0);
			break;

		default:
			break;
		}

		/* Update the device state. */
		device->state = FDEV_IDLE;
		device->offset = FDEV_STRAT_OFFSET;
		res = RES_OK;

		break;
	}

	return (res);
}
#endif /* FS_FAT */
