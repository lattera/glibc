/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)hash_stat.c	10.12 (Sleepycat) 12/19/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "hash.h"

/*
 * __ham_stat --
 *	Gather/print the hash statistics
 *
 * PUBLIC: int __ham_stat __P((DB *, void *, void *(*)(size_t), u_int32_t));
 */
int
__ham_stat(dbp, spp, db_malloc, flags)
	DB *dbp;
	void *spp;
	void *(*db_malloc) __P((size_t));
	u_int32_t flags;
{
	COMPQUIET(spp, NULL);
	COMPQUIET(db_malloc, NULL);
	COMPQUIET(flags, 0);

	DB_PANIC_CHECK(dbp);

	return (__db_eopnotsup(dbp->dbenv));
}
