/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_stat.c	10.6 (Sleepycat) 7/2/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "os_ext.h"
#include "common_ext.h"

/*
 * __db_exists --
 *	Return if the file exists.
 *
 * PUBLIC: int __db_exists __P((const char *, int *));
 */
int
__db_exists(path, isdirp)
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
 * __db_stat --
 *	Return file size and I/O size; abstracted to make it easier
 *	to replace.
 *
 * PUBLIC: int __db_stat __P((DB_ENV *, const char *, int, off_t *, off_t *));
 */
int
__db_stat(dbenv, path, fd, sizep, iop)
	DB_ENV *dbenv;
	const char *path;
	int fd;
	off_t *sizep, *iop;
{
	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		__db_err(dbenv, "%s: fstat: %s", path, strerror(errno));
		return (errno);
	}

	/* Return the size of the file. */
	if (sizep != NULL)
		*sizep = sb.st_size;

	/*
	 * Return the underlying filesystem blocksize, if available.  Default
	 * to 8K on the grounds that most OS's use less than 8K as their VM
	 * page size.
	 */
#ifdef HAVE_ST_BLKSIZE
	if (iop != NULL)
		*iop = sb.st_blksize;
#else
	if (iop != NULL)
		*iop = 8 * 1024;
#endif
	return (0);
}
