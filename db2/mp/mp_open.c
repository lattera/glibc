/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_open.c	10.12 (Sleepycat) 7/6/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "common_ext.h"

/*
 * memp_open --
 *	Initialize and/or join a memory pool.
 */
int
memp_open(path, flags, mode, dbenv, retp)
	const char *path;
	int flags, mode;
	DB_ENV *dbenv;
	DB_MPOOL **retp;
{
	DB_MPOOL *dbmp;
	size_t cachesize;
	int ret;

	/* Validate arguments. */
#ifdef HAVE_SPINLOCKS
#define	OKFLAGS	(DB_CREATE | DB_MPOOL_PRIVATE | DB_NOMMAP | DB_THREAD)
#else
#define	OKFLAGS	(DB_CREATE | DB_MPOOL_PRIVATE | DB_NOMMAP)
#endif
	if ((ret = __db_fchk(dbenv, "memp_open", flags, OKFLAGS)) != 0)
		return (ret);

	/* Extract fields from DB_ENV structure. */
	cachesize = dbenv == NULL ? 0 : dbenv->mp_size;

	/* Create and initialize the DB_MPOOL structure. */
	if ((dbmp = (DB_MPOOL *)calloc(1, sizeof(DB_MPOOL))) == NULL)
		return (ENOMEM);
	LOCKINIT(dbmp, &dbmp->mutex);
	LIST_INIT(&dbmp->dbregq);
	TAILQ_INIT(&dbmp->dbmfq);

	dbmp->dbenv = dbenv;

	/* Decide if it's possible for anyone else to access the pool. */
	if ((dbenv == NULL && path == NULL) ||
	    (dbenv != NULL && F_ISSET(dbenv, DB_MPOOL_PRIVATE)))
		F_SET(dbmp, MP_ISPRIVATE);

	/*
	 * Map in the region.  We do locking regardless, as portions of it are
	 * implemented in common code (if we put the region in a file, that is).
	 */
	F_SET(dbmp, MP_LOCKREGION);
	if ((ret = __memp_ropen(dbmp, path, cachesize, mode, flags)) != 0)
		goto err;
	F_CLR(dbmp, MP_LOCKREGION);

	/*
	 * If there's concurrent access, then we have to lock the region.
	 * If it's threaded, then we have to lock both the handles and the
	 * region.
	 */
	if (!F_ISSET(dbmp, MP_ISPRIVATE))
		F_SET(dbmp, MP_LOCKREGION);
	if (LF_ISSET(DB_THREAD))
		F_SET(dbmp, MP_LOCKHANDLE | MP_LOCKREGION);

	*retp = dbmp;
	return (0);

err:	if (dbmp != NULL)
		FREE(dbmp, sizeof(DB_MPOOL));
	return (ret);
}

/*
 * memp_close --
 *	Close a memory pool.
 */
int
memp_close(dbmp)
	DB_MPOOL *dbmp;
{
	DB_MPOOLFILE *dbmfp;
	DB_MPREG *mpreg;
	int ret, t_ret;

	ret = 0;

	/* Discard DB_MPREGs. */
	while ((mpreg = LIST_FIRST(&dbmp->dbregq)) != NULL) {
		LIST_REMOVE(mpreg, q);
		FREE(mpreg, sizeof(DB_MPREG));
	}

	/* Discard DB_MPOOLFILEs. */
	while ((dbmfp = TAILQ_FIRST(&dbmp->dbmfq)) != NULL)
		if ((t_ret = memp_fclose(dbmfp)) != 0 && ret == 0)
			ret = t_ret;

	/* Close the region. */
	if ((t_ret = __memp_rclose(dbmp)) && ret == 0)
		ret = t_ret;

	/* Free the structure. */
	FREE(dbmp, sizeof(DB_MPOOL));

	return (ret);
}

/*
 * memp_unlink --
 *	Exit a memory pool.
 */
int
memp_unlink(path, force, dbenv)
	const char *path;
	int force;
	DB_ENV *dbenv;
{
	return (__db_runlink(dbenv,
	    DB_APP_NONE, path, DB_DEFAULT_MPOOL_FILE, force));
}

/*
 * memp_register --
 *	Register a file type's pgin, pgout routines.
 */
int
memp_register(dbmp, ftype, pgin, pgout)
	DB_MPOOL *dbmp;
	int ftype;
	int (*pgin) __P((db_pgno_t, void *, DBT *));
	int (*pgout) __P((db_pgno_t, void *, DBT *));
{
	DB_MPREG *mpr;

	if ((mpr = (DB_MPREG *)malloc(sizeof(DB_MPREG))) == NULL)
		return (ENOMEM);

	mpr->ftype = ftype;
	mpr->pgin = pgin;
	mpr->pgout = pgout;

	/*
	 * Insert at the head.  Because we do a linear walk, we'll find
	 * the most recent registry in the case of multiple entries, so
	 * we don't have to check for multiple registries.
	 */
	LOCKHANDLE(dbmp, &dbmp->mutex);
	LIST_INSERT_HEAD(&dbmp->dbregq, mpr, q);
	UNLOCKHANDLE(dbmp, &dbmp->mutex);

	return (0);
}
