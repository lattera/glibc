/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_config.c	10.9 (Sleepycat) 11/28/97";
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
imported extern void	*memset __P((void *, int, size_t));

/*
 * __db_jump --
 *	This list of interfaces that applications can replace.  In some
 *	cases, the user is permitted to replace the standard ANSI C or
 *	POSIX 1003.1 call, e.g., malloc or read.  In others, we provide
 *	a local interface to the functionality, e.g., __os_map.
 */
struct __db_jumptab __db_jump = {
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

int __db_tsl_spins;			/* DB_TSL_SPINS */

/*
 * db_jump_set --
 *	Replace functions for the DB package.
 */
int
db_jump_set(func, which)
	void *func;
	int which;
{
	switch (which) {
	case DB_FUNC_CALLOC:
		/*
		 * XXX
		 * Obsolete, calloc is no longer called by DB.
		 */
		 break;
	case DB_FUNC_CLOSE:
		__db_jump.db_close = (int (*) __P((int)))func;
		break;
	case DB_FUNC_DIRFREE:
		__db_jump.db_dirfree = (void (*) __P((char **, int)))func;
		break;
	case DB_FUNC_DIRLIST:
		__db_jump.db_dirlist =
		    (int (*) __P((const char *, char ***, int *)))func;
		break;
	case DB_FUNC_EXISTS:
		__db_jump.db_exists = (int (*) __P((const char *, int *)))func;
		break;
	case DB_FUNC_FREE:
		__db_jump.db_free = (void (*) __P((void *)))func;
		break;
	case DB_FUNC_FSYNC:
		__db_jump.db_fsync = (int (*) __P((int)))func;
		break;
	case DB_FUNC_IOINFO:
		__db_jump.db_ioinfo =
		    (int (*) __P((const char *, int, off_t *, off_t *)))func;
		break;
	case DB_FUNC_MALLOC:
		__db_jump.db_malloc = (void *(*) __P((size_t)))func;
		break;
	case DB_FUNC_MAP:
		__db_jump.db_map =
		    (int (*) __P((int, size_t, int, int, void **)))func;
		break;
	case DB_FUNC_OPEN:
		__db_jump.db_open = (int (*) __P((const char *, int, ...)))func;
		break;
	case DB_FUNC_READ:
		__db_jump.db_read =
		    (ssize_t (*) __P((int, void *, size_t)))func;
		break;
	case DB_FUNC_REALLOC:
		__db_jump.db_realloc = (void *(*) __P((void *, size_t)))func;
		break;
	case DB_FUNC_SEEK:
		__db_jump.db_seek =
		    (int (*) __P((int, size_t, db_pgno_t, u_long, int)))func;
		break;
	case DB_FUNC_SLEEP:
		__db_jump.db_sleep = (int (*) __P((u_long, u_long)))func;
		break;
	case DB_FUNC_STRDUP:
		__db_jump.db_strdup = (char *(*) __P((const char *)))func;
		break;
	case DB_FUNC_UNLINK:
		__db_jump.db_unlink = (int (*) __P((const char *)))func;
		break;
	case DB_FUNC_UNMAP:
		__db_jump.db_unmap = (int (*) __P((void *, size_t)))func;
		break;
	case DB_FUNC_WRITE:
		__db_jump.db_write =
		    (ssize_t (*) __P((int, const void *, size_t)))func;
		break;
	case DB_FUNC_YIELD:
		__db_jump.db_yield = (int (*) __P((void)))func;
		break;
	default:
		return (EINVAL);
	}
	return (0);
}

/*
 * db_value_set --
 *	Replace values for the DB package.
 */
int
db_value_set(value, which)
	int value, which;
{
	switch (which) {
	case DB_TSL_SPINS:
		if (value <= 0)
			return (EINVAL);
		__db_tsl_spins = value;
		break;
	default:
		return (EINVAL);
	}
	return (0);
}

/*
 * XXX
 * Correct for systems that return NULL when you allocate 0 bytes of memory.
 * There are several places in DB where we allocate the number of bytes held
 * by the key/data item, and it can be 0.  Correct here so that malloc never
 * returns a NULL for that reason.
 */
/*
 * __db_calloc --
 *	The calloc(3) function for DB.
 *
 * PUBLIC: void *__db_calloc __P((size_t, size_t));
 */
void *
__db_calloc(num, size)
	size_t num, size;
{
	void *p;

	size *= num;
	if ((p = __db_jump.db_malloc(size == 0 ? 1 : size)) != NULL)
		memset(p, 0, size);
	return (p);
}

/*
 * __db_malloc --
 *	The malloc(3) function for DB.
 *
 * PUBLIC: void *__db_malloc __P((size_t));
 */
void *
__db_malloc(size)
	size_t size;
{
	return (__db_jump.db_malloc(size == 0 ? 1 : size));
}

/*
 * __db_realloc --
 *	The realloc(3) function for DB.
 *
 * PUBLIC: void *__db_realloc __P((void *, size_t));
 */
void *
__db_realloc(ptr, size)
	void *ptr;
	size_t size;
{
	return (__db_jump.db_realloc(ptr, size == 0 ? 1 : size));
}
