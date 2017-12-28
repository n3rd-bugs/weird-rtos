/*
 * fs.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef FS_H
#define FS_H

#include <kernel.h>

#ifdef CONFIG_FS
#include <stdarg.h>
#include <semaphore.h>
#include <condition.h>

/* File system configuration. */
#ifndef CMAKE_BUILD
#define FS_CONSOLE
//#define FS_PIPE
//#define FS_FAT
#endif /* CMAKE_BUILD */

/* Error definitions. */
#define FS_NODE_DELETED         -800
#define FS_BUFFER_NO_SPACE      -801
#define FS_INVALID_BUFFER_TYPE  -802
#define FS_INVALID_COMMAND      -803

/* File descriptor definitions. */
typedef void *FD;

/* Include file system buffer definitions. */
#include <fs_buffer.h>

/* File system configurations. */
#define FS_PRINTF_BUFFER_SIZE   64

/* File system specific flags. */
#define FS_DATA_AVAILABLE   0x0001
#define FS_SPACE_AVAILABLE  0x0002
#define FS_BLOCK            0x0100
#define FS_BUFFERED         0x0200
#define FS_FLUSH_WRITE      0x0400
#define FS_WRITE_NO_BLOCK   0x0800

/* Suspend flags. */
#define FS_BLOCK_READ       0x00000001
#define FS_BLOCK_WRITE      0x00000002

/* File open flags. */
#define FS_READ             0x01
#define FS_WRITE            0x02
#define FS_CREATE           0x04
#define FS_APPEND           0x08

/* File system descriptor. */
typedef struct _fs FS;
struct _fs
{
    /* List next. */
    FS          *next;

    /* Parent node definition. */
    const char  *name;

    /* File operations. */
    void        *(*open) (void *, char *, uint32_t);
    void        (*close) (void **);
    int32_t     (*write) (void *, const uint8_t *, int32_t);
    int32_t     (*read) (void *, uint8_t *, int32_t);
    int32_t     (*ioctl) (void *, uint32_t, void *);

    /* Driver operations. */
    int32_t     (*get_lock) (void *, uint32_t);
    void        (*release_lock) (void *);

    /* Condition data for a file system. */
    CONDITION   condition;

    /* File system buffer data. */
    FS_BUFFER_DATA  *buffer;

    /* This will hold the timeout if blocking mode is used. */
    uint32_t    timeout;

    /* File system specific flags. */
    uint16_t    flags;

    /* File system priority, will be used as suspend priority. */
    uint8_t     priority;

    /* Structure padding. */
    uint8_t     pad[1];
};

/* This holds the resumption criteria for a task waiting on a FS. */
typedef struct _fs_param
{
    uint32_t    flag;
} FS_PARAM;

/* File system list. */
typedef struct _fs_data
{
    struct _fs_list
    {
        /* Link-list for the registered file systems. */
        FS          *head;
        FS          *tail;
    } list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock. */
    SEMAPHORE   lock;
#endif

} FS_DATA;

/* Internal data structure. */
typedef struct _file_desc
{
    /* Private data. */
    void *priv;
} FILE_DESC;

/* Search parameter for fs_sreach_filesystem. */
typedef struct _dir_param
{
    char        *name;
    char        *matched;
    void        *priv;
} DIR_PARAM;

/* Search parameter for fs_sreach_filesystem. */
typedef struct _node_param
{
    char        *name;
    void        *priv;
} NODE_PARAM;

#ifdef FS_FAT
#include <fat_fs.h>
#endif

/* Function prototypes. */
void fs_init(void);

/* File APIs. */
FD fs_open(char *, uint32_t);
void fs_close(FD *);

int32_t fd_get_lock(FD);
int32_t fd_try_get_lock(FD, uint32_t);
void fd_release_lock(FD);

int32_t fs_read(FD, uint8_t *, int32_t);
int32_t fs_write(FD, const uint8_t *, int32_t);
int32_t fs_ioctl(FD, uint32_t, void *);
int32_t fs_printf(FD, char *, ...);
int32_t fs_vprintf(FD, const char *, va_list);
int32_t fs_puts(FD, const uint8_t *, int32_t);

void fs_condition_init(FD);
void fs_condition_get(FD, CONDITION **, SUSPEND *, FS_PARAM *, uint32_t);
void fs_condition_lock(void *);
void fs_condition_unlock(void *);
void fd_handle_criteria(void *, FS_PARAM *, int32_t);

/* File system functions. */
void fs_register(FS *);
void fs_unregister(FS *);
void fd_data_available(void *);
void fd_data_flushed(void *);
void fd_space_available(void *);
void fd_space_consumed(void *);

/* Helper APIs. */
uint8_t fs_sreach_directory(void *, void *);
uint8_t fs_sreach_node(void *, void *);
void fs_memcpy_r(void *, void *, uint32_t);

#endif /* CONFIG_FS */

#endif /* FS_H */
