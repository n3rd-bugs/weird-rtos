/*
 * ffsync.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <ffsync.h>

#ifdef FS_FAT
#if (_FS_REENTRANT == TRUE)
/*
 * ff_cre_syncobj
 * @vol: Volume for which this object is needed to be created.
 * @sobj: Synchronization object needed to be created.
 * @return: FFSYNC_SUCCESS will be returned if object was successfully created,
 *  otherwise an error will be returned.
 * This function is responsible for creating a synchronization object.
 */
int ff_cre_syncobj(BYTE vol, _SYNC_t* sobj)
{
    /* Remove some compiler warning. */
    UNUSED_PARAM(vol);

    /* Create a semaphore for synchronization. */
    semaphore_create(sobj, 1);

    /* Return status to the caller. */
    return (FFSYNC_SUCCESS);

} /* ff_cre_syncobj */

/*
 * ff_req_grant
 * @sobj: Synchronization object needed to be acquired.
 * @return: FFSYNC_SUCCESS will be returned if object was successfully acquired,
 *  otherwise FFSYNC_ERROR will be returned.
 * This function is responsible for acquiring a synchronization object.
 */
int ff_req_grant(_SYNC_t *sobj)
{
    int status = FFSYNC_SUCCESS;

    /* Acquire the synchronization object. */
    if (semaphore_obtain(sobj, _FS_TIMEOUT) != SUCCESS)
    {
        /* Return error to the caller. */
        status = FFSYNC_ERROR;
    }

    /* Return status to the caller. */
    return (status);

} /* ff_req_grant */

/*
 * ff_rel_grant
 * @sobj: Synchronization object needed to be released.
 * This function is responsible for releasing a synchronization object.
 */
void ff_rel_grant(_SYNC_t *sobj)
{
    /* Release the synchronization object. */
    semaphore_release(sobj);

} /* ff_rel_grant */

/*
 * ff_del_syncobj
 * @sobj: Synchronization object needed to be released.
 * @return: FFSYNC_SUCCESS will be returned if synchronization object was
 *  successfully destroyed.
 * This function is responsible for deleting a synchronization object.
 */
int ff_del_syncobj(_SYNC_t *sobj)
{
    /* Destroy this synchronization object. */
    semaphore_destroy(sobj);

    /* Return status to the caller. */
    return (FFSYNC_SUCCESS);

} /* ff_del_syncobj */

#endif /* _FS_REENTRANT */
#endif /* FS_FAT */
