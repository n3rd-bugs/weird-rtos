/*
 * fs_buffer.c
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
#include <kernel.h>

#ifdef CONFIG_FS
#include <string.h>
#include <sll.h>
#include <fs.h>
#include <header.h>

#ifdef CONFIG_NET
#include <net_condition.h>
#endif

/* Internal function prototypes. */
static uint8_t fs_buffer_do_suspend(void *, void *);
static uint8_t fs_buffer_do_resume(void *, void *);
static int32_t fs_buffer_suspend(FD, uint32_t, uint32_t, uint32_t);

/*
 * fs_buffer_dataset
 * @fd: File descriptor for which buffer data-set is needed to be set.
 * @data: Pointer to buffer data structure.
 * @num_buffers: Total number of buffers in this descriptor.
 * @num_buffer_lists: Total number of buffer lists in this descriptor.
 * @buffer_size: Size of each buffer.
 * @threshold_buffers: Number if buffers to be left in threshold.
 * This function will set the buffer structure to be used by this file
 * descriptor also sets the flag to tell others that this will be a buffered
 * file descriptor.
 */
void fs_buffer_dataset(FD fd, FS_BUFFER_DATA *data)
{
    FS *fs = (FS *)fd;
    uint32_t i;

    /* Should never happen. */
    ASSERT(data == NULL);

    ASSERT(data->buffer_space == NULL);
    ASSERT(data->buffers == NULL);
    ASSERT(data->buffer_lists == NULL);

    /* Get lock for this file descriptor. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    /* This is a buffer file system. */
    fs->flags |= FS_BUFFERED;

    /* Set the buffer data structure provided by caller. */
    fs->buffer = data;

    /* Initialize buffer condition data. */
    fs_buffer_condition_init(fs);

    /* Release lock for this file descriptor. */
    fd_release_lock(fd);

    /* Add buffer for this device. */
    for (i = 0; i < data->num_buffers; i++)
    {
        /* Initialize a buffer. */
        fs_buffer_init(&data->buffers[i], &data->buffer_space[data->buffer_size * i], data->buffer_size);

        /* Add this buffer to the free buffer list for this file descriptor. */
        fs_buffer_add(fd, &data->buffers[i], FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
    }

    /* Add buffer lists for this device. */
    for (i = 0; i < data->num_buffer_lists; i++)
    {
        /* Initialize a buffer list. */
        fs_buffer_list_init(&data->buffer_lists[i], fd);

        /* Add this list to the free list for this file descriptor. */
        fs_buffer_add(fd, &data->buffer_lists[i], FS_LIST_FREE, FS_BUFFER_ACTIVE);
    }

} /* fs_buffer_dataset */

/*
 * fs_buffer_list_init
 * @list: File buffer list needed to be initialized.
 * @fd: File descriptor for this buffer list.
 * This function will initialize a buffer list structure.
 */
void fs_buffer_list_init(FS_BUFFER_LIST *list, FD fd)
{
    /* Clear this buffer. */
    memset(list, 0, sizeof(FS_BUFFER_LIST));

    /* Initialize this buffer. */
    list->fd = fd;

} /* fs_buffer_list_init */

/*
 * fs_buffer_init
 * @buffer: File buffer needed to be initialized.
 * @data: Data space needed to be used for this buffer.
 * @size: Size of the data allocated for this buffer.
 * This function will initialize a buffer with given data.
 */
void fs_buffer_init(FS_BUFFER *buffer, void *data, uint32_t size)
{
    /* Initialize this buffer. */
    buffer->data = buffer->buffer = (uint8_t *)data;
    buffer->max_length = size;

    /* Clear the remaining members. */
    buffer->length = 0;
    buffer->next = NULL;

} /* fs_buffer_init */

/*
 * fs_buffer_update
 * @buffer: File buffer needed to be updated.
 * @data: New buffer pointer.
 * @size: Size of valid data in the buffer.
 * This function will update a buffer data pointers.
 */
void fs_buffer_update(FS_BUFFER *buffer, void *data, uint32_t size)
{
    /* Update the buffer data. */
    buffer->buffer = (uint8_t *)data;
    buffer->length = size;

} /* fs_buffer_update */

/*
 * fs_buffer_list_move
 * @dst: Buffer in which we need to make a copy.
 * @src: Buffer needed to be moved.
 * This function will move data of a buffer to another buffer.
 */
void fs_buffer_list_move(FS_BUFFER_LIST *dst, FS_BUFFER_LIST *src)
{
    /* Save the destination buffer file descriptor. */
    FD buffer_fd = dst->fd;

    /* Copy the buffer data as it is. */
    memcpy(dst, src, sizeof(FS_BUFFER_LIST));

    /* Restore the file descriptor for destination buffer. */
    dst->fd = buffer_fd;

    /* Reset the source buffer. */
    fs_buffer_list_init(src, src->fd);

} /* fs_buffer_list_move */

/*
 * fs_buffer_list_move_data
 * @dst: List to which data will be moved.
 * @src: List from which data will be moved.
 * @flags: Operation flags.
 *  FS_BUFFER_HEAD: If we need to add data on the head of existing data,
 *  FS_BUFFER_COPY: If we need to copy data from the source buffer.
 * @return: Success will be returned if data was successfully copied/moved from
 *  buffer.
 * This function will move all the data from a buffer list to the given buffer
 * list.
 */
int32_t fs_buffer_list_move_data(FS_BUFFER_LIST *dst, FS_BUFFER_LIST *src, uint8_t flags)
{
    int32_t status = SUCCESS;
    FS_BUFFER *buffer;

    /* Move all the data from the source buffer to the destination buffer. */

    /* If we are actually copying the data. */
    if (flags & FS_BUFFER_COPY)
    {
        /* Traverse all the buffers. */
        for (buffer = src->list.head; ((status == SUCCESS) && (buffer != NULL)); buffer = buffer->next)
        {
            /* Copy data from this buffer. */
            status = fs_buffer_list_push(dst, buffer->buffer, buffer->length, flags);
        }
    }
    else
    {
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
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_list_move_data */

/*
 * fs_buffer_num_remaining
 * @fd: File descriptor on which number of buffers in a list is required.
 * @type: Type of buffer needed to be checked.
 *  FS_BUFFER_FREE: If this is a free buffer.
 *  FS_LIST_FREE: If this is a free buffer list.
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
    ASSERT(data == NULL);

    /* Type of buffer we need to check. */
    switch (type)
    {
    /* A free buffer. */
    case FS_BUFFER_FREE:

        /* Return number of buffers remaining in the free buffers. */
        ret_num = (int32_t)data->free_buffers.buffers;

        break;

    /* A free buffer list. */
    case FS_LIST_FREE:

        /* Return number of lists remaining in the free list. */
        ret_num = (int32_t)data->free_lists.buffers;

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
    fs->buffer->condition.do_suspend = &fs_buffer_do_suspend;

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

    /* If we are suspending on free buffers. */
    case FS_BUFFER_FREE:

        /* Check if we have required number of buffers. */
        if (fs->buffer->free_buffers.buffers >= param->num_buffers)
        {
            /* Don't need to suspend. */
            do_suspend = FALSE;
        }

        break;

    /* If we are suspending on free buffer lists. */
    case FS_LIST_FREE:

        /* Check if we have required number of buffers. */
        if (fs->buffer->free_lists.buffers >= param->num_buffers)
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
    if (fs_resume->num_buffers >= fs_suspend->num_buffers)
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
void fs_buffer_condition_get(FD fd, CONDITION **condition, SUSPEND *suspend, FS_BUFFER_PARAM *param, uint32_t num_buffers, uint32_t type)
{
    FS *fs = (FS *)fd;
    FS_BUFFER_DATA *data = fs->buffer;

    /* Should never happen. */
    ASSERT(data == NULL);

    /* Initialize buffer suspend parameter. */
    param->num_buffers = num_buffers;
    param->type = type;

    /* Initialize file system parameter. */
    suspend->param = param;
    suspend->timeout_enabled = FALSE;
    suspend->priority = fs->priority;

    /* Return the condition for this file system buffer. */
    *condition = &(data->condition);

} /* fs_buffer_condition_get */

/*
 * fs_buffer_suspend
 * @fd: File descriptor on which we need to suspend for a buffer.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: If a free buffer is needed.
 *  FS_LIST_FREE: If a list buffer is needed.
 * @num_buffers: Number of buffer we would wait.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * This function will suspend on a buffer condition.
 */
static int32_t fs_buffer_suspend(FD fd, uint32_t type, uint32_t num_buffers, uint32_t flags)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    SUSPEND buffer_suspend, *suspend_ptr = &buffer_suspend;
    CONDITION *condition;
    FS_BUFFER_PARAM param;
    int32_t status;

#ifdef CONFIG_NET
    /* We should not be in the networking condition task. */
    ASSERT(get_current_task() == &net_condition_tcb);
#endif

    /* Get buffer condition. */
    fs_buffer_condition_get(fd, &condition, suspend_ptr, &param, ((flags & FS_BUFFER_TH) ? ((type == FS_BUFFER_FREE) ? data->threshold_buffers : data->threshold_lists) : 0) + num_buffers, type);

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
    if ((data->free_buffers.buffers <= data->threshold_buffers) || (data->free_lists.buffers <= data->threshold_lists))
    {
        /* We have reached the buffer threshold. */
        locked = TRUE;
    }

    /* Return if threshold is locked or not. */
    return (locked);

} /* fs_buffer_threshold_locked */

/*
 * fs_buffer_list_append
 * @list: List to which a new buffer is needed to be added.
 * @buffer: Buffer needed to be added.
 * @flags: Defines where the given buffer is needed to be added.
 *  FS_BUFFER_HEAD: If we need to add this buffer on the head.
 * This function will append the given buffer to a buffer list.
 */
void fs_buffer_list_append(FS_BUFFER_LIST *list, FS_BUFFER *buffer, uint8_t flag)
{
    /* If we need to add this buffer on the head. */
    if (flag & FS_BUFFER_HEAD)
    {
        /* Add this buffer on the head of the list. */
        sll_push(&list->list, buffer, OFFSETOF(FS_BUFFER, next));
    }

    else
    {
        /* Add this buffer at the end of the list. */
        sll_append(&list->list, buffer, OFFSETOF(FS_BUFFER, next));
    }

    /* Update the total length of this buffer. */
    list->total_length += buffer->length;

} /* fs_buffer_list_append */

/*
 * fs_buffer_list_append_list
 * @list: List needed to be added.
 * @type: Type of list needed to be added.
 *  FS_BUFFER_FREE: If this is a free buffer list.
 *  FS_BUFFER_RX: If this is a receive buffer list.
 *  FS_BUFFER_TX: If this is a transmit buffer list.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the buffer and invoke the any callbacks.
 * This function will add all the buffers in a list to the desired list.
 */
void fs_buffer_list_append_list(FS_BUFFER_LIST *list, uint32_t type, uint32_t flags)
{
    FS_BUFFER *buffer;

    /* Wile we have a buffer to add. */
    do
    {
        /* Pick a buffer from the buffer list. */
        buffer = sll_pop(&list->list, OFFSETOF(FS_BUFFER, next));

        if (buffer)
        {
            /* Add a buffer from the buffer to the given file descriptor. */
            fs_buffer_add(list->fd, buffer, type, flags);
        }

    } while (buffer != NULL);

    /* There are no more buffers, reset the buffer length. */
    list->total_length = 0;

} /* fs_buffer_list_append_list */

/*
 * fs_buffer_add_list_list
 * @list: List needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_RX: If this is a receive list.
 *  FS_BUFFER_TX: If this is a transmit list.
 *  FS_LIST_FREE: If this is a buffer list.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the lists and invoke the any callbacks.
 * This function will return all the linked lists to the file descriptor.
 */
void fs_buffer_add_list_list(FS_BUFFER_LIST *list, uint32_t type, uint32_t flags)
{
    FS_BUFFER_LIST *next_list;

    /* Wile we have a buffer to add. */
    while (list != NULL)
    {
        /* Save the next buffer we need to process. */
        next_list = list->next;

        /* Add this buffer to the desired buffer list. */
        fs_buffer_add(list->fd, list, type, flags);

        /* Pick the next buffer we need to process. */
        list = next_list;
    }

} /* fs_buffer_add_list_list */

/*
 * fs_buffer_add
 * @fd: File descriptor on which a free buffer is needed to be added.
 * @buffer: Buffer needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: If this is a free buffer.
 *  FS_BUFFER_RX: If this is a receive list.
 *  FS_BUFFER_TX: If this is a transmit list.
 *  FS_LIST_FREE: If this is a free list.
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
    ASSERT(data == NULL);

#ifdef FS_BUFFER_DEBUG
    /* Check if this node already exists on any of the file descriptor lists. */
    ASSERT(sll_in_list(&data->rx_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next)) == TRUE);
    ASSERT(sll_in_list(&data->tx_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next)) == TRUE);
    ASSERT(sll_in_list(&data->free_buffers, buffer, OFFSETOF(FS_BUFFER_LIST, next)) == TRUE);
    ASSERT(sll_in_list(&data->free_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next)) == TRUE);
#endif

    /* Type of buffer we are adding. */
    switch (type)
    {

    /* A free buffer. */
    case FS_BUFFER_FREE:

        /* Reinitialize a buffer. */
        fs_buffer_init(((FS_BUFFER *)buffer), ((FS_BUFFER *)buffer)->data, ((FS_BUFFER *)buffer)->max_length);

        /* Just add this buffer in the free buffer list. */
        sll_append(&data->free_buffers, buffer, OFFSETOF(FS_BUFFER_LIST, next));

        /* Increment the number of buffers on free list. */
        data->free_buffers.buffers ++;

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
            sll_push(&data->rx_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next));
        }
        else
        {
            /* Just add this buffer in the receive buffer list. */
            sll_append(&data->rx_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next));
        }

#ifdef FS_BUFFER_DEBUG
        /* Increment the number of buffers on receive list. */
        data->rx_lists.buffers ++;
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
            sll_push(&data->tx_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next));
        }
        else
        {
            /* Just add this buffer in the transmit buffer list. */
            sll_append(&data->tx_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next));
        }

#ifdef FS_BUFFER_DEBUG
        /* Increment the number of buffers on transmit list. */
        data->tx_lists.buffers ++;
#endif

        break;

    /* A buffer list buffer. */
    case FS_LIST_FREE:

        /* Check if we need to return this buffer to somebody else. */
        if ((((FS_BUFFER_LIST *)buffer)->free != NULL) && (((FS_BUFFER_LIST *)buffer)->free(((FS_BUFFER_LIST *)buffer)->free_data, buffer) == TRUE))
        {
            /* No need to resume any one. */
            do_resume = FALSE;
        }
        else
        {
            /* First free any buffer still on this list. */
            fs_buffer_list_append_list((FS_BUFFER_LIST *)buffer, FS_BUFFER_FREE, flags);

            /* Reinitialize this buffer. */
            fs_buffer_list_init(((FS_BUFFER_LIST *)buffer), ((FS_BUFFER_LIST *)buffer)->fd);

            /* Just add this buffer in the buffer list. */
            sll_append(&data->free_lists, buffer, OFFSETOF(FS_BUFFER_LIST, next));

            /* Increment the number of buffers on buffer list. */
            data->free_lists.buffers ++;
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
            case FS_BUFFER_FREE:
            case FS_LIST_FREE:

            /* Initialize resume criteria. */
            param.num_buffers = ((type == FS_BUFFER_FREE) ? data->free_buffers.buffers : data->free_lists.buffers);
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
    ASSERT(sll_num_items(&data->rx_lists, OFFSETOF(FS_BUFFER_LIST, next)) != data->rx_lists.buffers);
    ASSERT(sll_num_items(&data->tx_lists, OFFSETOF(FS_BUFFER_LIST, next)) != data->tx_lists.buffers);
    ASSERT(sll_num_items(&data->free_buffers, OFFSETOF(FS_BUFFER_LIST, next)) != data->free_buffers.buffers);
    ASSERT(sll_num_items(&data->free_lists, OFFSETOF(FS_BUFFER_LIST, next)) != data->free_lists.buffers);
#endif

} /* fs_buffer_add */

/*
 * fs_buffer_get
 * @fd: File descriptor from which a free buffer is needed.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: If a free buffer is needed.
 *  FS_BUFFER_RX: If a receive list is needed.
 *  FS_BUFFER_TX: If a transmit list is needed.
 *  FS_LIST_FREE: If a list is needed.
 * @flags: Operation flags.
 *  FS_BUFFER_INPLACE: Will not remove the buffer from the list just return a
 *      pointer to it.
 *  FS_BUFFER_SUSPEND: If needed suspend to wait for a buffer.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * This function return a buffer from a required buffer list for this file
 * descriptor.
 */
void *fs_buffer_get(FD fd, uint32_t type, uint32_t flags)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    void *buffer = NULL;
    int32_t status = SUCCESS;

    /* Should never happen. */
    ASSERT(data == NULL);

    /* Type of buffer we need. */
    switch (type)
    {

    /* A free buffer. */
    case FS_BUFFER_FREE:

        /* Validate the input arguments. */
        ASSERT(flags & FS_BUFFER_INPLACE);

        /* Check if we need to suspend. */
        if (flags & FS_BUFFER_SUSPEND)
        {
            /* Suspend if required on this buffer. */
            /* Check if we have required number of buffers. */
            if (data->free_buffers.buffers < (((flags & FS_BUFFER_TH) ? data->threshold_buffers : 0) + 1))
            {
                /* Suspend to wait for buffers. */
                status = fs_buffer_suspend(fd, type, 1, flags);
            }
        }

        if (status == SUCCESS)
        {
            /* Pop a buffer from this file descriptor's free buffer list. */
            buffer = sll_pop(&data->free_buffers, OFFSETOF(FS_BUFFER_LIST, next));
        }

        /* If we are returning a buffer. */
        if (buffer)
        {
            /* Decrement the number of buffers on free list. */
            data->free_buffers.buffers --;

            /* Clear the next buffer pointer. */
            ((FS_BUFFER_LIST *)buffer)->next = NULL;
        }

        /* If we don't have any more free space on this file descriptor. */
        if (data->free_buffers.head == NULL)
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
            buffer = data->rx_lists.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's receive buffer list. */
            buffer = sll_pop(&data->rx_lists, OFFSETOF(FS_BUFFER_LIST, next));

#ifdef FS_BUFFER_DEBUG
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on receive list. */
                data->rx_lists.buffers --;
            }
#endif

            /* If we don't have any more data to read. */
            if (data->rx_lists.head == NULL)
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
            buffer = data->tx_lists.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's transmit buffer list. */
            buffer = sll_pop(&data->tx_lists, OFFSETOF(FS_BUFFER_LIST, next));

#ifdef FS_BUFFER_DEBUG
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on transmit list. */
                data->tx_lists.buffers --;
            }
#endif
        }

        break;

    /* A buffer list buffer. */
    case FS_LIST_FREE:

        /* Validate the input arguments. */
        ASSERT(flags & FS_BUFFER_INPLACE);

        /* Check if we need to suspend. */
        if (flags & FS_BUFFER_SUSPEND)
        {
            /* Check if we have required number of buffers. */
            if (data->free_lists.buffers < (((flags & FS_BUFFER_TH) ? data->threshold_lists : 0) + 1))
            {
                /* Suspend to wait for buffers. */
                status = fs_buffer_suspend(fd, type, 1, flags);
            }
        }

        if (status == SUCCESS)
        {
            /* Pop a buffer from this file descriptor's buffer list. */
            buffer = sll_pop(&data->free_lists, OFFSETOF(FS_BUFFER_LIST, next));
        }

        /* If we are returning a buffer. */
        if (buffer)
        {
            /* Decrement the number of buffers on transmit list. */
            data->free_lists.buffers --;

            /* Clear the next buffer pointer. */
            ((FS_BUFFER_LIST *)buffer)->next = NULL;
        }

        break;
    }

#ifdef FS_BUFFER_DEBUG
    /* Validate the buffer lists for this file descriptors. */
    ASSERT(sll_num_items(&data->rx_lists, OFFSETOF(FS_BUFFER_LIST, next)) != data->rx_lists.buffers);
    ASSERT(sll_num_items(&data->tx_lists, OFFSETOF(FS_BUFFER_LIST, next)) != data->tx_lists.buffers);
    ASSERT(sll_num_items(&data->free_buffers, OFFSETOF(FS_BUFFER_LIST, next)) != data->free_buffers.buffers);
    ASSERT(sll_num_items(&data->free_lists, OFFSETOF(FS_BUFFER_LIST, next)) != data->free_lists.buffers);
#endif

    /* Return the buffer. */
    return ((void *)buffer);

} /* fs_buffer_get */

/*
 * fs_buffer_list_pull_offset
 * @list: Buffer list from which data is needed to be pulled.
 * @data: Data buffer where data will be pulled.
 * @size: Number of bytes needed to be pulled.
 * @offset: Number of bytes from start or end at which the required data lies.
 * @flags: Defines how we will be pulling the data.
 *  FS_BUFFER_PACKED: If we need to pull a packet structure.
 *  FS_BUFFER_TAIL: If we need to pull data from the tail.
 *  FS_BUFFER_INPLACE: If we are just peeking and don't want data to be removed
 *      actually.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will remove data from a list. If given will also copy the
 * data in the provided buffer.
 */
int32_t fs_buffer_list_pull_offset(FS_BUFFER_LIST *list, void *data, uint32_t size, uint32_t offset, uint8_t flags)
{
    FS_BUFFER *buffer = NULL;
    int32_t status = SUCCESS;
    uint32_t this_size, this_offset = offset;
#ifdef LITTLE_ENDIAN
    uint8_t reverse = (uint8_t)((flags & FS_BUFFER_PACKED) != 0);
#endif
    uint8_t *to;

    /* Head flag should not be used with pull. */
    ASSERT(flags & FS_BUFFER_HEAD);

    /* Validate that we do have enough space on this buffer. */
    if (list->total_length >= (size + offset))
    {
        /* If an offset was given. */
        if (offset != 0)
        {
            /* We should not be removing the data. */
            ASSERT((flags & FS_BUFFER_INPLACE) == 0);
        }

        /* If we are pulling data in place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* If need to pull from the tail. */
            if (flags & FS_BUFFER_TAIL)
            {
                /* Adjust the offset to the start of buffer. */
                this_offset = list->total_length - (this_offset + size);

                /* Clear the tail flag. */
                flags &= (uint8_t)~(FS_BUFFER_TAIL);
            }

            /* Pick the head buffer. */
            buffer = list->list.head;

            /* While we have some offset and the data in this buffer is greater
             * than the offset. */
            while ((this_offset > 0) && (buffer != NULL) && (this_offset > buffer->length))
            {
                /* Remove this buffer from the offset. */
                this_offset -= buffer->length;

                /* Pick the next buffer. */
                buffer = buffer->next;
            }

            /* We should have a buffer here. */
            ASSERT(buffer == NULL);
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
                    buffer = list->list.tail;
                }
                else
                {
                    /* Pick the head buffer. */
                    buffer = list->list.head;
                }
            }

            /* There is data in the buffer so we must have a buffer. */
            ASSERT(buffer == NULL);

            /* There should not be a zero length buffer. */
            ASSERT(buffer->length == 0);

            /* Check if we need to do a partial read of this buffer. */
            if ((size + this_offset) >= buffer->length)
            {
                /* Only copy the number of bytes that can be copied from
                 * this buffer. */
                this_size = (buffer->length - this_offset);
            }
            else
            {
                /* Copy the given number of bytes. */
                this_size = size;
            }

            /* Pick the destination pointer. */
            to = (uint8_t *)data;

#ifdef LITTLE_ENDIAN
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
            ASSERT(fs_buffer_pull_offset(buffer, to, this_size, this_offset, flags) != SUCCESS);

            /* If there is no more valid data on this buffer. */
            if (buffer->length == 0)
            {
                /* We no longer need this buffer on our list. */
                ASSERT(sll_remove(&list->list, buffer, OFFSETOF(FS_BUFFER, next)) != buffer);

                /* Actively free this buffer. */
                fs_buffer_add(list->fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
            }

            /* Decrement the number of bytes we still need to copy. */
            size = (uint32_t)(size - this_size);

            /* If we are returning the data. */
            if ( (data != NULL)
#ifdef LITTLE_ENDIAN
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
                /* Decrement number of bytes we have left on this list. */
                list->total_length = (list->total_length - this_size);
            }

            /* If we are pulling data in place. */
            if (flags & FS_BUFFER_INPLACE)
            {
                /* Reset the offset as this will not be required for next buffers. */
                this_offset = 0;

                /* Pick the next buffer. */
                buffer = buffer->next;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_list_pull_offset */

/*
 * fs_buffer_list_push_offset
 * @list: Buffer list on which data is needed to be pushed.
 * @data: Data buffer to be pushed on buffer list.
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
 * This function will add data to the buffer list.
 */
int32_t fs_buffer_list_push_offset(FS_BUFFER_LIST *list, void *data, uint32_t size, uint8_t offset, uint8_t flags)
{
    int32_t status = SUCCESS;
    FS_BUFFER *buffer = NULL;
    uint32_t this_size, num_buffers, this_offset = offset;
    FS_BUFFER_DATA *buffer_data = ((FS *)list->fd)->buffer;
#ifdef LITTLE_ENDIAN
    uint8_t reverse = (uint8_t)(((flags & FS_BUFFER_PACKED) != 0) ^ (((flags & FS_BUFFER_HEAD) != 0) && ((flags & FS_BUFFER_UPDATE) == 0) && (offset == 0)));
#endif
    uint8_t *from;
#ifdef FS_BUFFER_DEBUG
    uint8_t should_not_fail = FALSE;
#endif /* FS_BUFFER_DEBUG */

    /* Should never happen. */
    ASSERT(data == NULL);

    /* If an offset was given. */
    if (offset != 0)
    {
        /* We should be updating the existing data. */
        ASSERT((flags & FS_BUFFER_UPDATE) == 0);
    }

    /* If we are updating the existing data. */
    if (flags & FS_BUFFER_UPDATE)
    {
        /* The buffer should already have the data we need to update. */
        ASSERT(list->total_length < (size + offset));

        /* If we are not updating the data on the head. */
        if ((flags & FS_BUFFER_HEAD) == 0)
        {
            /* Adjust the offset from the start. */
            this_offset = (list->total_length - (size + this_offset));

            /* Set the head flag. */
            flags |= FS_BUFFER_HEAD;
        }

        /* Pick the head buffer. */
        buffer = list->list.head;

        /* While we have an offset and nothing is needed from this buffer. */
        while ((this_offset > 0) && (buffer != NULL) && (this_offset >= buffer->length))
        {
            /* Remove data for this buffer from the offset. */
            this_offset -= buffer->length;

            /* Pick the next buffer. */
            buffer = buffer->next;
        }

        /* If we don't have a buffer. */
        if (buffer == NULL)
        {
            /* There is no space in this buffer. */
            status = FS_BUFFER_NO_SPACE;
        }
    }

    /* If we have some data to copy. */
    if ((status == SUCCESS) && (size > 0))
    {
        /* Reset the required number of buffers. */
        num_buffers = 0;

        /* If we need to push data on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* Pick the head buffer. */
                buffer = list->list.head;
                this_size =  0;

                /* If we have a buffer and there is some space on it. */
                if ((buffer) && (FS_BUFFER_SPACE(buffer) > 0))
                {
                    /* Pick the number of bytes we can copy on this buffer. */
                    this_size = FS_BUFFER_SPACE(buffer);

                    /* If we have more space then we require. */
                    if (this_size > size)
                    {
                        /* Copy the required amount of data. */
                        this_size = size;
                    }
                }

                /* If we have more data to copy. */
                if ((size - this_size) > 0)
                {
                    /* Calculate the required number of buffers. */
                    num_buffers += CEIL_DIV((size - this_size), buffer_data->buffer_size);
                }
            }
        }

        else
        {
            /* Pick the tail buffer. */
            buffer = list->list.tail;
            this_size =  0;

            /* If we have a buffer and there is some space on it. */
            if ((buffer) && (FS_BUFFER_TAIL_ROOM(buffer) > 0))
            {
                /* Pick the number of bytes we can copy on this buffer. */
                this_size = FS_BUFFER_TAIL_ROOM(buffer);

                /* If we have more space then we require. */
                if (this_size > size)
                {
                    /* Copy the required amount of data. */
                    this_size = size;
                }
            }

            /* If we have more data to copy. */
            if ((size - this_size) > 0)
            {
                /* Calculate the required number of buffers. */
                num_buffers += CEIL_DIV((size - this_size), buffer_data->buffer_size);
            }
        }

        /* If we will need to allocate buffers. */
        if (num_buffers > 0)
        {
            /* Check if we don't have the required number of buffers. */
            if (buffer_data->free_buffers.buffers < (((flags & FS_BUFFER_TH) ? buffer_data->threshold_buffers : 0) + num_buffers))
            {
                /* If we can suspend on buffers. */
                if (flags & FS_BUFFER_SUSPEND)
                {
                    /* Suspend to wait for buffers to become available. */
                    status = fs_buffer_suspend(list->fd, FS_BUFFER_FREE, num_buffers, flags);
                }
                else
                {
                    /* There is no space on the file descriptor. */
                    status = FS_BUFFER_NO_SPACE;
                }
            }
        }

        /* If required buffers were successfully allocated. */
        if (status == SUCCESS)
        {
#ifdef FS_BUFFER_DEBUG
            /* If we would wait for the buffer. */
            if (flags & FS_BUFFER_SUSPEND)
            {
                /* We should never fail. */
                should_not_fail = TRUE;
            }
#endif /* FS_BUFFER_DEBUG */

            /* Never suspend on buffers afterwards. */
            flags &= (uint8_t)~(FS_BUFFER_SUSPEND);
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
                buffer = list->list.head;

                /* Either we don't have a buffer in the buffer or there is no
                 * space on the head buffer. */
                if ((buffer == NULL) || (FS_BUFFER_SPACE(buffer) == 0))
                {
                    /* Need to allocate a new buffer to be pushed on the head. */
                    buffer = fs_buffer_get(list->fd, FS_BUFFER_FREE, flags);

                    /* If a buffer was allocated. */
                    if (buffer)
                    {
                        /* Use all the space in this buffer as head room. */
                        ASSERT(fs_buffer_add_head(buffer, buffer->length) != SUCCESS);

                        /* Add this buffer on the head of the buffer. */
                        sll_push(&list->list, buffer, OFFSETOF(FS_BUFFER, next));
                    }
                }

                if (buffer != NULL)
                {
                    /* Pick the number of bytes we can copy on this buffer. */
                    this_size = FS_BUFFER_SPACE(buffer);

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
                if ((this_size + this_offset) > buffer->length)
                {
                    /* Copy the data that can be copied in this buffer. */
                    this_size = (buffer->length - this_offset);
                }
            }
        }
        else
        {
            /* Pick the tail buffer. */
            buffer = list->list.tail;

            /* We should not be updating the existing value. */
            ASSERT(flags & FS_BUFFER_UPDATE);

            /* Either we don't have a buffer in the or there is no space
             * in the tail buffer. */
            if ((buffer == NULL) || (FS_BUFFER_TAIL_ROOM(buffer) == 0))
            {
                /* Need to allocate a new buffer to be appended on the tail. */
                buffer = fs_buffer_get(list->fd, FS_BUFFER_FREE, flags);

                /* If a buffer was allocated. */
                if (buffer)
                {
                    /* Append this buffer at the end of buffer. */
                    sll_append(&list->list, buffer, OFFSETOF(FS_BUFFER, next));
                }
            }

            if (buffer != NULL)
            {
                /* Pick the number of bytes we can copy on this buffer. */
                this_size = FS_BUFFER_TAIL_ROOM(buffer);

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

#ifdef LITTLE_ENDIAN
            /* If we need to copy data MSB first. */
            if ((data != NULL) && (reverse == TRUE))
            {
                /* Move to the offset at the end of the provided buffer. */
                from += (size - this_size);
            }
#endif

            /* Push data on the buffer we have selected. */
            ASSERT(fs_buffer_push_offset(buffer, from, this_size, this_offset, flags) != SUCCESS);

            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* Update the buffer size. */
                list->total_length += this_size;
            }

            /* Decrement the bytes we have copied in this go. */
            size = size - this_size;

            /* If we are copying data normally. */
            if ( (data != NULL)
#ifdef LITTLE_ENDIAN
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

                /* Pick the next buffer. */
                buffer = buffer->next;
            }
        }
    }

#ifdef FS_BUFFER_DEBUG
    /* If we failed. */
    if (status != SUCCESS)
    {
        /* Test if we should not have failed. */
        ASSERT(should_not_fail);
    }
#endif

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_list_push_offset */

/*
 * fs_buffer_list_divide
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
int32_t fs_buffer_list_divide(FS_BUFFER_LIST *list, uint32_t flags, uint32_t data_len)
{
    FS_BUFFER *buffer, *new_buffer = NULL;
    FS_BUFFER_LIST *new_list;
    int32_t status = SUCCESS;
    uint32_t this_len = data_len;

    /* Should never happen. */
    ASSERT(list->total_length <= data_len);

    /* Find the buffer we need to divide. */
    buffer = list->list.head;

    while (buffer)
    {
        /* If remaining data length is less than the data in this buffer. */
        if (this_len < buffer->length)
        {
            /* Break out of this loop. */
            break;
        }

        /* Remove data length for this buffer. */
        this_len -= buffer->length;

        /* If we need zero bytes from next buffer. */
        if (this_len == 0)
        {
            /* The new buffer will be the next buffer. */
            new_buffer = buffer->next;

            /* Break out of this loop. */
            break;
        }

        /* Pick the next buffer. */
        buffer = buffer->next;
    }

    /* Should never happen. */
    ASSERT(buffer == NULL);

    /* Get a new buffer to store the remaining data for this buffer. */
    new_list = fs_buffer_get(list->fd, FS_LIST_FREE, flags);

    /* If we do have a buffer to store remaining data of this buffer. */
    if (new_list != NULL)
    {
        /* If we have to divide the buffer. */
        if (new_buffer == NULL)
        {
            /* Remove the extra data from this buffer to a new buffer. */
            ASSERT(fs_buffer_divide(list->fd, buffer, &new_buffer, flags, this_len) != SUCCESS);

            /* Initialize the new buffers. */
            new_buffer->next = buffer->next;
        }

        /* This will be last buffer in the original buffer. */
        buffer->next = NULL;

        /* Initialize the new buffer. */
        new_list->list.head = new_buffer;
        new_list->list.tail = list->list.tail;
        new_list->total_length = (list->total_length - data_len);

        /* Divide the original buffer. */
        list->list.tail = buffer;
        list->total_length = data_len;

        /* Put new buffer in the buffer chain. */
        list->next = new_list;
        new_list->next = NULL;
    }
    else
    {
        /* Return error to the caller. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_list_divide */

/*
 * fs_buffer_add_head
 * @buffer: File buffer needed to be updated.
 * @size: Size of head room needed to be left in the buffer.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will add a head room to the given buffer, if there is
 * already some data on the buffer it will be moved. If it already has some
 * head room it will be maintained.
 */
int32_t fs_buffer_add_head(FS_BUFFER *buffer, uint32_t size)
{
    int32_t status = SUCCESS;

    /* An empty buffer should not come here. */
    ASSERT(buffer->data == NULL);

    /* Validate that there is enough space on the buffer. */
    if (FS_BUFFER_SPACE(buffer) >= size)
    {
        /* Check if we have some data on the buffer. */
        if (buffer->length != 0)
        {
            /* Move the data to make room for head. */
            memmove(&buffer->buffer[size], buffer->buffer, buffer->length);
        }

        /* Update the buffer pointer. */
        buffer->buffer = buffer->buffer + size;
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_add_head */

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
 * This function will remove data from a given buffer. If given will also copy
 * the data in the provided buffer.
 */
int32_t fs_buffer_pull_offset(FS_BUFFER *buffer, void *data, uint32_t size, uint32_t offset, uint8_t flags)
{
    uint8_t *from;
    int32_t status = SUCCESS;

    /* If an offset was given. */
    if (offset != 0)
    {
        /* We should not be removing the data. */
        ASSERT((flags & FS_BUFFER_INPLACE) == 0);
    }

    /* Validate if we do have required amount of data on the buffer. */
    if (buffer->length >= (size + offset))
    {
        /* If we need to pull data from the tail. */
        if (flags & FS_BUFFER_TAIL)
        {
            /* Pick the data from the end of the buffer. */
            from = &buffer->buffer[buffer->length - (size + offset)];
        }
        else
        {
            /* Pick the data from the start of the buffer. */
            from = &buffer->buffer[offset];

            /* If we don't want data to be removed. */
            if ((flags & FS_BUFFER_INPLACE) == 0)
            {
                /* Advance the buffer pointer. */
                buffer->buffer += size;
            }
        }

        /* If we need to actually need to return the pulled data. */
        if (data != NULL)
        {
#ifdef LITTLE_ENDIAN
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
            buffer->length -= size;
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
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
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will add data in the buffer.
 */
int32_t fs_buffer_push_offset(FS_BUFFER *buffer, void *data, uint32_t size, uint32_t offset, uint8_t flags)
{
    int32_t status = SUCCESS;
    uint8_t *to;

    /* An empty buffer should not come here. */
    ASSERT(buffer->data == NULL);

    /* If an offset was given. */
    if (offset != 0)
    {
        /* We should be updating the existing data. */
        ASSERT((flags & FS_BUFFER_UPDATE) == 0);
    }

    /* If we do have enough space on the buffer. */
    if ( ((flags & FS_BUFFER_UPDATE) && (buffer->length >= (size + offset))) ||
         (((flags & FS_BUFFER_UPDATE) == 0) &&
          (((flags & FS_BUFFER_HEAD) && (FS_BUFFER_SPACE(buffer) >= size)) ||
           (((flags & FS_BUFFER_HEAD) == 0) && (FS_BUFFER_TAIL_ROOM(buffer) >= size)))) )
    {
        /* If we need to add head room on this buffer. */
        if ( ((flags & FS_BUFFER_UPDATE) == 0) && (flags & FS_BUFFER_HEAD) && (FS_BUFFER_HEAD_ROOM(buffer) < size))
        {
            /* Add required head room. */
            status = fs_buffer_add_head(buffer, (size - FS_BUFFER_HEAD_ROOM(buffer)));
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
                buffer->buffer -= size;
            }

            /* Pick the pointer at which we will be adding data. */
            to = buffer->buffer + offset;
        }

        else
        {
            /* If we are not updating the existing value. */
            if ((flags & FS_BUFFER_UPDATE) == 0)
            {
                /* We will be pushing data at the end of the buffer, no need to
                 * adjust for the offset. */
                to = &buffer->buffer[buffer->length];
            }
            else
            {
                /* We will be updating data at the tail, adjust for the offset
                 * and size of the data. */
                to = &buffer->buffer[buffer->length - (size + offset)];
            }
        }

        /* If we actually need to push some data. */
        if (data != NULL)
        {
#ifdef LITTLE_ENDIAN
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
            buffer->length += size;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_push_offset */

/*
 * fs_buffer_divide
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
int32_t fs_buffer_divide(FD fd, FS_BUFFER *buffer, FS_BUFFER **new_buffer, uint32_t flags, uint32_t data_len)
{
    FS_BUFFER *ret_buffer = NULL;
    int32_t status = SUCCESS;

    /* Should never happen. */
    ASSERT(data_len >= buffer->length);
    ASSERT(new_buffer == NULL);

    /* Allocate a free buffer. */
    ret_buffer = fs_buffer_get(fd, FS_BUFFER_FREE, flags);

    /* If a free buffer was allocated. */
    if (ret_buffer != NULL)
    {
        /* Check if the new buffer has enough space to copy the data. */
        ASSERT(ret_buffer->max_length < data_len);

        /* Set the number of bytes valid in this buffer. */
        ret_buffer->length = (buffer->length - data_len);

        /* Update the number of bytes valid in the new buffer. */
        buffer->length = data_len;

        /* Copy data from old buffer to the new buffer. */
        memcpy(ret_buffer->buffer, &buffer->buffer[data_len], ret_buffer->length);

        /* Return the new buffer. */
        *new_buffer = ret_buffer;
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
    return (fs_buffer_list_pull((FS_BUFFER_LIST *)buffer, data, size, ((uint8_t)flags)));

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
    return (fs_buffer_list_push((FS_BUFFER_LIST *)buffer, data, size, (FS_BUFFER_HEAD | (uint8_t)flags)));

} /* fs_buffer_hdr_push */

#endif /* CONFIG_FS */
