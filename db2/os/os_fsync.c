/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_fsync.c	10.3 (Sleepycat) 10/25/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#endif

#include "db_int.h"

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
	return (__os_fsync(fd) ? errno : 0);
}
