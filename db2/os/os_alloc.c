/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_alloc.c	10.1 (Sleepycat) 12/1/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"

/*
 * XXX
 * Correct for systems that return NULL when you allocate 0 bytes of memory.
 * There are several places in DB where we allocate the number of bytes held
 * by the key/data item, and it can be 0.  Correct here so that malloc never
 * returns a NULL for that reason (which behavior is permitted by ANSI).  We
 * could make these calls macros on non-Alpha architectures (that's where we
 * saw the problem), but it's probably not worth the autoconf complexity.
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
