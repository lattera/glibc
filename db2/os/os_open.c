/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_open.c	10.33 (Sleepycat) 10/12/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * __db_open --
 *	Open a file descriptor.
 *
 * PUBLIC: int __db_open __P((const char *, u_int32_t, u_int32_t, int, int *));
 */
int
__db_open(name, arg_flags, ok_flags, mode, fdp)
	const char *name;
	u_int32_t arg_flags, ok_flags;
	int mode, *fdp;
{
#if !defined(_WIN32) && defined(HAVE_SIGFILLSET)
	sigset_t set, oset;
#endif
	int flags, ret;

	if (arg_flags & ~ok_flags)
		return (EINVAL);

	flags = 0;

	/*
	 * DB requires the semantic that two files opened at the same time
	 * with O_CREAT and O_EXCL set will return failure in at least one.
	 */
	if (arg_flags & DB_CREATE)
		flags |= O_CREAT;

	if (arg_flags & DB_EXCL)
		flags |= O_EXCL;

	if (arg_flags & DB_RDONLY)
		flags |= O_RDONLY;
	else
		flags |= O_RDWR;

#if defined(_WIN32) || defined(WIN16)
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

#if !defined(_WIN32) && defined(HAVE_SIGFILLSET)
	/*
	 * We block every signal we can get our hands on so that the temporary
	 * file isn't left around if we're interrupted at the wrong time.  Of
	 * course, if we drop core in-between the calls we'll hang forever, but
	 * that's probably okay.  ;-)
	 */
	if (arg_flags & DB_TEMPORARY) {
		(void)sigfillset(&set);
		(void)sigprocmask(SIG_BLOCK, &set, &oset);
	}
#endif

	/* Open the file. */
	if ((ret = __os_open(name, flags, mode, fdp)) != 0)
		return (ret);

#if !defined(_WIN32)
	/* Delete any temporary file; done for Win32 by _O_TEMPORARY. */
	if (arg_flags & DB_TEMPORARY) {
		(void)__os_unlink(name);
#if defined(HAVE_SIGFILLSET)
		(void)sigprocmask(SIG_SETMASK, &oset, NULL);
#endif
	}
#endif

#if !defined(_WIN32) && !defined(WIN16) && !defined(VMS)
	/*
	 * Deny access to any child process.
	 *	VMS: does not have fd inheritance.
	 *	Win32: done by O_NOINHERIT.
	 */
	if (fcntl(*fdp, F_SETFD, 1) == -1) {
		ret = errno;

		(void)__os_close(*fdp);
		return (ret);
	}
#endif
	return (0);
}

/*
 * __os_open --
 *	Open a file.
 *
 * PUBLIC: int __os_open __P((const char *, int, int, int *));
 */
int
__os_open(name, flags, mode, fdp)
	const char *name;
	int flags, mode, *fdp;
{
	*fdp = __db_jump.j_open != NULL ?
	    __db_jump.j_open(name, flags, mode) : open(name, flags, mode);
	return (*fdp == -1 ? errno : 0);
}

/*
 * __os_close --
 *	Close a file descriptor.
 *
 * PUBLIC: int __os_close __P((int));
 */
int
__os_close(fd)
	int fd;
{
	int ret;

	ret = __db_jump.j_close != NULL ? __db_jump.j_close(fd) : close(fd);
	return (ret == 0 ? 0 : errno);
}
