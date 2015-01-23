/*
 * fs.h
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
#ifndef FS_H
#define FS_H

#include <os.h>

#ifdef CONFIG_SEMAPHORE
#include <semaphore.h>
#endif

#ifdef CONFIG_FS

/* File system configuration. */
#define FS_PIPE
#define FS_CONSOLE

/* File descriptor definitions. */
typedef void *FD;

/* File system specific flags. */
#define FS_BLOCK            0x0001000
#define FS_PRIORITY_SORT    0x0002000
#define FS_DATA_AVAILABLE   0x0000001

/* File system descriptor. */
typedef struct _fs FS;
struct _fs
{
    /* List next. */
    FS          *next;

    /* Parent node definition. */
    char        *name;

    /* File operations. */
    void        *(*open) (char *, uint32_t);
    void        (*close) (void **);
    uint32_t    (*write) (void *, char *, uint32_t);
    uint32_t    (*read) (void *, char *, uint32_t);
    uint32_t    (*ioctl) (void *, uint32_t, void *);

    /* Driver operations. */
    void        (*get_lock) (void *);
    void        (*release_lock) (void *);
    uint32_t    (*should_resume) (void *, void *, void *);

    /* File system specific flags. */
    uint32_t    flags;

    /* This will hold the timeout if blocking mode is used. */
    uint32_t    timeout;

    struct _fs_task_list
    {
        /* Link-list for the tasks waiting for data. */
        TASK        *head;
        TASK        *tail;
    } task_list;
};

/* This holds the resumption criteria for a task waiting on an FS. */
/* This parameter structure should be at head of the parameter passed in when
 * data is available. */
typedef struct _fs_param
{
    void        *fs;
    void        *param;
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

/* Function prototypes. */
void fs_init();

/* File APIs. */
FD fs_open(char *, uint32_t);
void fs_close(FD *);

uint32_t fs_read(FD, char *, uint32_t);
uint32_t fs_write(FD, char *, uint32_t);
uint32_t fs_ioctl(FD, uint32_t, void *);

/* File system functions. */
void fs_register(FS *file_system);
void fd_data_available(void *fs, FS_PARAM *param);
void fd_data_flushed(void *fs);

/* Helper APIs. */
uint8_t fs_sreach_directory(void *node, void *param);
uint8_t fs_sreach_node(void *node, void *param);

/* Include sub modules. */
#ifdef FS_PIPE
#include <pipe.h>
#endif

#ifdef FS_CONSOLE
#include <console.h>
#endif

#endif /* CONFIG_FS */

#endif /* FS_H */
