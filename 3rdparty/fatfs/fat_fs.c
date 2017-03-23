/*
 * fat_fs.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <fat_fs.h>

#ifdef FS_FAT
#ifdef CONFIG_SEMAPHORE
#include <semaphore.h>
#endif

/* Internal function prototypes. */
static void *fatfs_open(void *, char *, uint32_t);
static int32_t fatfs_read(void *, uint8_t *, int32_t);
static int32_t fatfs_write(void *, uint8_t *, int32_t);
static void fatfs_close(void **priv_data);

/* Global file list. */
static FAT_FILE fat_files[FAT_NUM_FILES];
static FS fat_fs =
    {
        /* Name of this interface. */
        .name = "fatfs",

        /* Hook-up file system. */
        .open = &fatfs_open,
    };

#ifdef CONFIG_SEMAPHORE
static SEMAPHORE fat_lock;
#endif

/*
 * fatfs_init
 * This function will initialize FAT file system.
 */
void fatfs_init()
{
    uint32_t i;

    SYS_LOG_FUNTION_ENTRY(FATFS);

#ifdef CONFIG_SEMAPHORE
    /* Create lock to protect FAT file system layer. */
    semaphore_create(&fat_lock, 1, 1, 0);
#endif

    /* Initialize file system for all the files. */
    for (i = 0; i < FAT_NUM_FILES; i++)
    {
        /* Hook up file system for each file. */
        fat_files[i].fs.read = &fatfs_read;
        fat_files[i].fs.write = &fatfs_write;
        fat_files[i].fs.close = &fatfs_close;

        /* Space and data is always available. */
        fat_files[i].fs.flags |= (FS_SPACE_AVAILABLE | FS_DATA_AVAILABLE);
    }

    /* Register FAT file system. */
    fs_register(&fat_fs);

    SYS_LOG_FUNTION_EXIT(FATFS);

} /* fatfs_init */

/*
 * fatfs_open
 * @priv_data: Private data.
 * @name: File name.
 * @flags: Open flags.
 * This function will open a file.
 */
static void *fatfs_open(void *priv_data, char *name, uint32_t flags)
{
    uint32_t i;
    FAT_FILE *fd = NULL;
    int32_t status;
    uint32_t fs_flags = 0;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);

    SYS_LOG_FUNTION_ENTRY(FATFS);

#ifdef CONFIG_SEMAPHORE
    /* Lock the FAT layer. */
    if (semaphore_obtain(&fat_lock, MAX_WAIT) == SUCCESS)
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif
    {
        /* Find a free file entry. */
        for (i = 0; i < FAT_NUM_FILES; i++)
        {
            /* If this file entry is not opened. */
            if ((fat_files[i].flags & FAT_FILE_OPEN) == 0)
            {
                /* Use this file entry. */
                fd = &fat_files[i];

                break;
            }
        }

        /* If we did find a file entry. */
        if (fd != NULL)
        {
            /* Mark this entry as open. */
            fd->flags |= FAT_FILE_OPEN;
        }

#ifdef CONFIG_SEMAPHORE
        /* Unlock the FAT layer. */
        semaphore_release(&fat_lock);
#else
        /* Unlock the scheduler. */
        scheduler_unlock();
#endif
    }

    /* If we did find a file entry. */
    if (fd != NULL)
    {
        SYS_LOG_FUNTION_MSG(FATFS, SYS_LOG_DEBUG, "opening %s", name);

        /* If read is requested. */
        if (flags & FS_READ)
        {
            /* Set read flag. */
            fs_flags |= FA_READ;
        }

        /* If write was requested. */
        if (flags & FS_WRITE)
        {
            /* Set write flag. */
            fs_flags |= FA_WRITE;
        }

        /* If create was requested. */
        if (flags & FS_CREATE)
        {
            /* Set create flag. */
            fs_flags |= FA_CREATE_ALWAYS;
        }

        /* If append was requested. */
        if (flags & FS_APPEND)
        {
            /* Set append flag. */
            fs_flags |= FA_OPEN_APPEND;
        }

        /* Transfer open to the FAT file system. */
        if ((status = f_open(&fd->file, name, fs_flags)) != FR_OK)
        {
            SYS_LOG_FUNTION_MSG(FATFS, SYS_LOG_INFO, "f_open returned %d", status);

#ifdef CONFIG_SEMAPHORE
            /* Lock the FAT layer. */
            if (semaphore_obtain(&fat_lock, MAX_WAIT) == SUCCESS)
#else
            /* Lock the scheduler. */
            scheduler_lock();
#endif
            {
                /* Mark this entry as free. */
                fd->flags &= (uint32_t)(~FAT_FILE_OPEN);

                /* Return null descriptor. */
                fd = NULL;

#ifdef CONFIG_SEMAPHORE
                /* Unlock the FAT layer. */
                semaphore_release(&fat_lock);
#else
                /* Unlock the scheduler. */
                scheduler_unlock();
#endif
            }
        }
    }

    SYS_LOG_FUNTION_EXIT(FATFS);

    /* Return the resolved file descriptor. */
    return (fd);

} /* fatfs_open */

/*
 * fatfs_read
 * @priv_data: FAT file descriptor.
 * @buf: Buffer in which data is needed to be read.
 * @nbytes: Number of bytes to be read.
 * @return: Number of bytes will be returned if read was successful.
 * This function will read data from a file.
 */
static int32_t fatfs_read(void *priv_data, uint8_t *buf, int32_t nbytes)
{
    FAT_FILE *fd = (FAT_FILE *)priv_data;
    UINT bytes;
    int32_t status;

    SYS_LOG_FUNTION_ENTRY(FATFS);

    /* Transfer read to the file system. */
    if ((status = f_read(&fd->file, buf, nbytes, (UINT *)&bytes)) != FR_OK)
    {
        /* Nothing was read from the file system. */
        bytes = 0;

        SYS_LOG_FUNTION_MSG(FATFS, SYS_LOG_INFO, "f_read returned %d", status);
    }

    SYS_LOG_FUNTION_HEXDUMP(FATFS, SYS_LOG_DEBUG, "read ", buf, nbytes);

    SYS_LOG_FUNTION_EXIT(FATFS);

    /* Return number of bytes read. */
    return ((int32_t)bytes);

} /* fatfs_read */

/*
 * fatfs_write
 * @priv_data: FAT file descriptor.
 * @buf: Data to be written.
 * @nbytes: Number of bytes to be written.
 * @return: Number of bytes will be returned if write was successful.
 * This function will write data in a file.
 */
static int32_t fatfs_write(void *priv_data, uint8_t *buf, int32_t nbytes)
{
    FAT_FILE *fd = (FAT_FILE *)priv_data;
    UINT bytes;
    int32_t status;

    SYS_LOG_FUNTION_ENTRY(FATFS);

    SYS_LOG_FUNTION_HEXDUMP(FATFS, SYS_LOG_DEBUG, "writing ", buf, nbytes);

    /* Transfer write to the file system. */
    if ((status = f_write(&fd->file, buf, nbytes, (UINT *)&bytes)) != FR_OK)
    {
        /* Nothing was read from the file system. */
        bytes = 0;

        SYS_LOG_FUNTION_MSG(FATFS, SYS_LOG_INFO, "f_write returned %d", status);
    }

    SYS_LOG_FUNTION_EXIT(FATFS);

    /* Return number of bytes read. */
    return ((int32_t)bytes);

} /* fatfs_write */

/*
 * fatfs_close
 * @priv_data: Pointer to FAT file descriptor.
 * This function will close an already opened file.
 */
static void fatfs_close(void **priv_data)
{
    FAT_FILE **fd = (FAT_FILE **)priv_data;
    int32_t status;

    SYS_LOG_FUNTION_ENTRY(FATFS);

    /* Close the required file. */
    if ((status = f_close(&(*fd)->file)) != FR_OK)
    {
        SYS_LOG_FUNTION_MSG(FATFS, SYS_LOG_INFO, "f_close returned %d", status);
    }

#ifdef CONFIG_SEMAPHORE
    /* Lock the FAT layer. */
    if (semaphore_obtain(&fat_lock, MAX_WAIT) == SUCCESS)
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif
    {
        /* Mark this entry as free. */
        (*fd)->flags &= (uint32_t)(~FAT_FILE_OPEN);

#ifdef CONFIG_SEMAPHORE
        /* Unlock the FAT layer. */
        semaphore_release(&fat_lock);
#else
        /* Unlock the scheduler. */
        scheduler_unlock();
#endif
    }

    SYS_LOG_FUNTION_EXIT(FATFS);

} /* fatfs_close */

#endif /* FS_FAT */
