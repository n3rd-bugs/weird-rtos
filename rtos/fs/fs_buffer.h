/*
 * fs_buffer.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef FS_BUFFER_H
#define FS_BUFFER_H
#include <kernel.h>

#ifdef CONFIG_FS

/* Buffer configuration. */
#ifndef CMAKE_BUILD
//#define FS_BUFFER_DEBUG
#endif /* CMAKE_BUILD */

/* Buffer type definition. */
#define FS_BUFFER_ONE_FREE  1
#define FS_BUFFER_LIST      2
#define FS_BUFFER_RX        3
#define FS_BUFFER_TX        4

/* Buffer management flags. */
#define FS_BUFFER_ACTIVE    0x01
#define FS_BUFFER_INPLACE   0x02
#define FS_BUFFER_UPDATE    FS_BUFFER_INPLACE
#define FS_BUFFER_PACKED    0x04
#define FS_BUFFER_TAIL      0x08
#define FS_BUFFER_HEAD      0x10
#define FS_BUFFER_SUSPEND   0x20
#define FS_BUFFER_TH        0x40

/* Week definition of a buffer structure. */
typedef struct _fs_buffer FS_BUFFER;

/* Call back definition to submit a buffer back to the caller. */
typedef uint8_t FS_RETURN_BUFFER (void *, FS_BUFFER *);

/* File system one buffer structure. */
typedef struct _fs_buffer_one FS_BUFFER_ONE;
struct _fs_buffer_one
{
    /* Buffer list member. */
    FS_BUFFER_ONE   *next;

    /* Actual buffer data. */
    uint8_t     *data;
    uint32_t    max_length;

    /* Buffer data. */
    uint8_t     *buffer;
    uint32_t    length;
};

/* File system buffer structure. */
struct _fs_buffer
{
    /* Buffer list member. */
    FS_BUFFER   *next;

    /* List of buffers in this chain. */
    struct _fs_buffer_list
    {
        FS_BUFFER_ONE   *head;
        FS_BUFFER_ONE   *tail;
    } list;

    /* Total length of buffers. */
    uint32_t    total_length;

    /* File descriptor from which this chain was allocated. */
    FD          fd;

    /* If set this function will be called to submit this buffer back to
     * the user. */
    FS_RETURN_BUFFER    *free;

    /* Private data to be passed to the user. */
    void        *free_data;

};

/* File system buffer data, need by a buffered file descriptor. */
typedef struct _fs_buffer_data
{
    /* Free buffer list. */
    struct _fs_free_buffer_list
    {
        FS_BUFFER_ONE   *head;
        FS_BUFFER_ONE   *tail;
        uint32_t        buffers;
    } free_buffer_list;

    /* Transmit buffer list. */
    struct _fs_tx_buffer_list
    {
        FS_BUFFER   *head;
        FS_BUFFER   *tail;
#ifdef FS_BUFFER_DEBUG
        uint32_t        buffers;
#endif
    } tx_buffer_list;

    /* Receive buffer list. */
    struct _fs_rx_buffer_list
    {
        FS_BUFFER   *head;
        FS_BUFFER   *tail;
#ifdef FS_BUFFER_DEBUG
        uint32_t        buffers;
#endif
    } rx_buffer_list;

    /* Buffer lists list. */
    struct _fs_buffers_list
    {
        FS_BUFFER   *head;
        FS_BUFFER   *tail;
        uint32_t    buffers;
    } buffers_list;

    /* Condition structure for this buffer data. */
    CONDITION       condition;

    /* File system buffer data. */
    uint8_t         *buffer_space;
    FS_BUFFER_ONE   *buffer_ones;
    FS_BUFFER       *buffer_lists;

    /* Threshold buffer configuration. */
    uint32_t        threshold_buffers;
    uint32_t        threshold_lists;

    /* Buffer data. */
    uint32_t        buffer_size;
    uint32_t        num_buffer_ones;
    uint32_t        num_buffer_lists;

} FS_BUFFER_DATA;

/* This holds the resumption criteria for a task waiting on a file system
 * buffer. */
typedef struct _fs_buffer_param
{
    /* Number of buffers or which we are waiting. */
    uint32_t    num_buffers;

    /* Type of buffer for which we are waiting. */
    uint32_t    type;

} FS_BUFFER_PARAM;

/* One buffer helper macros. */
#define FS_BUFFER_RESET(b)          fs_buffer_one_init((b), (b)->data, (b)->max_length)
#define FS_BUFFER_LEN(b)            (((b)->next == NULL) ? (b)->length : ((FS_BUFFER *)(b))->length)
#define FS_BUFFER_SPACE(b)          ((b)->max_length - (b)->length)
#define FS_BUFFER_HEAD_ROOM(b)      ((uint32_t)((b)->buffer - (b)->data))
#define FS_BUFFER_TAIL_ROOM(b)      (FS_BUFFER_SPACE(b) - FS_BUFFER_HEAD_ROOM(b))
#define fs_buffer_one_get(fd, flag) (FS_BUFFER_ONE *)fs_buffer_get((fd), FS_BUFFER_ONE_FREE, (flag))

/* File system buffer management APIs. */
void fs_buffer_dataset(FD, FS_BUFFER_DATA *);
void fs_buffer_init(FS_BUFFER *, FD);
void fs_buffer_one_init(FS_BUFFER_ONE *, void *, uint32_t);
void fs_buffer_one_update(FS_BUFFER_ONE *, void *, uint32_t);
void fs_buffer_move(FS_BUFFER *, FS_BUFFER *);
void fs_buffer_move_data(FS_BUFFER *, FS_BUFFER *, uint8_t);
int32_t fs_buffer_num_remaining(FD, uint32_t);
void fs_buffer_condition_init(FD);
void fs_buffer_condition_get(FD, CONDITION **, SUSPEND *, FS_BUFFER_PARAM *, uint32_t, uint32_t);
uint8_t fs_buffer_threshold_locked(FD);
void fs_buffer_add_one(FS_BUFFER *, FS_BUFFER_ONE *, uint8_t);
void fs_buffer_add_list(FS_BUFFER *, uint32_t, uint32_t);
void fs_buffer_add_buffer_list(FS_BUFFER *, uint32_t, uint32_t);
void fs_buffer_add(FD, void *, uint32_t, uint32_t);
FS_BUFFER *fs_buffer_get(FD, uint32_t, uint32_t);


/* File system buffer manipulation APIs. */
#define fs_buffer_pull(b, d, l, f)      fs_buffer_pull_offset((b), (d), (l), 0, (f))
int32_t fs_buffer_pull_offset(FS_BUFFER *, void *, uint32_t, uint32_t, uint8_t);
#define fs_buffer_push(b, d, l, f)      fs_buffer_push_offset((b), (d), (l), 0, (f))
int32_t fs_buffer_push_offset(FS_BUFFER *, void *, uint32_t, uint8_t, uint8_t);
int32_t fs_buffer_divide(FS_BUFFER *, uint32_t, uint32_t);

/* File system one buffer manipulation APIs. */
int32_t fs_buffer_one_add_head(FS_BUFFER_ONE *, uint32_t);
#define fs_buffer_one_pull(o, d, l, f)  fs_buffer_one_pull_offset((o), (d), (l), 0, (f))
int32_t fs_buffer_one_pull_offset(FS_BUFFER_ONE *, void *, uint32_t, uint32_t, uint8_t);
#define fs_buffer_one_push(o, d, l, f)  fs_buffer_one_push_offset((o), (d), (l), 0, (f))
int32_t fs_buffer_one_push_offset(FS_BUFFER_ONE *, void *, uint32_t, uint32_t, uint8_t);
int32_t fs_buffer_one_divide(FD, FS_BUFFER_ONE *, FS_BUFFER_ONE **, uint32_t, uint32_t);

/* Helper routines. */
int32_t fs_buffer_hdr_pull(void *, uint8_t *, uint32_t, uint16_t);
int32_t fs_buffer_hdr_push(void *, uint8_t *, uint32_t, uint16_t);

#endif /* CONFIG_FS */
#endif /* FS_BUFFER_H */
