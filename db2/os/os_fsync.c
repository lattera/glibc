/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_fsync.c	10.7 (Sleepycat) 10/12/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>			/* XXX: Required by __hp3000s900 */
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_jump.h"

#ifdef __hp3000s900
int
__mpe_fsync(fd)
	int fd;
{
	extern FCONTROL(short, short, void *);

	FCONTROL(_MPE_FILENO(fd), 2, NULL);	/* Flush the buffers */
	FCONTROL(_MPE_FILENO(fd), 6, NULL);	/* Write the EOF */
	return (0);
}
#endif

#ifdef __hp3000s900
#define	fsync(fd)	__mpe_fsync(fd);
#endif
#ifdef _WIN32
#define	fsync(fd)	_commit(fd);
#endif

/*
 * __os_fsync --
 *	Flush a file descriptor.
 *
 * PUBLIC: int __os_fsync __P((int));
 */
int
__os_fsync(fd)
	int fd;
{
	int ret;

	ret = __db_jump.j_fsync != NULL ?  __db_jump.j_fsync(fd) : fsync(fd);
	return (ret == 0 ? 0 : errno);
}
