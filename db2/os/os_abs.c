/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_abs.c	10.7 (Sleepycat) 10/24/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"

/*
 * __db_abspath --
 *	Return if a path is an absolute path.
 *
 * PUBLIC: int __db_abspath __P((const char *));
 */
int
__db_abspath(path)
	const char *path;
{
	return (path[0] == '/');
}
