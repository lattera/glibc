/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_sync.c	10.9 (Sleepycat) 8/29/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "common_ext.h"

/*
 * memp_sync --
 *	Mpool sync function.
 */
int
memp_sync(dbmp, lsnp)
	DB_MPOOL *dbmp;
	DB_LSN *lsnp;
{
	BH *bhp;
	DB_ENV *dbenv;
	MPOOL *mp;
	MPOOLFILE *mfp;
	int can_write, wrote, lsn_cnt, restart, ret;

	dbenv = dbmp->dbenv;

	if (dbmp->dbenv->lg_info == NULL) {
		__db_err(dbenv, "memp_sync requires logging");
		return (EINVAL);
	}

	LOCKREGION(dbmp);

	/*
	 * If the application is asking about a previous call, and we haven't
	 * found any buffers that the application holding the pin couldn't
	 * write, return yes or no based on the current count.  Note, if the
	 * application is asking about a LSN *smaller* than one we've already
	 * handled, then we return based on the count for that LSN.
	 */
	mp = dbmp->mp;
	if (!F_ISSET(mp, MP_LSN_RETRY) && log_compare(lsnp, &mp->lsn) <= 0) {
		if (mp->lsn_cnt == 0) {
			*lsnp = mp->lsn;
			ret = 0;
		} else
			ret = DB_INCOMPLETE;

		UNLOCKREGION(dbmp);
		return (ret);
	}

	/* Else, it's a new checkpoint. */
	F_CLR(mp, MP_LSN_RETRY);

	/*
	 * Save the LSN.  We know that it's a new LSN or larger than the one
	 * for which we were already doing a checkpoint.  (BTW, I don't expect
	 * to see multiple LSN's from the same or multiple processes, but You
	 * Just Never Know.  Responding as if they all called with the largest
	 * of the LSNs specified makes everything work.
	 *
	 * We don't currently use the LSN we save.  We could potentially save
	 * the last-written LSN in each buffer header and use it to determine
	 * what buffers need to be written.  The problem with this is that it's
	 * sizeof(LSN) more bytes of buffer header.  We currently write all the
	 * dirty buffers instead.
	 *
	 * Walk the list of shared memory segments clearing the count of
	 * buffers waiting to be written.
	 */
	mp->lsn = *lsnp;
	mp->lsn_cnt = 0;
	for (mfp = SH_TAILQ_FIRST(&dbmp->mp->mpfq, __mpoolfile);
	    mfp != NULL; mfp = SH_TAILQ_NEXT(mfp, q, __mpoolfile))
		mfp->lsn_cnt = 0;

	/*
	 * Walk the list of buffers and mark all dirty buffers to be written
	 * and all pinned buffers to be potentially written.  We do this in
	 * single fell swoop while holding the region locked so that processes
	 * can't make new buffers dirty, causing us to never finish.  Since
	 * the application may have restarted the sync, clear any BH_WRITE
	 * flags that appear to be left over.
	 */
	can_write = lsn_cnt = 0;
	for (lsn_cnt = 0, bhp = SH_TAILQ_FIRST(&mp->bhq, __bh);
	    bhp != NULL; bhp = SH_TAILQ_NEXT(bhp, q, __bh))
		if (F_ISSET(bhp, BH_DIRTY) || bhp->ref != 0) {
			F_SET(bhp, BH_WRITE);

			if (bhp->ref == 0)
				can_write = 1;

			mfp = ADDR(dbmp, bhp->mf_offset);
			++mfp->lsn_cnt;

			++lsn_cnt;
		} else
			F_CLR(bhp, BH_WRITE);

	mp->lsn_cnt = lsn_cnt;

	/* If there no buffers we can write, we're done. */
	if (!can_write) {
		UNLOCKREGION(dbmp);
		return (mp->lsn_cnt ? DB_INCOMPLETE : 0);
	}

	/*
	 * Write any buffers that we can.  Restart the walk after each write,
	 * __memp_pgwrite() discards and reacquires the region lock during I/O.
	 */
retry:	for (bhp = SH_TAILQ_FIRST(&mp->bhq, __bh);
	    bhp != NULL; bhp = SH_TAILQ_NEXT(bhp, q, __bh)) {
		/* Ignore pinned or locked buffers. */
		if (!F_ISSET(bhp, BH_WRITE) ||
		    bhp->ref != 0 || F_ISSET(bhp, BH_LOCKED))
			continue;

		mfp = ADDR(dbmp, bhp->mf_offset);
		if ((ret =
		    __memp_bhwrite(dbmp, mfp, bhp, &restart, &wrote)) != 0)
			goto err;
		if (wrote) {
			if (restart)
				goto retry;
			continue;
		}
		__db_err(dbenv, "%s: unable to flush page: %lu",
		    ADDR(dbmp, mfp->path_off), (u_long)bhp->pgno);
		ret = EPERM;
		goto err;
	}
	ret = mp->lsn_cnt ? DB_INCOMPLETE : 0;

err:	UNLOCKREGION(dbmp);
	return (ret);
}

/*
 * memp_fsync --
 *	Mpool file sync function.
 */
int
memp_fsync(dbmfp)
	DB_MPOOLFILE *dbmfp;
{
	BH *bhp;
	DB_MPOOL *dbmp;
	size_t mf_offset;
	int pincnt, restart, ret, wrote;

	/*
	 * If this handle doesn't have a file descriptor that's open for
	 * writing, or if the file is a temporary, there's no reason to
	 * proceed further.
	 */
	if (F_ISSET(dbmfp, MP_READONLY | MP_PATH_TEMP))
		return (0);

	dbmp = dbmfp->dbmp;
	ret = 0;

	mf_offset = OFFSET(dbmp, dbmfp->mfp);

	LOCKREGION(dbmp);

	/*
	 * Walk the list of buffer headers for the MPOOLFILE, and write out any
	 * dirty buffers that we can.
	 */
retry:	pincnt = 0;
	for (bhp = SH_TAILQ_FIRST(&dbmp->mp->bhq, __bh);
	    bhp != NULL; bhp = SH_TAILQ_NEXT(bhp, q, __bh))
		if (F_ISSET(bhp, BH_DIRTY) && bhp->mf_offset == mf_offset) {
			if (bhp->ref != 0 || F_ISSET(bhp, BH_LOCKED)) {
				++pincnt;
				continue;
			}
			if ((ret =
			    __memp_pgwrite(dbmfp, bhp, &restart, &wrote)) != 0)
				goto err;
			if (!wrote)
				++pincnt;
			if (restart)
				goto retry;
		}

err:	UNLOCKREGION(dbmp);

	return (ret == 0 ? (pincnt ? DB_INCOMPLETE : 0) : ret);
}
