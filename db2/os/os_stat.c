/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_stat.c	10.11 (Sleepycat) 1/8/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "common_ext.h"

/*
 * __os_exists --
 *	Return if the file exists.
 *
 * PUBLIC: int __os_exists __P((const char *, int *));
 */
int
__os_exists(path, isdirp)
	const char *path;
	int *isdirp;
{
	struct stat sb;

	if (stat(path, &sb) != 0)
		return (errno);
	if (isdirp != NULL)
		*isdirp = S_ISDIR(sb.st_mode);
	return (0);
}

/*
 * __os_ioinfo --
 *	Return file size and I/O size; abstracted to make it easier
 *	to replace.
 *
 * PUBLIC: int __os_ioinfo
 * PUBLIC:    __P((const char *, int, u_int32_t *, u_int32_t *, u_int32_t *));
 */
int
__os_ioinfo(path, fd, mbytesp, bytesp, iosizep)
	const char *path;
	int fd;
	u_int32_t *mbytesp, *bytesp, *iosizep;
{
	struct stat sb;

	COMPQUIET(path, NULL);

	if (fstat(fd, &sb) == -1)
		return (errno);

	/* Return the size of the file. */
	if (mbytesp != NULL)
		*mbytesp = sb.st_size / MEGABYTE;
	if (bytesp != NULL)
		*bytesp = sb.st_size % MEGABYTE;

	/* Return the underlying filesystem blocksize, if available. */
#ifdef HAVE_ST_BLKSIZE
	if (iosizep != NULL)
		*iosizep = sb.st_blksize;
#else
	if (iosizep != NULL)
		*iosizep = DB_DEF_IOSIZE;
#endif
	return (0);
}
