/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_abs.c	10.5 (Sleepycat) 7/5/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <string.h>
#endif

#include "db_int.h"
#include "os_ext.h"

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
#ifdef _WIN32
	/*
	 * !!!
	 * Check for drive specifications, e.g., "C:".  In addition, the path
	 * separator used by the win32 DB (PATH_SEPARATOR) is \; look for both
	 * / and \ since these are user-input paths.
	 */
	if (isalpha(path[0]) && path[1] == ':')
		path += 2;
	return (path[0] == '/' || path[0] == '\\');
#else
#ifdef macintosh
	/*
	 * !!!
	 * Absolute pathnames always start with a volume name, which must be
	 * followed by a colon, thus they are of the form:
	 *	volume: or volume:dir1:dir2:file
	 *
	 * Relative pathnames are either a single name without colons or a
	 * path starting with a colon, thus of the form:
	 *	file or :file or :dir1:dir2:file
	 */
	return (strchr(path, ':') != NULL && path[0] != ':');
#else
	return (path[0] == '/');
#endif
#endif
}

/*
 * __db_rpath --
 *	Return the last path separator in the path or NULL if none found.
 *
 * PUBLIC: char *__db_rpath __P((const char *));
 */
char *
__db_rpath(path)
	const char *path;
{
	const char *s, *last;

	last = NULL;
	if (PATH_SEPARATOR[1] != '\0') {
		for (s = path; s[0] != '\0'; ++s)
			if (strchr(PATH_SEPARATOR, s[0]) != NULL)
				last = s;
	} else
		for (s = path; s[0] != '\0'; ++s)
			if (s[0] == PATH_SEPARATOR[0])
				last = s;
	return ((char *)last);
}
