/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_open.c	10.14 (Sleepycat) 7/5/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_ext.h"

/*
 * __db_oflags --
 *	Convert open(2) flags to DB flags.
 *
 * PUBLIC: int __db_oflags __P((int));
 */
int
__db_oflags(oflags)
	int oflags;
{
	int dbflags;

	/*
	 * XXX
	 * Convert POSIX 1003.1 open(2) flags to DB flags.  Not an exact
	 * science as most POSIX implementations don't have a flag value
	 * for O_RDONLY, it's simply the lack of a write flag.
	 */
	dbflags = 0;
	if (oflags & O_CREAT)
		dbflags |= DB_CREATE;
	if (!(oflags & (O_RDWR | O_WRONLY)) || oflags & O_RDONLY)
		dbflags |= DB_RDONLY;
	if (oflags & O_TRUNC)
		dbflags |= DB_TRUNCATE;
	return (dbflags);
}

/*
 * __db_fdopen --
 *	Open a file descriptor.
 *
 * PUBLIC: int __db_fdopen __P((const char *, int, int, int, int *));
 */
int
__db_fdopen(name, arg_flags, ok_flags, mode, fdp)
	const char *name;
	int arg_flags, ok_flags, mode, *fdp;
{
	int fd, flags;

	if (arg_flags & ~ok_flags)
		return (EINVAL);

	flags = 0;
	if (arg_flags & DB_CREATE)
		flags |= O_CREAT;

	if (arg_flags & DB_EXCL)
		flags |= O_EXCL;

	if (arg_flags & DB_RDONLY)
		flags |= O_RDONLY;
	else
		flags |= O_RDWR;

#ifdef _WIN32
#ifdef _MSC_VER
	if (arg_flags & DB_SEQUENTIAL)
		flags |= _O_SEQUENTIAL;
	else
		flags |= _O_RANDOM;

	if (arg_flags & DB_TEMPORARY)
		flags |= _O_TEMPORARY;
#endif
	flags |= O_BINARY | O_NOINHERIT;
#endif

	if (arg_flags & DB_TRUNCATE)
		flags |= O_TRUNC;

	/* Open the file. */
	if ((fd = open(name, flags, mode)) == -1)
		return (errno);

#ifndef _WIN32
	/* Delete any temporary file; done for Win32 by _O_TEMPORARY. */
	if (arg_flags & DB_TEMPORARY)
		(void)unlink(name);
#endif

#if !defined(_WIN32) && !defined(macintosh)
	/*
	 * Deny access to any child process; done for Win32 by O_NOINHERIT,
	 * MacOS has neither child processes nor fd inheritance.
	 */
	if (fcntl(fd, F_SETFD, 1) == -1) {
		int ret = errno;

		(void)__db_close(fd);
		return (ret);
	}
#endif
	*fdp = fd;
	return (0);
}

/*
 * __db_fsync --
 *	Flush a file descriptor.
 *
 * PUBLIC: int __db_fsync __P((int));
 */
int
__db_fsync(fd)
	int fd;
{
	return (fsync(fd) ? errno : 0);
}

/*
 * __db_close --
 *	Close a file descriptor.
 *
 * PUBLIC: int __db_close __P((int));
 */
int
__db_close(fd)
	int fd;
{
	return (close(fd) ? errno : 0);
}
