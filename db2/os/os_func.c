/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_func.c	10.4 (Sleepycat) 10/28/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#endif

#include "db_int.h"

/*
 * XXX
 * We provide our own extern declarations so that we don't collide with
 * systems that get them wrong, e.g., SunOS.
 */
#ifdef _WIN32
#define fsync		_commit
#define imported	__declspec(dllimport)
#else
#define imported
#endif

imported extern void    *calloc __P((size_t, size_t));
imported extern int	 close __P((int));
imported extern void	 free __P((void *));
imported extern int	 fsync __P((int));
imported extern void    *malloc __P((size_t));
imported extern int	 open __P((const char *, int, ...));
imported extern ssize_t	 read __P((int, void *, size_t));
imported extern char	*strdup __P((const char *));
imported extern void    *realloc __P((void *, size_t));
imported extern int	 unlink __P((const char *));
imported extern ssize_t	 write __P((int, const void *, size_t));

/*
 * __db_jump --
 *	This list of interfaces that applications can replace.  In some
 *	cases, the user is permitted to replace the standard ANSI C or
 *	POSIX 1003.1 call, e.g., calloc or read.  In others, we provide
 *	a local interface to the functionality, e.g., __os_map.
 */
struct __db_jumptab __db_jump = {
	calloc,				/* DB_FUNC_CALLOC */
	close,				/* DB_FUNC_CLOSE */
	__os_dirfree,			/* DB_FUNC_DIRFREE */
	__os_dirlist,			/* DB_FUNC_DIRLIST */
	__os_exists,			/* DB_FUNC_EXISTS */
	free,				/* DB_FUNC_FREE */
	fsync,				/* DB_FUNC_FSYNC */
	__os_ioinfo,			/* DB_FUNC_IOINFO */
	malloc,				/* DB_FUNC_MALLOC */
	__os_map,			/* DB_FUNC_MAP */
	open,				/* DB_FUNC_OPEN */
	read,				/* DB_FUNC_READ */
	realloc,			/* DB_FUNC_REALLOC */
	__os_seek,			/* DB_FUNC_SEEK */
	__os_sleep,			/* DB_FUNC_SLEEP */
	strdup,				/* DB_FUNC_STRDUP */
	unlink,				/* DB_FUNC_UNLINK */
	__os_unmap,			/* DB_FUNC_UNMAP */
	write,				/* DB_FUNC_WRITE */
	NULL				/* DB_FUNC_YIELD */
};

/*
 * db_jump_set --
 *	Replace an interface.
 */
int
db_jump_set(func, which)
	void *func;
	int which;
{
	switch (which) {
	case DB_FUNC_CALLOC:
		__db_calloc = (void *(*) __P((size_t, size_t)))func;
		break;
	case DB_FUNC_CLOSE:
		__os_close = (int (*) __P((int)))func;
		break;
	case DB_FUNC_DIRFREE:
		__db_dirfree = (void (*) __P((char **, int)))func;
		break;
	case DB_FUNC_DIRLIST:
		__db_dirlist =
		    (int (*) __P((const char *, char ***, int *)))func;
		break;
	case DB_FUNC_EXISTS:
		__db_exists = (int (*) __P((const char *, int *)))func;
		break;
	case DB_FUNC_FREE:
		__db_free = (void (*) __P((void *)))func;
		break;
	case DB_FUNC_FSYNC:
		__os_fsync = (int (*) __P((int)))func;
		break;
	case DB_FUNC_IOINFO:
		__db_ioinfo =
		    (int (*) __P((const char *, int, off_t *, off_t *)))func;
		break;
	case DB_FUNC_MALLOC:
		__db_malloc = (void *(*) __P((size_t)))func;
		break;
	case DB_FUNC_MAP:
		__db_map = (int (*) __P((int, size_t, int, int, void **)))func;
		break;
	case DB_FUNC_OPEN:
		__os_open = (int (*) __P((const char *, int, ...)))func;
		break;
	case DB_FUNC_READ:
		__os_read = (ssize_t (*) __P((int, void *, size_t)))func;
		break;
	case DB_FUNC_REALLOC:
		__db_realloc = (void *(*) __P((void *, size_t)))func;
		break;
	case DB_FUNC_SEEK:
		__db_seek =
		    (int (*) __P((int, size_t, db_pgno_t, u_long, int)))func;
		break;
	case DB_FUNC_SLEEP:
		__db_sleep = (int (*) __P((u_long, u_long)))func;
		break;
	case DB_FUNC_STRDUP:
		__db_strdup = (char *(*) __P((const char *)))func;
		break;
	case DB_FUNC_UNLINK:
		__os_unlink = (int (*) __P((const char *)))func;
		break;
	case DB_FUNC_UNMAP:
		__db_unmap = (int (*) __P((void *, size_t)))func;
		break;
	case DB_FUNC_WRITE:
		__os_write = (ssize_t (*) __P((int, const void *, size_t)))func;
		break;
	case DB_FUNC_YIELD:
		__db_yield = (int (*) __P((void)))func;
		break;
	default:
		return (EINVAL);
	}
	return (0);
}
