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

#ifdef CONFIG_FS
#include <semaphore.h>
#include <condition.h>

/* File system configuration. */
#define FS_PIPE
#define FS_CONSOLE
#define FS_FAT

/* Error definitions. */
#define FS_NODE_DELETED         -800
#define FS_BUFFER_NO_SPACE      -801
#define FS_INVALID_BUFFER_TYPE  -802
#define FS_INVALID_COMMAND      -803

/* File descriptor definitions. */
typedef void *FD;

/* Include file system buffer definitions. */
#include <fs_buffer.h>

/* File system specific flags. */
#define FS_DATA_AVAILABLE   0x00000001
#define FS_SPACE_AVAILABLE  0x00000002
#define FS_CHAIN_HEAD       0x00000004
#define FS_BLOCK            0x00010000
#define FS_BUFFERED         0x00020000
#define FS_FLUSH_WRITE      0x00040000

/* Suspend flags. */
#define FS_BLOCK_READ       0x00000001
#define FS_BLOCK_WRITE      0x00000002

/* File open flags. */
#define FS_READ             0x01
#define FS_WRITE            0x02
#define FS_CREATE           0x04
#define FS_APPEND           0x08

/* Connection watcher data. */
typedef struct _fs_connection_watcher FS_CONNECTION_WATCHER;
struct _fs_connection_watcher
{
    /* List next. */
    FS_CONNECTION_WATCHER *next;

    /* Watcher data. */
    void        (*connected) (void *, void *);
    void        (*disconnected) (void *, void *);
    void        *data;
};

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
    int32_t     (*write) (void *, uint8_t *, int32_t);
    int32_t     (*read) (void *, uint8_t *, int32_t);
    int32_t     (*ioctl) (void *, uint32_t, void *);

    /* Driver operations. */
    int32_t     (*get_lock) (void *, uint32_t);
    void        (*release_lock) (void *);

    /* Condition data for a file system. */
    CONDITION   condition;

    /* Connection watcher hooks. */
    struct _fs_connection_watcher_list
    {
        FS_CONNECTION_WATCHER   *head;
        FS_CONNECTION_WATCHER   *tail;
    } connection_watcher_list;

    union _fs_fd_chain
    {
        struct _fs_fd_list
        {
            /* Link-list for connected file systems. */
            FD          *head;
            FD          *tail;
        } fd_list;

        struct _fs_fd_node
        {
            /* Next file descriptor. */
            FD          *next;

            /* File descriptor that will act as list head. */
            FD          *head;
        } fd_node;
    } fd_chain;

    /* File system buffer data. */
    FS_BUFFER_DATA  *buffer;

    /* This will hold the timeout if blocking mode is used. */
    uint32_t    timeout;

    /* File system specific flags. */
    uint32_t    flags;
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
void fs_init();

/* File APIs. */
FD fs_open(char *, uint32_t);
void fs_close(FD *);

int32_t fd_get_lock(FD);
int32_t fd_try_get_lock(FD, uint32_t);
void fd_release_lock(FD);

int32_t fs_read(FD, uint8_t *, int32_t);
int32_t fs_write(FD, uint8_t *, int32_t);
int32_t fs_ioctl(FD, uint32_t, void *);

void fs_connection_watcher_set(FD, FS_CONNECTION_WATCHER *);
void fs_connected(FD);
void fs_disconnected(FD);
void fs_connect(FD, FD);
void fs_destroy_chain(FD);
void fs_disconnect(FD);

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
