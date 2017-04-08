/*
 * fs_buffer.c
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

#ifdef CONFIG_FS
#include <string.h>
#include <sll.h>
#include <fs.h>
#include <header.h>

#ifdef CONFIG_NET
#include "net_condition.h"
#endif

/* Internal function prototypes. */
static uint8_t fs_buffer_do_suspend(void *, void *);
static uint8_t fs_buffer_do_resume(void *, void *);
static int32_t fs_buffer_suspend(FD, uint32_t, uint32_t);

/*
 * fs_buffer_dataset
 * @fd: File descriptor for which buffer data-set is needed to be set.
 * @data: Pointer to buffer data structure.
 * @num_buffers: Total number of buffers in the system.
 * This function will set the buffer structure to be used by this file
 * descriptor also sets the flag to tell others that this will be a buffered
 * file descriptor.
 */
void fs_buffer_dataset(FD fd, FS_BUFFER_DATA *data, int32_t num_buffers)
{
    FS *fs = (FS *)fd;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

    /* Get lock for this file descriptor. */
    OS_ASSERT(fd_get_lock(fd) != SUCCESS);

    /* This is a buffer file system. */
    fs->flags |= FS_BUFFERED;

    /* Set the buffer data structure provided by caller. */
    fs->buffer = data;

    /* Clear the structure. */
    memset(data, 0, sizeof(FS_BUFFER_DATA));

#ifdef FS_BUFFER_DEBUG
    data->buffers = num_buffers;
#else
    UNUSED_PARAM(num_buffers);
#endif

    /* Initialize buffer condition data. */
    fs_buffer_condition_init(fs);

    /* Release lock for this file descriptor. */
    fd_release_lock(fd);

} /* fs_buffer_dataset */

/*
 * fs_buffer_init
 * @buffer: File buffer needed to be initialized.
 * @fd: File descriptor for which this buffer will be initialized.
 * This function will initialize a buffer structure.
 */
void fs_buffer_init(FS_BUFFER *buffer, FD fd)
{
    /* Clear this buffer. */
    memset(buffer, 0, sizeof(FS_BUFFER));

    /* Initialize this buffer. */
    buffer->id = FS_BUFFER_ID_BUFFER;
    buffer->fd = fd;

} /* fs_buffer_init */

/*
 * fs_buffer_one_init
 * @buffer: File buffer needed to be initialized.
 * @data: Data space needed to be used for this buffer.
 * @size: Size of the data allocated for this buffer.
 * This function will initialize a one buffer with given data.
 */
void fs_buffer_one_init(FS_BUFFER_ONE *one, void *data, uint32_t size)
{
    /* Initialize this buffer. */
    one->id = FS_BUFFER_ID_ONE;
    one->data = one->buffer = (uint8_t *)data;
    one->max_length = size;

    /* Clear the remaining members. */
    one->length = 0;
    one->next = NULL;

} /* fs_buffer_one_init */

/*
 * fs_buffer_one_update
 * @buffer: File buffer needed to be updated.
 * @data: New buffer pointer.
 * @size: Size of valid data in the buffer.
 * This function will update a buffer data pointers.
 */
void fs_buffer_one_update(FS_BUFFER_ONE *one, void *data, uint32_t size)
{
    /* Update the buffer data. */
    one->buffer = (uint8_t *)data;
    one->length = size;

} /* fs_buffer_one_update */

/*
 * fs_buffer_move
 * @src_buffer: Buffer needed to be moved.
 * @dst_buffer: Buffer in which we need to make a copy.
 * This function will move data of one buffer to another buffer.
 */
void fs_buffer_move(FS_BUFFER *dst_buffer, FS_BUFFER *src_buffer)
{
    /* Save the destination buffer file descriptor. */
    FD buffer_fd = dst_buffer->fd;

    /* Copy the buffer data as it is. */
    memcpy(dst_buffer, src_buffer, sizeof(FS_BUFFER));

    /* Restore the file descriptor for destination buffer. */
    dst_buffer->fd = buffer_fd;

    /* Reset the source buffer. */
    fs_buffer_init(src_buffer, src_buffer->fd);

} /* fs_buffer_move */

/*
 * fs_buffer_move_data
 * @dst: Buffer to which data will be moved.
 * @src: Buffer from which data will be moved.
 * @flags: Operation flags.
 *  FS_BUFFER_HEAD: If we need to add data on the head of existing data.
 * This function will move all the data from one buffer to the other buffer.
 */
void fs_buffer_move_data(FS_BUFFER *dst, FS_BUFFER *src, uint8_t flags)
{
    /* Move all the data from the source buffer to the destination buffer. */

    /* If data is needed to be added at the start of existing data. */
    if (flags & FS_BUFFER_HEAD)
    {
        /* Add data at the end of the existing data. */
        src->list.tail->next = dst->list.head;
        dst->list.head = src->list.head;
    }
    else
    {
        /* Add data at the end of the existing data. */
        dst->list.tail->next = src->list.head;
        dst->list.tail = src->list.tail;
    }

    /* Increment number of bytes added. */
    dst->total_length += src->total_length;

    /* Clear the list for this buffer. */
    src->list.head = src->list.tail = NULL;
    src->total_length = 0;

} /* fs_buffer_move_data */

/*
 * fs_buffer_num_remaining
 * @fd: File descriptor on which number of buffers in a list is required.
 * @type: Type of buffer needed to be checked.
 *  FS_BUFFER_ONE_FREE: If this is a free buffer.
 *  FS_BUFFER_LIST: If this is a free buffer list.
 * @return: If >= zero the number of buffers remaining in the list will be
 *  returned, FS_INVALID_BUFFER_TYPE will be returned if an invalid buffer type
 *  was given.
 * This function will return the number of buffers remaining in a given type of
 * buffer list.
 */
int32_t fs_buffer_num_remaining(FD fd, uint32_t type)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    int32_t ret_num;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

    /* Type of buffer we need to check. */
    switch (type)
    {
    /* A free buffer. */
    case FS_BUFFER_ONE_FREE:

        /* Return number of buffers remaining in the free one buffer list. */
        ret_num = data->free_buffer_list.buffers;

        break;

    /* A free buffer list. */
    case FS_BUFFER_LIST:

        /* Return number of buffers remaining in the free buffer list. */
        ret_num = data->buffers_list.buffers;

        break;

    /* Unknown buffer type. */
    default:

        /* Unknown buffer type. */
        ret_num = FS_INVALID_BUFFER_TYPE;

        break;
    }

    /* Return number of buffers are remaining for the given type. */
    return (ret_num);

} /* fs_buffer_num_remaining */

/*
 * fs_buffer_condition_init
 * @fd: File descriptor for which condition is needed to be initialized.
 * This function will initialize condition structure for a file descriptor.
 */
void fs_buffer_condition_init(FD fd)
{
    FS *fs = (FS *)fd;

   /* Clear the condition structure. */
    memset(&fs->buffer->condition, 0, sizeof(CONDITION));

    /* Initialize condition for this file descriptor buffer. */
    fs->buffer->condition.data = fd;
    fs->buffer->condition.lock = &fs_condition_lock;
    fs->buffer->condition.unlock = &fs_condition_unlock;

} /* fs_buffer_condition_init */

/*
 * fs_buffer_do_suspend
 * @data: Condition data that will be passed to check for a file system buffer.
 * @suspend_data: Suspend data that will hold why we were suspended.
 * This function will called to see if we do need to suspend on the condition.
 */
static uint8_t fs_buffer_do_suspend(void *data, void *suspend_data)
{
    FS *fs = (FS *)data;
    FS_BUFFER_PARAM *param = (FS_BUFFER_PARAM *)suspend_data;
    uint8_t do_suspend = TRUE;

    /* Check if the condition for which we were waiting is now met. */
    switch (param->type)
    {

    /* If we are suspending on free one buffers. */
    case FS_BUFFER_ONE_FREE:

        /* Check if we have required number of buffers. */
        if (fs->buffer->free_buffer_list.buffers > param->num_buffers)
        {
            /* Don't need to suspend. */
            do_suspend = FALSE;
        }

        break;

    /* If we are suspending on free buffer lists. */
    case FS_BUFFER_LIST:

        /* Check if we have required number of buffers. */
        if (fs->buffer->buffers_list.buffers > param->num_buffers)
        {
            /* Don't need to suspend. */
            do_suspend = FALSE;
        }

        break;
    }

    /* Return if we need to suspend or not. */
    return (do_suspend);

} /* fs_buffer_do_suspend */

/*
 * fs_buffer_do_resume
 * @param_resume: Parameter for which we need to resume a task.
 * @param_suspend: Parameter for which a task was suspended.
 * @return: TRUE if we need to resume this task, FALSE if we cannot resume
 *  this task.
 * This is callback to see if we can resume a task suspended on a file
 * system buffer for a given condition.
 */
static uint8_t fs_buffer_do_resume(void *param_resume, void *param_suspend)
{
    FS_BUFFER_PARAM *fs_resume = (FS_BUFFER_PARAM *)param_resume;
    FS_BUFFER_PARAM *fs_suspend = (FS_BUFFER_PARAM *)param_suspend;
    uint8_t resume = FALSE;

    /* Check if we have the number of buffers we were waiting for. */
    if (fs_resume->num_buffers > fs_suspend->num_buffers)
    {
        /* Resume this task. */
        resume = TRUE;
    }

    /* Return if we can resume this task. */
    return (resume);

} /* fs_buffer_do_resume */

/*
 * fs_buffer_condition_get
 * @fd: File descriptor for which buffer condition is needed.
 * @condition: Pointer where condition for this file descriptor will be
 *  returned.
 * @suspend: Suspend structure that will be populated for the required
 *  condition.
 * @param: File system buffer parameter that will used to suspend on this
 *  buffer.
 * @num_buffers: Number of buffers we will wait.
 * @type: Type of buffer we will wait.
 * This function will return condition for this file system, and also will also
 * populate the suspend.
 */
void fs_buffer_condition_get(FD fd, CONDITION **condition, SUSPEND *suspend, FS_BUFFER_PARAM *param, int32_t num_buffers, uint32_t type)
{
    FS *fs = (FS *)fd;
    FS_BUFFER_DATA *data = fs->buffer;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

    /* Initialize buffer suspend parameter. */
    param->num_buffers = num_buffers;
    param->type = type;

    /* Initialize file system parameter. */
    suspend->param = param;
    suspend->do_suspend = &fs_buffer_do_suspend;
    suspend->timeout_enabled = FALSE;

    /* Return the condition for this file system buffer. */
    *condition = &(data->condition);

} /* fs_buffer_condition_get */

/*
 * fs_buffer_suspend
 * @fd: File descriptor on which we need to suspend for a buffer.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_ONE_FREE: If a free buffer is needed.
 *  FS_BUFFER_LIST: If a list buffer is needed.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * This function will suspend on a buffer condition.
 */
static int32_t fs_buffer_suspend(FD fd, uint32_t type, uint32_t flags)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    SUSPEND buffer_suspend, *suspend_ptr = &buffer_suspend;
    CONDITION *condition;
    FS_BUFFER_PARAM param;
    int32_t status;

#ifdef CONFIG_NET
    /* We should not be in the networking condition task. */
    OS_ASSERT(get_current_task() == &net_condition_tcb);
#endif

    /* Get buffer condition. */
    fs_buffer_condition_get(fd, &condition, suspend_ptr, &param, ((flags & FS_BUFFER_TH) ? ((type == FS_BUFFER_ONE_FREE) ? data->threshold_buffers : data->threshold_lists) : 0), type);

    /* We are already in the locked state. */
    status = suspend_condition(&condition, &suspend_ptr, NULL, TRUE);

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_suspend */

/*
 * fs_buffer_threshold_locked
 * @fd: File descriptor for which we need to check if buffer threshold has been
 *  achieved.
 * @return: Returns true if file descriptor threshold is achieved and we must
 *  not feed more buffers to the application, otherwise false will be returned.
 * This function will tell the threshold buffer status and will be called to
 * see if we can push data to application without causing a complete buffer
 * starvation.
 */
uint8_t fs_buffer_threshold_locked(FD fd)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    uint8_t locked = FALSE;

    /* Check if we have enough free buffers. */
    if ((data->free_buffer_list.buffers <= data->threshold_buffers) || (data->buffers_list.buffers <= data->threshold_lists))
    {
        /* We have reached the buffer threshold. */
        locked = TRUE;
    }

    /* Return if threshold is locked or not. */
    return (locked);

} /* fs_buffer_threshold_locked */

/*
 * fs_buffer_add_one
 * @buffer: Buffer to which a new buffer is needed to be added.
 * @one: One buffer needed to be added.
 * @flags: Defines where the given buffer is needed to be added.
 *  FS_BUFFER_HEAD: If we need to add this one buffer on the head.
 * This function will add a given one buffer to the given buffer.
 */
void fs_buffer_add_one(FS_BUFFER *buffer, FS_BUFFER_ONE *one, uint8_t flag)
{
    /* If we need to add this buffer on the head. */
    if (flag & FS_BUFFER_HEAD)
    {
        /* Add this buffer on the head of the list. */
        sll_push(&buffer->list, one, OFFSETOF(FS_BUFFER_ONE, next));
    }

    else
    {
        /* Add this buffer at the end of the list. */
        sll_append(&buffer->list, one, OFFSETOF(FS_BUFFER_ONE, next));
    }

    /* Update the total length of this buffer. */
    buffer->total_length += one->length;

} /* fs_buffer_add_one */

/*
 * fs_buffer_add_list
 * @buffer: Buffer needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_ONE_FREE: If this is a free buffer list.
 *  FS_BUFFER_RX: If this is a receive buffer list.
 *  FS_BUFFER_TX: If this is a transmit buffer list.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the buffer and invoke the any callbacks.
 * This function will add all the one buffers in a buffer to the desired list.
 */
void fs_buffer_add_list(FS_BUFFER *buffer, uint32_t type, uint32_t flags)
{
    FS_BUFFER_ONE *one;

    /* Wile we have a one buffer to add. */
    do
    {
        /* Pick a one buffer from the buffer list. */
        one = sll_pop(&buffer->list, OFFSETOF(FS_BUFFER_ONE, next));

        if (one)
        {
            /* Add a one buffer from the buffer to the given file descriptor. */
            fs_buffer_add(buffer->fd, one, type, flags);
        }

    } while (one != NULL);

    /* There are no more buffers, reset the buffer length. */
    buffer->total_length = 0;

} /* fs_buffer_add_list */

/*
 * fs_buffer_add_buffer_list
 * @buffer: Buffer needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_RX: If this is a receive buffer list.
 *  FS_BUFFER_TX: If this is a transmit buffer list.
 *  FS_BUFFER_LIST: If this is a buffer list.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the buffer and invoke the any callbacks.
 * This function will add all the buffers in a buffer list to the desired list.
 */
void fs_buffer_add_buffer_list(FS_BUFFER *buffer, uint32_t type, uint32_t flags)
{
    FS_BUFFER *next_buffer;

    /* Wile we have a buffer to add. */
    while (buffer != NULL)
    {
        /* Save the next buffer we need to process. */
        next_buffer = buffer->next;

        /* Add this buffer to the desired buffer list. */
        fs_buffer_add(buffer->fd, buffer, type, flags);

        /* Pick the next buffer we need to process. */
        buffer = next_buffer;
    }

} /* fs_buffer_add_buffer_list */

/*
 * fs_buffer_add
 * @fd: File descriptor on which a free buffer is needed to be added.
 * @buffer: Buffer needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_ONE_FREE: If this is a free buffer.
 *  FS_BUFFER_RX: If this is a receive buffer.
 *  FS_BUFFER_TX: If this is a transmit buffer.
 *  FS_BUFFER_LIST: If this is a free buffer list.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the buffer and invoke the any callbacks.
 *  FS_BUFFER_HEAD: If buffer is needed to be added on the head of a list.
 * This function will adds a buffer in the file descriptor for the required
 * type.
 */
void fs_buffer_add(FD fd, void *buffer, uint32_t type, uint32_t flags)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    FS_BUFFER_PARAM param;
    RESUME resume;
    uint8_t do_resume = TRUE;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

#ifdef FS_BUFFER_DEBUG
    /* Check if this node already exists on any of the file descriptor lists. */
    OS_ASSERT(sll_in_list(&data->rx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next)) == TRUE);
    OS_ASSERT(sll_in_list(&data->tx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next)) == TRUE);
    OS_ASSERT(sll_in_list(&data->free_buffer_list, buffer, OFFSETOF(FS_BUFFER, next)) == TRUE);
    OS_ASSERT(sll_in_list(&data->buffers_list, buffer, OFFSETOF(FS_BUFFER, next)) == TRUE);
#endif

    /* Type of buffer we are adding. */
    switch (type)
    {

    /* A free buffer. */
    case FS_BUFFER_ONE_FREE:

        /* A buffer should not get here. */
        OS_ASSERT(((FS_BUFFER *)buffer)->id == FS_BUFFER_ID_BUFFER);

        /* Reinitialize a one buffer. */
        fs_buffer_one_init(((FS_BUFFER_ONE *)buffer), ((FS_BUFFER_ONE *)buffer)->data, ((FS_BUFFER_ONE *)buffer)->max_length);

        /* Just add this buffer in the free buffer list. */
        sll_append(&data->free_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

        /* Increment the number of buffers on free list. */
        data->free_buffer_list.buffers ++;

#ifdef FS_BUFFER_DEBUG
        /* Should not happen. */
        OS_ASSERT(data->buffers == 0);

        /* Decrement number of buffers in the system. */
        data->buffers --;
#endif

        /* If we are doing this actively. */
        if (flags & FS_BUFFER_ACTIVE)
        {
            /* Some space is now available. */
            fd_space_available(fd);
        }
        else
        {
            /* Just set flag that some data is available. */
            ((FS *)fd)->flags |= FS_SPACE_AVAILABLE;
        }

        break;

    /* A receive buffer. */
    case FS_BUFFER_RX:

        /* If we need to add this on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* Add this buffer on the head of receive list. */
            sll_push(&data->rx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));
        }
        else
        {
            /* Just add this buffer in the receive buffer list. */
            sll_append(&data->rx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));
        }

#ifdef FS_BUFFER_DEBUG
        /* Increment the number of buffers on receive list. */
        data->rx_buffer_list.buffers ++;
#endif

#ifdef FS_BUFFER_DEBUG
        /* Should not happen. */
        OS_ASSERT(data->buffers == 0);

        /* Decrement number of buffers in the system. */
        data->buffers --;
#endif

        /* If we are doing this actively. */
        if (flags & FS_BUFFER_ACTIVE)
        {
            /* Some new data is now available. */
            fd_data_available(fd);
        }
        else
        {
            /* Just set the flags that data is available. */
            ((FS *)fd)->flags |= FS_DATA_AVAILABLE;
        }

        break;

    /* A transmit buffer. */
    case FS_BUFFER_TX:

        /* If we need to add this on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* Add this buffer on the head of transmit list. */
            sll_push(&data->tx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));
        }
        else
        {
            /* Just add this buffer in the transmit buffer list. */
            sll_append(&data->tx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));
        }

#ifdef FS_BUFFER_DEBUG
        /* Increment the number of buffers on transmit list. */
        data->tx_buffer_list.buffers ++;
#endif

#ifdef FS_BUFFER_DEBUG
        /* Should not happen. */
        OS_ASSERT(data->buffers == 0);

        /* Decrement number of buffers in the system. */
        data->buffers --;
#endif

        break;

    /* A buffer list buffer. */
    case FS_BUFFER_LIST:

        /* A one buffer should not get here. */
        OS_ASSERT(((FS_BUFFER *)buffer)->id == FS_BUFFER_ID_ONE);

        /* Check if we need to return this buffer to somebody else. */
        if ((((FS_BUFFER *)buffer)->free != NULL) && (((FS_BUFFER *)buffer)->free(((FS_BUFFER *)buffer)->free_data, buffer) == TRUE))
        {
            /* No need to resume any one. */
            do_resume = FALSE;
        }
        else
        {
            /* First free any buffer still on this list. */
            fs_buffer_add_list((FS_BUFFER *)buffer, FS_BUFFER_ONE_FREE, flags);

            /* Reinitialize this buffer. */
            fs_buffer_init(((FS_BUFFER *)buffer), ((FS_BUFFER *)buffer)->fd);

            /* Just add this buffer in the buffer list. */
            sll_append(&data->buffers_list, buffer, OFFSETOF(FS_BUFFER, next));

            /* Increment the number of buffers on buffer list. */
            data->buffers_list.buffers ++;
        }

        break;
    }

    /* If we can resume anyone waiting on this buffer. */
    if (do_resume == TRUE)
    {
        /* Type of buffer we are added. */
        switch (type)
        {
            /* A free buffer or a buffer list. */
            case FS_BUFFER_ONE_FREE:
            case FS_BUFFER_LIST:

            /* Initialize resume criteria. */
            param.num_buffers = ((type == FS_BUFFER_ONE_FREE) ? data->free_buffer_list.buffers : data->buffers_list.buffers);
            param.type = type;

            /* Initialize resume criteria. */
            resume.do_resume = &fs_buffer_do_resume;
            resume.param = &param;
            resume.status = TASK_RESUME;

            /* Resume any tasks waiting on this buffer. */
            resume_condition(&data->condition, &resume, TRUE);

            break;
        }
    }

#ifdef FS_BUFFER_DEBUG
    /* Validate the buffer lists for this file descriptors. */
    OS_ASSERT((int32_t)sll_num_items(&data->rx_buffer_list, OFFSETOF(FS_BUFFER, next)) != data->rx_buffer_list.buffers);
    OS_ASSERT((int32_t)sll_num_items(&data->tx_buffer_list, OFFSETOF(FS_BUFFER, next)) != data->tx_buffer_list.buffers);
    OS_ASSERT((int32_t)sll_num_items(&data->free_buffer_list, OFFSETOF(FS_BUFFER, next)) != data->free_buffer_list.buffers);
    OS_ASSERT((int32_t)sll_num_items(&data->buffers_list, OFFSETOF(FS_BUFFER, next)) != data->buffers_list.buffers);
#endif

} /* fs_buffer_add */

/*
 * fs_buffer_get_by_id
 * @fd: File descriptor from which a free buffer is needed.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_ONE_FREE: If a free buffer is needed.
 *  FS_BUFFER_RX: If a receive buffer is needed.
 *  FS_BUFFER_TX: If a transmit buffer is needed.
 *  FS_BUFFER_LIST: If a list buffer is needed.
 * @flags: Operation flags.
 *  FS_BUFFER_INPLACE: Will not remove the buffer from the list just return a
 *      pointer to it.
 *  FS_BUFFER_SUSPEND: If needed suspend to wait for a buffer.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @id: Buffer ID we need to get.
 * This function return a buffer from a required buffer list for this file
 * descriptor.
 */
void *fs_buffer_get_by_id(FD fd, uint32_t type, uint32_t flags, uint32_t id)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    void *buffer = NULL;
    int32_t status = SUCCESS;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

    /* Type of buffer we need. */
    switch (type)
    {

    /* A free buffer. */
    case FS_BUFFER_ONE_FREE:

        /* Validate the input arguments. */
        OS_ASSERT(flags & FS_BUFFER_INPLACE);
        OS_ASSERT(id != FS_BUFFER_ID_ONE);

        /* Check if we need to suspend. */
        if (flags & FS_BUFFER_SUSPEND)
        {
            /* Suspend if required on this buffer. */
            /* Check if we have required number of buffers. */
            if (data->free_buffer_list.buffers <= ((flags & FS_BUFFER_TH) ? data->threshold_buffers : 0))
            {
                /* Suspend to wait for buffers. */
                status = fs_buffer_suspend(fd, type, flags);
            }
        }

        if (status == SUCCESS)
        {
            /* Pop a buffer from this file descriptor's free buffer list. */
            buffer = sll_search_pop(&data->free_buffer_list, &fs_buffer_type_search, (void *)(&id), OFFSETOF(FS_BUFFER, next));
        }

        /* If we are returning a buffer. */
        if (buffer)
        {
            /* Decrement the number of buffers on free list. */
            data->free_buffer_list.buffers --;

            /* Clear the next buffer pointer. */
            ((FS_BUFFER *)buffer)->next = NULL;
        }

        /* If we don't have any more free space on this file descriptor. */
        if (data->free_buffer_list.head == NULL)
        {
            /* Tell the file system to block the write until there is some
             * space available. */
            fd_space_consumed(fd);
        }

        break;

    /* A receive buffer. */
    case FS_BUFFER_RX:

        /* If we need to return the buffer in-place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* Return the pointer to the head buffer. */
            buffer = data->rx_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's receive buffer list. */
            buffer = sll_search_pop(&data->rx_buffer_list, &fs_buffer_type_search, (void *)(&id), OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_DEBUG
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on receive list. */
                data->rx_buffer_list.buffers --;
            }
#endif

            /* If we don't have any more data to read. */
            if (data->rx_buffer_list.head == NULL)
            {
                /* No more data is available to read. */
                fd_data_flushed(fd);
            }
        }

        break;

    /* A transmit buffer. */
    case FS_BUFFER_TX:

        /* If we need to return the buffer in-place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* Return the pointer to the head buffer. */
            buffer = data->tx_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's transmit buffer list. */
            buffer = sll_search_pop(&data->tx_buffer_list, &fs_buffer_type_search, (void *)(&id), OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_DEBUG
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on transmit list. */
                data->tx_buffer_list.buffers --;
            }
#endif
        }

        break;

    /* A buffer list buffer. */
    case FS_BUFFER_LIST:

        /* Validate the input arguments. */
        OS_ASSERT(flags & FS_BUFFER_INPLACE);
        OS_ASSERT(id != FS_BUFFER_ID_BUFFER);

        /* Check if we need to suspend. */
        if (flags & FS_BUFFER_SUSPEND)
        {
            /* Check if we have required number of buffers. */
            if (data->buffers_list.buffers <= ((flags & FS_BUFFER_TH) ? data->threshold_lists : 0))
            {
                /* Suspend to wait for buffers. */
                status = fs_buffer_suspend(fd, type, flags);
            }
        }

        if (status == SUCCESS)
        {
            /* Pop a buffer from this file descriptor's buffer list. */
            buffer = sll_search_pop(&data->buffers_list, &fs_buffer_type_search, (void *)(&id), OFFSETOF(FS_BUFFER, next));
        }

        /* If we are returning a buffer. */
        if (buffer)
        {
            /* Decrement the number of buffers on transmit list. */
            data->buffers_list.buffers --;

            /* Clear the next buffer pointer. */
            ((FS_BUFFER *)buffer)->next = NULL;
        }

        break;
    }

    /* If we are returning a buffer. */
    if ( (buffer) &&
         (type != FS_BUFFER_LIST) &&
         ((flags & FS_BUFFER_INPLACE) == 0) )
    {
#ifdef FS_BUFFER_DEBUG
        /* Increase the number of buffers in the system. */
        data->buffers ++;
#endif
    }

#ifdef FS_BUFFER_DEBUG
    /* Validate the buffer lists for this file descriptors. */
    OS_ASSERT((int32_t)sll_num_items(&data->rx_buffer_list, OFFSETOF(FS_BUFFER, next)) != data->rx_buffer_list.buffers);
    OS_ASSERT((int32_t)sll_num_items(&data->tx_buffer_list, OFFSETOF(FS_BUFFER, next)) != data->tx_buffer_list.buffers);
    OS_ASSERT((int32_t)sll_num_items(&data->free_buffer_list, OFFSETOF(FS_BUFFER, next)) != data->free_buffer_list.buffers);
    OS_ASSERT((int32_t)sll_num_items(&data->buffers_list, OFFSETOF(FS_BUFFER, next)) != data->buffers_list.buffers);
#endif

    /* Return the buffer. */
    return ((void *)buffer);

} /* fs_buffer_get_by_id */

/*
 * fs_buffer_pull_offset
 * @buffer: File buffer from which data is needed to be pulled.
 * @data: Buffer in which data is needed to be pulled.
 * @size: Number of bytes needed to be pulled.
 * @offset: Number of bytes from start or end the required data lies.
 * @flags: Defines how we will be pulling the data.
 *  FS_BUFFER_PACKED: If we need to pull a packet structure.
 *  FS_BUFFER_TAIL: If we need to pull data from the tail.
 *  FS_BUFFER_INPLACE: If we are just peeking and don't want data to be removed
 *      actually.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will remove data from a buffer. If given will also copy the
 * data in the provided buffer.
 */
int32_t fs_buffer_pull_offset(FS_BUFFER *buffer, void *data, uint32_t size, uint32_t offset, uint8_t flags)
{
    FS_BUFFER_ONE *one = NULL;
    int32_t status = SUCCESS;
    uint32_t this_size, this_offset = offset;
#ifdef OS_LITTLE_ENDIAN
    uint8_t reverse = (uint8_t)((flags & FS_BUFFER_PACKED) != 0);
#endif
    uint8_t *to;

    /* Head flag should not be used with pull. */
    OS_ASSERT(flags & FS_BUFFER_HEAD);

    /* Validate that we do have enough space on this buffer. */
    if (buffer->total_length >= (size + offset))
    {
        /* If an offset was given. */
        if (offset != 0)
        {
            /* We should not be removing the data. */
            OS_ASSERT((flags & FS_BUFFER_INPLACE) == 0);
        }

        /* If we are pulling data in place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* If need to pull from the tail. */
            if (flags & FS_BUFFER_TAIL)
            {
                /* Adjust the offset to the start of buffer. */
                this_offset = buffer->total_length - (this_offset + size);

                /* Clear the tail flag. */
                flags &= (uint8_t)~(FS_BUFFER_TAIL);
            }

            /* Pick the head buffer. */
            one = buffer->list.head;

            /* While we have some offset and the data in this buffer is greater
             * than the offset. */
            while ((this_offset > 0) && (one != NULL) && (this_offset > one->length))
            {
                /* Remove this buffer from the offset. */
                this_offset -= one->length;

                /* Pick the next buffer. */
                one = one->next;
            }

            /* We should have a buffer here. */
            OS_ASSERT(one == NULL);
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Validate if we do have that much data on the buffer. */
    if (status == SUCCESS)
    {
        /* While we have some data to copy. */
        while (size > 0)
        {
            /* If we are not removing the pulled data. */
            if ((flags & FS_BUFFER_INPLACE) == 0)
            {
                /* If we need to pull data from the tail. */
                if (flags & FS_BUFFER_TAIL)
                {
                    /* Pick the tail buffer. */
                    one = buffer->list.tail;
                }
                else
                {
                    /* Pick the head buffer. */
                    one = buffer->list.head;
                }
            }

            /* There is data in the buffer so we must have a one buffer. */
            OS_ASSERT(one == NULL);

            /* There should not be a zero length buffer. */
            OS_ASSERT(one->length == 0);

            /* Check if we need to do a partial read of this buffer. */
            if ((size + this_offset) >= one->length)
            {
                /* Only copy the number of bytes that can be copied from
                 * this buffer. */
                this_size = (one->length - this_offset);
            }
            else
            {
                /* Copy the given number of bytes. */
                this_size = size;
            }

            /* Pick the destination pointer. */
            to = (uint8_t *)data;

#ifdef OS_LITTLE_ENDIAN
            /* If we need to copy data MSB first. */
            if ((data != NULL) && (reverse == TRUE))
            {
                /* Move to the offset at the end of the provided buffer. */
                to += (size - this_size);
            }
#endif

            /* Pull data from this buffer. */
            /* We have already verified that we have enough length on the buffer,
             * so we should never get an error here. */
            OS_ASSERT(fs_buffer_one_pull_offset(one, to, this_size, this_offset, flags) != SUCCESS);

            /* If there is no more valid data on this buffer. */
            if (one->length == 0)
            {
                /* We no longer need this one buffer on our buffer. */
                OS_ASSERT(sll_remove(&buffer->list, one, OFFSETOF(FS_BUFFER_ONE, next)) != one);

                /* Actively free this buffer. */
                fs_buffer_add(buffer->fd, one, FS_BUFFER_ONE_FREE, FS_BUFFER_ACTIVE);
            }

            /* Decrement the number of bytes we still need to copy. */
            size = (uint32_t)(size - this_size);

            /* If we are returning the data. */
            if ( (data != NULL)
#ifdef OS_LITTLE_ENDIAN
                 && (reverse == FALSE)
#endif
                )
            {
                /* Update the data pointer. */
                data = ((uint8_t *)data + this_size);
            }

            /* If we are not peeking the data. */
            if ((flags & FS_BUFFER_INPLACE) == 0)
            {
                /* Decrement number of bytes we have left on this buffer. */
                buffer->total_length = (buffer->total_length - this_size);
            }

            /* If we are not pulling data in place. */
            if (flags & FS_BUFFER_INPLACE)
            {
                /* Reset the offset as this will not be required for next buffers. */
                this_offset = 0;

                /* Pick the next one buffer. */
                one = one->next;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_pull_offset */

/*
 * fs_buffer_push_offset
 * @buffer: File buffer on which data is needed to be pushed.
 * @data: Buffer from which data is needed to pushed.
 * @size: Number of bytes needed to be pushed.
 * @offset: Number of bytes from start or end the required data lies.
 * @flags: Defines how we will be pushing the data.
 *  FS_BUFFER_UPDATE: If we are not adding new data and need to update the
 *      existing data.
 *  FS_BUFFER_PACKED: If we need to push a packet structure.
 *  FS_BUFFER_HEAD: If data is needed to be pushed on the head.
 *  FS_BUFFER_SUSPEND: If needed suspend to wait for a buffer.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  file descriptor for new buffers.
 * This function will add data to the buffer.
 */
int32_t fs_buffer_push_offset(FS_BUFFER *buffer, void *data, uint32_t size, uint8_t offset, uint8_t flags)
{
    int32_t status = SUCCESS;
    FS_BUFFER_ONE *one = NULL;
    uint32_t this_size, this_offset = offset;
#ifdef OS_LITTLE_ENDIAN
    uint8_t reverse = (uint8_t)(((flags & FS_BUFFER_PACKED) != 0) ^ (((flags & FS_BUFFER_HEAD) != 0) && ((flags & FS_BUFFER_UPDATE) == 0) && (offset == 0)));
#endif
    uint8_t *from;

    /* If an offset was given. */
    if (offset != 0)
    {
        /* We should be updating the existing data. */
        OS_ASSERT((flags & FS_BUFFER_UPDATE) == 0);
    }

    /* If we are updating the existing data. */
    if (flags & FS_BUFFER_UPDATE)
    {
        /* The buffer should already have the data we need to update. */
        OS_ASSERT(buffer->total_length < (size + offset));

        /* If we are not updating the data on the head. */
        if ((flags & FS_BUFFER_HEAD) == 0)
        {
            /* Adjust the offset from the start. */
            this_offset = (buffer->total_length - (size + this_offset));

            /* Set the head flag. */
            flags |= FS_BUFFER_HEAD;
        }

        /* Pick the head buffer. */
        one = buffer->list.head;

        /* While we have an offset and nothing is needed from this buffer. */
        while ((this_offset > 0) && (one != NULL) && (this_offset >= one->length))
        {
            /* Remove data for this buffer from the offset. */
            this_offset -= one->length;

            /* Pick the next buffer. */
            one = one->next;
        }

        /* If we don't have a one buffer. */
        if (one == NULL)
        {
            /* There is no space in this buffer. */
            status = FS_BUFFER_NO_SPACE;
        }
    }

    /* While we have some data to copy. */
    while ((status == SUCCESS) && (size > 0))
    {
        /* If we need to push data on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* Pick the head buffer. */
                one = buffer->list.head;

                /* Either we don't have a one buffer in the buffer or there is no
                 * space on the head buffer. */
                if ((one == NULL) || (FS_BUFFER_SPACE(one) == 0))
                {
                    /* Need to allocate a new buffer to be pushed on the head. */
                    one = fs_buffer_one_get(buffer->fd, FS_BUFFER_ONE_FREE, flags);

                    /* If a buffer was allocated. */
                    if (one)
                    {
                        /* Use all the space in this buffer as head room. */
                        OS_ASSERT(fs_buffer_one_add_head(one, one->length) != SUCCESS);

                        /* Add this one buffer on the head of the buffer. */
                        sll_push(&buffer->list, one, OFFSETOF(FS_BUFFER_ONE, next));
                    }
                }

                if (one != NULL)
                {
                    /* Pick the number of bytes we can copy on this buffer. */
                    this_size = FS_BUFFER_SPACE(one);

                    /* If we have more space then we require. */
                    if (this_size > size)
                    {
                        /* Copy the required amount of data. */
                        this_size = size;
                    }
                }
                else
                {
                    /* There is no space on the file descriptor. */
                    status = FS_BUFFER_NO_SPACE;
                }
            }
            else
            {
                /* Pick the number of bytes we need to copy. */
                this_size = size;

                /* If not all the data in this buffer can be updated. */
                if ((this_size + this_offset) > one->length)
                {
                    /* Copy the data that can be copied in this buffer. */
                    this_size = (one->length - this_offset);
                }
            }
        }
        else
        {
            /* Pick the tail buffer. */
            one = buffer->list.tail;

            /* We should not be updating the existing value. */
            OS_ASSERT(flags & FS_BUFFER_UPDATE);

            /* Either we don't have a one buffer in the or there is no space
             * in the tail buffer. */
            if ((one == NULL) || (FS_BUFFER_TAIL_ROOM(one) == 0))
            {
                /* Need to allocate a new buffer to be appended on the tail. */
                one = fs_buffer_one_get(buffer->fd, FS_BUFFER_ONE_FREE, flags);

                /* If a buffer was allocated. */
                if (one)
                {
                    /* Append this buffer at the end of buffer. */
                    sll_append(&buffer->list, one, OFFSETOF(FS_BUFFER_ONE, next));
                }
            }

            if (one != NULL)
            {
                /* Pick the number of bytes we can copy on this buffer. */
                this_size = FS_BUFFER_TAIL_ROOM(one);

                /* If we have more space then we require. */
                if (this_size > size)
                {
                    /* Copy the required amount of data. */
                    this_size = size;
                }
            }
            else
            {
                /* There is no space on the file descriptor. */
                status = FS_BUFFER_NO_SPACE;
            }
        }

        if (status == SUCCESS)
        {
            /* Pick the source pointer. */
            from = (uint8_t *)data;

#ifdef OS_LITTLE_ENDIAN
            /* If we need to copy data MSB first. */
            if ((data != NULL) && (reverse == TRUE))
            {
                /* Move to the offset at the end of the provided buffer. */
                from += (size - this_size);
            }
#endif

            /* Push data on the buffer we have selected. */
            OS_ASSERT(fs_buffer_one_push_offset(one, from, this_size, this_offset, flags) != SUCCESS);

            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* Update the buffer size. */
                buffer->total_length += this_size;
            }

            /* Decrement the bytes we have copied in this go. */
            size = size - this_size;

            /* If we are copying data normally. */
            if ( (data != NULL)
#ifdef OS_LITTLE_ENDIAN
                 && (reverse == FALSE)
#endif
                )
            {
                /* Update the data pointer. */
                data = (uint8_t *)data + this_size;
            }

            /* If we are updating the existing value. */
            if (flags & FS_BUFFER_UPDATE)
            {
                /* Reset the offset. */
                this_offset = 0;

                /* Pick the next one buffer. */
                one = one->next;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_push_offset */

/*
 * fs_buffer_divide
 * @buffer: Buffer needed to be divided.
 * @flags: Operation flags.
 *  FS_BUFFER_SUSPEND: If needed suspend to wait for a buffer.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @data_len: Number of bytes valid in the original buffer.
 * @return: A success status will be returned if buffer was successfully divided,
 *  FS_BUFFER_NO_SPACE will be returned if there is no buffer to store remaining
 *  data of this buffer.
 * This function will divide the given buffer into two buffers. An empty buffer
 * will allocated to hold the remaining portion of buffer.
 */
int32_t fs_buffer_divide(FS_BUFFER *buffer, uint32_t flags, uint32_t data_len)
{
    FS_BUFFER_ONE *one, *new_one = NULL;
    FS_BUFFER *new_buffer;
    int32_t status = SUCCESS;
    uint32_t this_len = data_len;

    /* Should never happen. */
    OS_ASSERT(buffer->total_length <= data_len);

    /* Find the one buffer we need to divide. */
    one = buffer->list.head;

    while (one)
    {
        /* If remaining data length is less than the data in this buffer. */
        if (this_len < one->length)
        {
            /* Break out of this loop. */
            break;
        }

        /* Remove data length for this one buffer. */
        this_len -= one->length;

        /* If we need zero bytes from next one buffer. */
        if (this_len == 0)
        {
            /* The new one buffer will be the next one buffer. */
            new_one = one->next;

            /* Break out of this loop. */
            break;
        }

        /* Pick the next one buffer. */
        one = one->next;
    }

    /* Should never happen. */
    OS_ASSERT(one == NULL);

    /* Get a new buffer to store the remaining data for this buffer. */
    new_buffer = fs_buffer_get(buffer->fd, FS_BUFFER_LIST, flags);

    /* If we do have a buffer to store remaining data of this buffer. */
    if (new_buffer != NULL)
    {
        /* If we have to divide the one buffer. */
        if (new_one == NULL)
        {
            /* Remove the extra data from this one buffer to a new buffer. */
            OS_ASSERT(fs_buffer_one_divide(buffer->fd, one, &new_one, flags, this_len) != SUCCESS);

            /* Initialize the new one buffers. */
            new_one->next = one->next;
        }

        /* This will be last one buffer in the original buffer. */
        one->next = NULL;

        /* Initialize the new buffer. */
        new_buffer->list.head = new_one;
        new_buffer->list.tail = buffer->list.tail;
        new_buffer->total_length = (buffer->total_length - data_len);

        /* Divide the original buffer. */
        buffer->list.tail = one;
        buffer->total_length = data_len;

        /* Put new buffer in the buffer chain. */
        buffer->next = new_buffer;
        new_buffer->next = NULL;
    }
    else
    {
        /* Return error to the caller. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_divide */

/*
 * fs_buffer_one_add_head
 * @buffer: File one buffer needed to be updated.
 * @size: Size of head room needed to be left in the buffer.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will add a head room to the given one buffer, if there is
 * already some data on the buffer it will be moved. If it already has some
 * head room it will be maintained.
 */
int32_t fs_buffer_one_add_head(FS_BUFFER_ONE *one, uint32_t size)
{
    int32_t status = SUCCESS;

    /* An empty buffer should not come here. */
    OS_ASSERT(one->data == NULL);

    /* Validate that there is enough space on the buffer. */
    if (FS_BUFFER_SPACE(one) >= size)
    {
        /* Check if we have some data on the buffer. */
        if (one->length != 0)
        {
            /* Move the data to make room for head. */
            memmove(&one->buffer[size], one->buffer, one->length);
        }

        /* Update the buffer pointer. */
        one->buffer = one->buffer + size;
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_one_add_head */

/*
 * fs_buffer_one_pull
 * @buffer: File buffer from which data is needed to be pulled.
 * @data: Buffer in which data is needed to be pulled.
 * @size: Number of bytes needed to be pulled.
 * @offset: Number of bytes from start or end the required data lies.
 * @flags: Defines how we will be pulling the data.
 *  FS_BUFFER_PACKED: If we need to pull a packet structure.
 *  FS_BUFFER_TAIL: If we need to pull data from the tail.
 *  FS_BUFFER_INPLACE: If we are just peeking and don't want data to be removed
 *      actually.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will remove data from a given buffer. If given will also copy
 * the data in the provided buffer.
 */
int32_t fs_buffer_one_pull_offset(FS_BUFFER_ONE *one, void *data, uint32_t size, uint32_t offset, uint8_t flags)
{
    uint8_t *from;
    int32_t status = SUCCESS;

    /* If an offset was given. */
    if (offset != 0)
    {
        /* We should not be removing the data. */
        OS_ASSERT((flags & FS_BUFFER_INPLACE) == 0);
    }

    /* Validate if we do have required amount of data on the buffer. */
    if (one->length >= (size + offset))
    {
        /* If we need to pull data from the tail. */
        if (flags & FS_BUFFER_TAIL)
        {
            /* Pick the data from the end of the buffer. */
            from = &one->buffer[one->length - (size + offset)];
        }
        else
        {
            /* Pick the data from the start of the buffer. */
            from = &one->buffer[offset];

            /* If we don't want data to be removed. */
            if ((flags & FS_BUFFER_INPLACE) == 0)
            {
                /* Advance the buffer pointer. */
                one->buffer += size;
            }
        }

        /* If we need to actually need to return the pulled data. */
        if (data != NULL)
        {
#ifdef OS_LITTLE_ENDIAN
            if (flags & FS_BUFFER_PACKED)
            {
                /* Copy the last byte first. */
                fs_memcpy_r(data, from, size);
            }
            else
#endif
            {
                /* Copy the data in the provided buffer. */
                memcpy(data, from, size);
            }
        }

        /* If we don't want data to be removed. */
        if ((flags & FS_BUFFER_INPLACE) == 0)
        {
            /* Update the buffer length. */
            one->length -= size;
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_one_pull_offset */

/*
 * fs_buffer_one_push
 * @buffer: File buffer on which data is needed to be pushed.
 * @data: Buffer from which data is needed to pushed.
 * @size: Number of bytes needed to be pushed.
 * @offset: Number of bytes from start or end the required data lies.
 * @flags: Defines how we will be pushing the data.
 *  FS_BUFFER_UPDATE: If we are not adding new data and need to update the
 *      existing data.
 *  FS_BUFFER_PACKED: If we need to push a packet structure.
 *  FS_BUFFER_HEAD: If data is needed to be pushed on the head.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will add data in the buffer.
 */
int32_t fs_buffer_one_push_offset(FS_BUFFER_ONE *one, void *data, uint32_t size, uint32_t offset, uint8_t flags)
{
    int32_t status = SUCCESS;
    uint8_t *to;

    /* An empty buffer should not come here. */
    OS_ASSERT(one->data == NULL);

    /* If an offset was given. */
    if (offset != 0)
    {
        /* We should be updating the existing data. */
        OS_ASSERT((flags & FS_BUFFER_UPDATE) == 0);
    }

    /* If we do have enough space on the buffer. */
    if ( ((flags & FS_BUFFER_UPDATE) && (one->length >= (size + offset))) ||
         (((flags & FS_BUFFER_UPDATE) == 0) &&
          (((flags & FS_BUFFER_HEAD) && (FS_BUFFER_SPACE(one) >= size)) ||
           (((flags & FS_BUFFER_HEAD) == 0) && (FS_BUFFER_TAIL_ROOM(one) >= size)))) )
    {
        /* If we need to add head room on this buffer. */
        if ( ((flags & FS_BUFFER_UPDATE) == 0) && (flags & FS_BUFFER_HEAD) && (FS_BUFFER_HEAD_ROOM(one) < size))
        {
            /* Add required head room. */
            status = fs_buffer_one_add_head(one, (size - FS_BUFFER_HEAD_ROOM(one)));
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    if (status == SUCCESS)
    {
        /* If we need to push data on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* We will be adding data at the start of the existing data. */

            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* Update the buffer pointer to the memory at which new data
                 * will be added. */
                one->buffer -= size;
            }

            /* Pick the pointer at which we will be adding data. */
            to = one->buffer + offset;
        }

        else
        {
            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* We will be pushing data at the end of the buffer, no need to
                 * adjust for the offset. */
                to = &one->buffer[one->length];
            }
            else
            {
                /* We will be updating data at the tail, adjust for the offset
                 * and size of the data. */
                to = &one->buffer[one->length - (size + offset)];
            }
        }

        /* If we actually need to push some data. */
        if (data != NULL)
        {
#ifdef OS_LITTLE_ENDIAN
            if (flags & FS_BUFFER_PACKED)
            {
                /* Copy data from the provided buffer last byte first. */
                fs_memcpy_r(to, data, size);
            }

            else
#endif
            {
                /* Copy data from the provided buffer. */
                memcpy(to, data, size);
            }
        }

        /* If we are not updating the existing value. */
        if ((flags & FS_BUFFER_UPDATE) == 0)
        {
            /* Update the buffer length. */
            one->length += size;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_one_push */

/*
 * fs_buffer_one_divide
 * @fd: File descriptor from which given buffer allocated.
 * @buffer: Buffer for which data is needed to be divided.
 * @new_buffer: Buffer that will have the remaining data of this buffer.
 * @flags: Operation flags.
 *  FS_BUFFER_NO_SUSPEND: If needed suspend to wait for a buffer.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @data_len: Number of bytes valid in the original buffer.
 * @return: A success status will be returned if buffer was successfully divided,
 *  FS_BUFFER_NO_SPACE will be returned if there is no buffer to store remaining
 *  data of this buffer.
 * This function will divide the given buffer into two buffers. An empty buffer
 * will allocated to hold the remaining portion of buffer.
 */
int32_t fs_buffer_one_divide(FD fd, FS_BUFFER_ONE *one, FS_BUFFER_ONE **new_one, uint32_t flags, uint32_t data_len)
{
    FS_BUFFER_ONE *ret_one = NULL;
    int32_t status = SUCCESS;

    /* Should never happen. */
    OS_ASSERT(data_len >= one->length);
    OS_ASSERT(new_one == NULL);

    /* Allocate a free buffer. */
    ret_one = fs_buffer_one_get(fd, FS_BUFFER_ONE_FREE, flags);

    /* If a free buffer was allocated. */
    if (ret_one != NULL)
    {
        /* Check if the new buffer has enough space to copy the data. */
        OS_ASSERT(ret_one->max_length < data_len);

        /* Set the number of bytes valid in this buffer. */
        ret_one->length = (one->length - data_len);

        /* Update the number of bytes valid in the new buffer. */
        one->length = data_len;

        /* Copy data from old buffer to the new buffer. */
        memcpy(ret_one->buffer, &one->buffer[data_len], ret_one->length);

        /* Return the new buffer. */
        *new_one = ret_one;
    }
    else
    {
        /* Return error to the caller. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_one_divide */

/*
 * fs_buffer_hdr_pull
 * @buffer: File buffer from which data is needed to be pulled.
 * @data: Buffer in which data is needed to be pulled.
 * @size: Number of bytes needed to be pulled.
 * @flags: Header flags.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function is an abstraction function for header utility.
 */
int32_t fs_buffer_hdr_pull(void *buffer, uint8_t *data, uint32_t size, uint16_t flags)
{
    /* Call the underlying buffer pull function. */
    return (fs_buffer_pull((FS_BUFFER *)buffer, data, size, ((uint8_t)flags)));

} /* fs_buffer_hdr_pull */

/*
 * fs_buffer_hdr_push
 * @buffer: File buffer on which data is needed to be pushed.
 * @data: Buffer from which data is needed to be added.
 * @size: Number of bytes needed to be added.
 * @flags: Header flags.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *      buffer.
 * This function is an abstraction function for header utility.
 */
int32_t fs_buffer_hdr_push(void *buffer, uint8_t *data, uint32_t size, uint16_t flags)
{
    /* Call the underlying buffer pull function. */
    return (fs_buffer_push((FS_BUFFER *)buffer, data, size, (FS_BUFFER_HEAD | (uint8_t)flags)));

} /* fs_buffer_hdr_push */

/*
 * fs_buffer_type_search
 * @buffer: A buffer in the buffer list.
 * @param: Required type of buffer.
 * @return: True if this buffer is required, otherwise False will be returned.
 * This function return is the given buffer is of required type.
 */
uint8_t fs_buffer_type_search(void *buffer, void *param)
{
    uint8_t required = FALSE;

    /* Check if given ID matches the required buffer type. */
    if (((FS_BUFFER *)buffer)->id == (*(uint32_t *)param))
    {
        /* We need to return this buffer. */
        required = TRUE;
    }

    /* Return if we need to return this buffer or not. */
    return (required);

} /* fs_buffer_type_search */

#endif /* CONFIG_FS */
