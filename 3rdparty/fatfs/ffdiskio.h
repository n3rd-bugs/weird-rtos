/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2014          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <os.h>

#ifdef CONFIG_FS
#include <fs.h>

#ifdef FS_FAT
#include "ffinteger.h"

/* WeirdRTOS configuration. */
#define FF_NUM_DEVICES		1

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0 /* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1 /* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2 /* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE		3 /* Get erase block size (needed at _USE_MKFS == 1) */
#define CTRL_TRIM			4 /* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER			5 /* Get/Set power status */
#define CTRL_LOCK			6 /* Lock/Unlock media removal */
#define CTRL_EJECT			7 /* Eject media */
#define CTRL_FORMAT			8 /* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10 /* Get card type */
#define MMC_GET_CSD			11 /* Get CSD */
#define MMC_GET_CID			12 /* Get CID */
#define MMC_GET_OCR			13 /* Get OCR */
#define MMC_GET_SDSTAT		14 /* Get SD status */
#define ISDIO_READ			55 /* Read data form SD iSDIO register */
#define ISDIO_WRITE			56 /* Write data to SD iSDIO register */
#define ISDIO_MRITE			57 /* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20 /* Get F/W revision */
#define ATA_GET_MODEL		21 /* Get model name */
#define ATA_GET_SN			22 /* Get serial number */

/* Device states */
#define FDEV_IDLE			0
#define FDEV_READING		1
#define FDEV_WRITING		2

/* Disk read/write definitions. */
#define FDEV_STRAT_OFFSET	(uint64_t)(-1)

/* Device APIs. */
typedef int32_t FF_INIT(void *);
typedef int32_t FF_READ(void *, uint32_t, uint64_t *, uint8_t *, int32_t);
typedef int32_t FF_WRITE(void *, uint32_t, uint64_t *, uint8_t *, int32_t);

/* FatFile system device definition. */
typedef struct _ff_device
{
	/* Device manipulation APIs. */
	FF_INIT		*init;
	FF_READ		*read;
	FF_WRITE	*write;

	/* Physical device to be used. */
	void		*phy_device;

	/* Device data cursor. */
	uint64_t	current_sector;
	uint64_t	offset;

	/* Sector size of this device. */
	uint32_t	sector_size;

	/* If this device is initialized. */
	uint8_t		initialized;

	/* Index of this device. */
	uint8_t		phy_index;

	/* Current device state. */
	uint8_t		state;

	/* Structure padding. */
	uint8_t		pad[1];

} FF_DEVICE;

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_register (void *pdevice, FF_INIT *pinit, FF_READ *pread, FF_WRITE *pwrite, uint32_t psector_size, BYTE pdrv);
FF_DEVICE *disk_search (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DSTATUS disk_initialize (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

#endif /* FS_FAT */
#endif /* CONFIG_FS */

#ifdef __cplusplus
}
#endif

#endif
