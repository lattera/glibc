/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_oflags.c	10.2 (Sleepycat) 10/24/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <fcntl.h>
#endif

#include "db_int.h"

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
