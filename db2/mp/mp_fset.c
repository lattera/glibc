/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_fset.c	10.9 (Sleepycat) 9/20/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "common_ext.h"

/*
 * memp_fset --
 *	Mpool page set-flag routine.
 */
int
memp_fset(dbmfp, pgaddr, flags)
	DB_MPOOLFILE *dbmfp;
	void *pgaddr;
	int flags;
{
	BH *bhp;
	DB_MPOOL *dbmp;
	int ret;

	dbmp = dbmfp->dbmp;

	/* Validate arguments. */
	if (flags != 0) {
		if ((ret = __db_fchk(dbmp->dbenv, "memp_fset", flags,
		    DB_MPOOL_DIRTY | DB_MPOOL_CLEAN | DB_MPOOL_DISCARD)) != 0)
			return (ret);
		if ((ret = __db_fcchk(dbmp->dbenv, "memp_fset",
		    flags, DB_MPOOL_CLEAN, DB_MPOOL_DIRTY)) != 0)
			return (ret);

		if (LF_ISSET(DB_MPOOL_DIRTY) && F_ISSET(dbmfp, MP_READONLY)) {
			__db_err(dbmp->dbenv,
			    "%s: dirty flag set for readonly file page",
			    dbmfp->path);
			return (EACCES);
		}
	}

	/* Convert the page address to a buffer header. */
	bhp = (BH *)((u_int8_t *)pgaddr - SSZA(BH, buf));

	LOCKREGION(dbmp);

	if (LF_ISSET(DB_MPOOL_DIRTY))
		F_SET(bhp, BH_DIRTY);
	if (LF_ISSET(DB_MPOOL_CLEAN))
		F_CLR(bhp, BH_DIRTY);
	if (LF_ISSET(DB_MPOOL_DISCARD))
		F_SET(bhp, BH_DISCARD);

	UNLOCKREGION(dbmp);
	return (0);
}
