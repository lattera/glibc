/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_unlink.c	10.7 (Sleepycat) 10/12/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * __os_unlink --
 *	Remove a file.
 *
 * PUBLIC: int __os_unlink __P((const char *));
 */
int
__os_unlink(path)
	const char *path;
{
	int ret;

	ret = __db_jump.j_unlink != NULL ?
	    __db_jump.j_unlink(path) : unlink(path);
	return (ret == -1 ? errno : 0);
}
