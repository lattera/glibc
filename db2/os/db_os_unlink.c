/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_unlink.c	10.2 (Sleepycat) 6/28/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_ext.h"

/*
 * __db_unlink --
 *	Remove a file.
 *
 * PUBLIC: int __db_unlink __P((const char *));
 */
int
__db_unlink(path)
	const char *path;
{
	return (unlink(path) == -1 ? errno : 0);
}
