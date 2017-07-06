/*
 * path.c
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
#include <kernel.h>
#include <path.h>

/*
 * util_path_match
 * @required: Path that we need to match with.
 * @path: Pointer to path that is needed to be matched.
 * @return: TRUE if given path matches.
 * This function matches two paths. On return the path will be updated to
 * the point where paths were matched.
 */
uint32_t util_path_match(const char *requires, char **path)
{
    char *tmp = *path;
    uint32_t match = FALSE;

    /* Match the given path. */
    while ( (*requires) && (*tmp) && (*requires == *tmp) )
    {
        requires++;
        tmp++;
    }

    /* If we two paths match. */
    if ( ( (*requires == '\0') && ((*tmp == '\0') || (*tmp == '\\')) ) ||
         (*tmp == '*') )
    {
        /* Update the path pointer. */
        *path = tmp;

        /* We were able to match the path. */
        match  = TRUE;
    }

    /* Path did not match. */
    return (match);

} /* util_path_match */
