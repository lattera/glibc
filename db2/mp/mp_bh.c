/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_bh.c	10.15 (Sleepycat) 8/29/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "common_ext.h"

static int __memp_upgrade __P((DB_MPOOL *, DB_MPOOLFILE *, MPOOLFILE *));

/*
 * __memp_bhwrite --
 *	Write the page associated with a given bucket header.
 *
 * PUBLIC: int __memp_bhwrite
 * PUBLIC:     __P((DB_MPOOL *, MPOOLFILE *, BH *, int *, int *));
 */
int
__memp_bhwrite(dbmp, mfp, bhp, restartp, wrotep)
	DB_MPOOL *dbmp;
	MPOOLFILE *mfp;
	BH *bhp;
	int *restartp, *wrotep;
{
	DBT dbt;
	DB_MPOOLFILE *dbmfp;
	DB_MPREG *mpreg;

	if (restartp != NULL)
		*restartp = 0;
	if (wrotep != NULL)
		*wrotep = 0;

	/*
	 * Walk the process' DB_MPOOLFILE list and find a file descriptor for
	 * the file.  We also check that the descriptor is open for writing.
	 * If we find a descriptor on the file that's not open for writing, we
	 * try and upgrade it to make it writeable.
	 */
	LOCKHANDLE(dbmp, &dbmp->mutex);
	for (dbmfp = TAILQ_FIRST(&dbmp->dbmfq);
	    dbmfp != NULL; dbmfp = TAILQ_NEXT(dbmfp, q))
		if (dbmfp->mfp == mfp) {
			if (F_ISSET(dbmfp, MP_READONLY) &&
			    __memp_upgrade(dbmp, dbmfp, mfp))
				return (0);
			break;
		}
	UNLOCKHANDLE(dbmp, &dbmp->mutex);
	if (dbmfp != NULL)
		goto found;

	/*
	 * It's not a page from a file we've opened.  If the file requires
	 * input/output processing, see if this process has ever registered
	 * information as to how to write this type of file.  If not, there's
	 * nothing we can do.
	 */
	if (mfp->ftype != 0) {
		LOCKHANDLE(dbmp, &dbmp->mutex);
		for (mpreg = LIST_FIRST(&dbmp->dbregq);
		    mpreg != NULL; mpreg = LIST_NEXT(mpreg, q))
			if (mpreg->ftype == mfp->ftype)
				break;
		UNLOCKHANDLE(dbmp, &dbmp->mutex);
		if (mpreg == NULL)
			return (0);
	}

	/*
	 * Try and open the file; ignore any error, assume it's a permissions
	 * problem.
	 *
	 * XXX
	 * There's no negative cache here, so we may repeatedly try and open
	 * files that we have previously tried (and failed) to open.
	 */
	dbt.size = mfp->pgcookie_len;
	dbt.data = ADDR(dbmp, mfp->pgcookie_off);
	if (__memp_fopen(dbmp, ADDR(dbmp, mfp->path_off),
	    mfp->ftype, 0, 0, mfp->stat.st_pagesize,
	    mfp->lsn_off, &dbt, ADDR(dbmp, mfp->fileid_off), 0, &dbmfp) != 0)
		return (0);

found:	return (__memp_pgwrite(dbmfp, bhp, restartp, wrotep));
}

/*
 * __memp_pgread --
 *	Read a page from a file.
 *
 * PUBLIC: int __memp_pgread __P((DB_MPOOLFILE *, BH *, int));
 */
int
__memp_pgread(dbmfp, bhp, can_create)
	DB_MPOOLFILE *dbmfp;
	BH *bhp;
	int can_create;
{
	DB_MPOOL *dbmp;
	MPOOLFILE *mfp;
	size_t pagesize;
	ssize_t nr;
	int ret;

	dbmp = dbmfp->dbmp;
	mfp = dbmfp->mfp;
	pagesize = mfp->stat.st_pagesize;

	F_SET(bhp, BH_LOCKED | BH_TRASH);
	LOCKBUFFER(dbmp, bhp);
	UNLOCKREGION(dbmp);

	/*
	 * Temporary files may not yet have been created.
	 *
	 * Seek to the page location.
	 */
	ret = 0;
	LOCKHANDLE(dbmp, &dbmfp->mutex);
	if (dbmfp->fd == -1 || (ret =
	    __db_lseek(dbmfp->fd, pagesize, bhp->pgno, 0, SEEK_SET)) != 0) {
		if (!can_create) {
			if (dbmfp->fd == -1)
				ret = EINVAL;
			UNLOCKHANDLE(dbmp, &dbmfp->mutex);
			__db_err(dbmp->dbenv,
			    "%s: page %lu doesn't exist, create flag not set",
			    dbmfp->path, (u_long)bhp->pgno);
			goto err;
		}
		UNLOCKHANDLE(dbmp, &dbmfp->mutex);

		/* Clear any uninitialized data. */
		memset(bhp->buf, 0, pagesize);
		goto pgin;
	}

	/*
	 * Read the page; short reads are treated like creates, although
	 * any valid data is preserved.
	 */
	ret = __db_read(dbmfp->fd, bhp->buf, pagesize, &nr);
	UNLOCKHANDLE(dbmp, &dbmfp->mutex);
	if (ret != 0)
		goto err;

	if (nr == (ssize_t)pagesize)
		can_create = 0;
	else {
		if (!can_create) {
			ret = EINVAL;
			goto err;
		}

		/* Clear any uninitialized data. */
		memset(bhp->buf + nr, 0, pagesize - nr);
	}

	/* Call any pgin function. */
pgin:	ret = mfp->ftype == 0 ? 0 : __memp_pg(dbmfp, bhp, 1);

	/* Reacquire the region lock. */
	LOCKREGION(dbmp);

	/* If the pgin function succeeded, the data is now valid. */
	if (ret == 0)
		F_CLR(bhp, BH_TRASH);

	/* Update the statistics. */
	if (can_create) {
		++dbmp->mp->stat.st_page_create;
		++mfp->stat.st_page_create;
	} else {
		++dbmp->mp->stat.st_page_in;
		++mfp->stat.st_page_in;
	}

	if (0) {
err:		LOCKREGION(dbmp);
	}

	/* Release the buffer. */
	F_CLR(bhp, BH_LOCKED);
	UNLOCKBUFFER(dbmp, bhp);

	return (ret);
}

/*
 * __memp_pgwrite --
 *	Write a page to a file.
 *
 * PUBLIC: int __memp_pgwrite __P((DB_MPOOLFILE *, BH *, int *, int *));
 */
int
__memp_pgwrite(dbmfp, bhp, restartp, wrotep)
	DB_MPOOLFILE *dbmfp;
	BH *bhp;
	int *restartp, *wrotep;
{
	DB_ENV *dbenv;
	DB_LOG *lg_info;
	DB_LSN lsn;
	DB_MPOOL *dbmp;
	MPOOL *mp;
	MPOOLFILE *mfp;
	size_t pagesize;
	ssize_t nw;
	int callpgin, ret;
	const char *fail;

	dbmp = dbmfp->dbmp;
	dbenv = dbmp->dbenv;
	mfp = dbmfp->mfp;

	if (restartp != NULL)
		*restartp = 0;
	if (wrotep != NULL)
		*wrotep = 0;
	callpgin = 0;
	pagesize = mfp->stat.st_pagesize;

	F_SET(bhp, BH_LOCKED);
	LOCKBUFFER(dbmp, bhp);
	UNLOCKREGION(dbmp);

	if (restartp != NULL)
		*restartp = 1;

	/* Copy the LSN off the page if we're going to need it. */
	lg_info = dbenv->lg_info;
	if (lg_info != NULL || F_ISSET(bhp, BH_WRITE))
		memcpy(&lsn, bhp->buf + mfp->lsn_off, sizeof(DB_LSN));

	/* Ensure the appropriate log records are on disk. */
	if (lg_info != NULL && (ret = log_flush(lg_info, &lsn)) != 0)
		goto err;

	/*
	 * Call any pgout function.  We set the callpgin flag so that on
	 * error we flag that the contents of the buffer may be trash.
	 */
	if (mfp->ftype == 0)
		ret = 0;
	else {
		callpgin = 1;
		if ((ret = __memp_pg(dbmfp, bhp, 0)) != 0)
			goto err;
	}

	/* Temporary files may not yet have been created. */
	LOCKHANDLE(dbmp, &dbmfp->mutex);
	if (dbmfp->fd == -1 && ((ret = __db_appname(dbenv, DB_APP_TMP,
	    NULL, NULL, &dbmfp->fd, NULL)) != 0 || dbmfp->fd == -1)) {
		UNLOCKHANDLE(dbmp, &dbmfp->mutex);
		__db_err(dbenv, "unable to create temporary backing file");
		goto err;
	}

	/* Write the page out. */
	if ((ret =
	    __db_lseek(dbmfp->fd, pagesize, bhp->pgno, 0, SEEK_SET)) != 0)
		fail = "seek";
	else if ((ret = __db_write(dbmfp->fd, bhp->buf, pagesize, &nw)) != 0)
		fail = "write";
	UNLOCKHANDLE(dbmp, &dbmfp->mutex);
	if (ret != 0) {
		/*
		 * XXX
		 * Shut the compiler up; it doesn't understand the correlation
		 * between the failing clauses to __db_lseek and __db_write and
		 * this ret != 0.
		 */
		fail = NULL;
		goto syserr;
	}

	if (nw != (ssize_t)pagesize) {
		ret = EIO;
		fail = "write";
		goto syserr;
	}

	if (wrotep != NULL)
		*wrotep = 1;

	/* Reacquire the region lock. */
	LOCKREGION(dbmp);

	/* Clean up the flags based on a successful write. */
	F_SET(bhp, BH_CALLPGIN);
	F_CLR(bhp, BH_DIRTY | BH_LOCKED);
	UNLOCKBUFFER(dbmp, bhp);

	/*
	 * If we wrote a buffer which a checkpoint is waiting for, update
	 * the count of pending buffers (both in the mpool as a whole and
	 * for this file).  If the count for this file goes to zero, flush
	 * the writes.
	 *
	 * XXX:
	 * We ignore errors from the sync -- it makes no sense to return an
	 * error to the calling process, so set a flag causing the sync to
	 * be retried later.
	 *
	 * If the buffer we wrote has a LSN larger than the current largest
	 * we've written for this checkpoint, update the saved value.
	 */
	mp = dbmp->mp;
	if (F_ISSET(bhp, BH_WRITE)) {
		if (log_compare(&lsn, &mp->lsn) > 0)
			mp->lsn = lsn;
		F_CLR(bhp, BH_WRITE);

		--mp->lsn_cnt;
		if (--mfp->lsn_cnt == 0) {
			/*
			 * Don't lock -- there are no atomicity issues for
			 * fsync(2).
			 */
			if (__db_fsync(dbmfp->fd) != 0)
				F_SET(mp, MP_LSN_RETRY);
		}
	}

	/* Update I/O statistics. */
	++mp->stat.st_page_out;
	++mfp->stat.st_page_out;

	return (0);

syserr:	__db_err(dbenv,
	    "%s: %s failed for page %lu", dbmfp->path, fail, (u_long)bhp->pgno);

err:	UNLOCKBUFFER(dbmp, bhp);
	LOCKREGION(dbmp);
	if (callpgin)
		F_SET(bhp, BH_CALLPGIN);
	F_CLR(bhp, BH_LOCKED);
	return (ret);
}

/*
 * __memp_pg --
 *	Call the pgin/pgout routine.
 *
 * PUBLIC: int __memp_pg __P((DB_MPOOLFILE *, BH *, int));
 */
int
__memp_pg(dbmfp, bhp, is_pgin)
	DB_MPOOLFILE *dbmfp;
	BH *bhp;
	int is_pgin;
{
	DBT dbt, *dbtp;
	DB_MPOOL *dbmp;
	DB_MPREG *mpreg;
	MPOOLFILE *mfp;
	int ftype, ret;

	dbmp = dbmfp->dbmp;
	mfp = dbmfp->mfp;

	LOCKHANDLE(dbmp, &dbmp->mutex);

	ftype = mfp->ftype;
	for (mpreg = LIST_FIRST(&dbmp->dbregq);
	    mpreg != NULL; mpreg = LIST_NEXT(mpreg, q)) {
		if (ftype != mpreg->ftype)
			continue;
		if (mfp->pgcookie_len == 0)
			dbtp = NULL;
		else {
			dbt.size = mfp->pgcookie_len;
			dbt.data = ADDR(dbmp, mfp->pgcookie_off);
			dbtp = &dbt;
		}
		UNLOCKHANDLE(dbmp, &dbmp->mutex);

		if (is_pgin) {
			if (mpreg->pgin != NULL && (ret =
			    mpreg->pgin(bhp->pgno, bhp->buf, dbtp)) != 0)
				goto err;
		} else
			if (mpreg->pgout != NULL && (ret =
			    mpreg->pgout(bhp->pgno, bhp->buf, dbtp)) != 0)
				goto err;
		break;
	}

	if (mpreg == NULL)
		UNLOCKHANDLE(dbmp, &dbmp->mutex);

	return (0);

err:	UNLOCKHANDLE(dbmp, &dbmp->mutex);
	__db_err(dbmp->dbenv, "%s: %s failed for page %lu",
	    dbmfp->path, is_pgin ? "pgin" : "pgout", (u_long)bhp->pgno);
	return (ret);
}

/*
 * __memp_bhfree --
 *	Free a bucket header and its referenced data.
 *
 * PUBLIC: void __memp_bhfree __P((DB_MPOOL *, MPOOLFILE *, BH *, int));
 */
void
__memp_bhfree(dbmp, mfp, bhp, free_mem)
	DB_MPOOL *dbmp;
	MPOOLFILE *mfp;
	BH *bhp;
	int free_mem;
{
	size_t off;

	/* Delete the buffer header from the MPOOL hash list. */
	off = BUCKET(dbmp->mp, OFFSET(dbmp, mfp), bhp->pgno);
	SH_TAILQ_REMOVE(&dbmp->htab[off], bhp, mq, __bh);

	/* Delete the buffer header from the LRU chain. */
	SH_TAILQ_REMOVE(&dbmp->mp->bhq, bhp, q, __bh);

	/*
	 * If we're not reusing it immediately, free the buffer header
	 * and data for real.
	 */
	if (free_mem)
		__db_shalloc_free(dbmp->addr, bhp);
}

/*
 * __memp_upgrade --
 *	Upgrade a file descriptor from readonly to readwrite.
 */
static int
__memp_upgrade(dbmp, dbmfp, mfp)
	DB_MPOOL *dbmp;
	DB_MPOOLFILE *dbmfp;
	MPOOLFILE *mfp;
{
	int fd;

	/*
	 * !!!
	 * We expect the handle to already be locked.
	 */

	/* Check to see if we've already upgraded. */
	if (F_ISSET(dbmfp, MP_UPGRADE))
		return (0);

	/* Check to see if we've already failed. */
	if (F_ISSET(dbmfp, MP_UPGRADE_FAIL))
		return (1);

	/* Try the open. */
	if (__db_fdopen(ADDR(dbmp, mfp->path_off), 0, 0, 0, &fd) != 0) {
		F_SET(dbmfp, MP_UPGRADE_FAIL);
		return (1);
	}

	/* Swap the descriptors and set the upgrade flag. */
	(void)close(dbmfp->fd);
	dbmfp->fd = fd;
	F_SET(dbmfp, MP_UPGRADE);

	return (0);
}
