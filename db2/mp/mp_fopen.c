/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_fopen.c	10.27 (Sleepycat) 9/23/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "common_ext.h"

static int __memp_mf_close __P((DB_MPOOL *, DB_MPOOLFILE *));
static int __memp_mf_open __P((DB_MPOOL *,
    DB_MPOOLFILE *, int, size_t, int, DBT *, u_int8_t *, int, MPOOLFILE **));

/*
 * memp_fopen --
 *	Open a backing file for the memory pool.
 */
int
memp_fopen(dbmp, path, ftype,
    flags, mode, pagesize, lsn_offset, pgcookie, fileid, retp)
	DB_MPOOL *dbmp;
	const char *path;
	int ftype, flags, mode, lsn_offset;
	size_t pagesize;
	DBT *pgcookie;
	u_int8_t *fileid;
	DB_MPOOLFILE **retp;
{
	int ret;

	/* Validate arguments. */
	if ((ret = __db_fchk(dbmp->dbenv,
	    "memp_fopen", flags, DB_CREATE | DB_NOMMAP | DB_RDONLY)) != 0)
		return (ret);

	return (__memp_fopen(dbmp, path, ftype,
	    flags, mode, pagesize, lsn_offset, pgcookie, fileid, 1, retp));
}

/*
 * __memp_fopen --
 *	Open a backing file for the memory pool; internal version.
 *
 * PUBLIC: int __memp_fopen __P((DB_MPOOL *, const char *, int, int,
 * PUBLIC:    int, size_t, int, DBT *, u_int8_t *, int, DB_MPOOLFILE **));
 */
int
__memp_fopen(dbmp, path,
    ftype, flags, mode, pagesize, lsn_offset, pgcookie, fileid, needlock, retp)
	DB_MPOOL *dbmp;
	const char *path;
	int ftype, flags, mode, lsn_offset, needlock;
	size_t pagesize;
	DBT *pgcookie;
	u_int8_t *fileid;
	DB_MPOOLFILE **retp;
{
	DB_ENV *dbenv;
	DB_MPOOLFILE *dbmfp;
	MPOOLFILE *mfp;
	off_t size;
	int ret;

	dbenv = dbmp->dbenv;
	ret = 0;

	/* Require a non-zero pagesize. */
	if (pagesize == 0) {
		__db_err(dbenv, "memp_fopen: pagesize not specified");
		return (EINVAL);
	}

	/* Allocate and initialize the per-process structure. */
	if ((dbmfp =
	    (DB_MPOOLFILE *)calloc(1, sizeof(DB_MPOOLFILE))) == NULL) {
		__db_err(dbenv, "%s: %s",
		    path == NULL ? TEMPORARY : path, strerror(ENOMEM));
		return (ENOMEM);
	}
	dbmfp->dbmp = dbmp;
	dbmfp->fd = -1;
	if (LF_ISSET(DB_RDONLY))
		F_SET(dbmfp, MP_READONLY);

	if (path == NULL) {
		if (LF_ISSET(DB_RDONLY)) {
			__db_err(dbenv,
			    "memp_fopen: temporary files can't be readonly");
			ret = EINVAL;
			goto err;
		}
		dbmfp->path = (char *)TEMPORARY;
		F_SET(dbmfp, MP_PATH_TEMP);
	} else {
		/* Calculate the real name for this file. */
		if ((ret = __db_appname(dbenv,
		    DB_APP_DATA, NULL, path, NULL, &dbmfp->path)) != 0)
			goto err;
		F_SET(dbmfp, MP_PATH_ALLOC);


		/* Open the file. */
		if ((ret = __db_fdopen(dbmfp->path,
		    LF_ISSET(DB_CREATE | DB_RDONLY), DB_CREATE | DB_RDONLY,
		    mode, &dbmfp->fd)) != 0) {
			__db_err(dbenv, "%s: %s", dbmfp->path, strerror(ret));
			goto err;
		}

		/* Don't permit files that aren't a multiple of the pagesize. */
		if ((ret = __db_stat(dbenv,
		     dbmfp->path, dbmfp->fd, &size, NULL)) != 0)
			goto err;
		if (size % pagesize) {
			__db_err(dbenv,
			    "%s: file size not a multiple of the pagesize",
			    dbmfp->path);
			ret = EINVAL;
			goto err;
		}
	}

	/*
	 * Find/allocate the shared file objects.  This includes allocating
	 * space for the per-process thread lock.
	 */
	if (needlock)
		LOCKREGION(dbmp);
	ret = __memp_mf_open(dbmp, dbmfp, ftype, pagesize,
	    lsn_offset, pgcookie, fileid, F_ISSET(dbmfp, MP_PATH_TEMP), &mfp);
	if (ret == 0 &&
	    F_ISSET(dbmp, MP_LOCKHANDLE) && (ret =
	    __memp_ralloc(dbmp, sizeof(db_mutex_t), NULL, &dbmfp->mutexp)) == 0)
		LOCKINIT(dbmp, dbmfp->mutexp);
	if (needlock)
		UNLOCKREGION(dbmp);

	if (ret != 0)
		goto err;

	dbmfp->mfp = mfp;

	/*
	 * If a file:
	 *	+ is read-only
	 *	+ isn't temporary
	 *	+ doesn't require any pgin/pgout support
	 *	+ the DB_NOMMAP flag wasn't set
	 *	+ and is less than mp_mmapsize bytes in size
	 *
	 * we can mmap it instead of reading/writing buffers.  Don't do error
	 * checking based on the mmap call failure.  We want to do normal I/O
	 * on the file if the reason we failed was because the file was on an
	 * NFS mounted partition, and we can fail in buffer I/O just as easily
	 * as here.
	 *
	 * XXX
	 * We'd like to test to see if the file is too big to mmap.  Since we
	 * don't know what size or type off_t's or size_t's are, or the largest
	 * unsigned integral type is, or what random insanity the local C
	 * compiler will perpetrate, doing the comparison in a portable way is
	 * flatly impossible.  Hope that mmap fails if the file is too large.
	 */
#define	DB_MAXMMAPSIZE	(10 * 1024 * 1024)	/* 10 Mb. */
	if (mfp->can_mmap) {
		if (!F_ISSET(dbmfp, MP_READONLY))
			mfp->can_mmap = 0;
		if (path == NULL)
			mfp->can_mmap = 0;
		if (ftype != 0)
			mfp->can_mmap = 0;
		if (LF_ISSET(DB_NOMMAP))
			mfp->can_mmap = 0;
		if (size > (dbenv == NULL || dbenv->mp_mmapsize == 0 ?
		    DB_MAXMMAPSIZE : (off_t)dbenv->mp_mmapsize))
			mfp->can_mmap = 0;
	}
	dbmfp->addr = NULL;
	if (mfp->can_mmap) {
		dbmfp->len = size;
		if (__db_mmap(dbmfp->fd, dbmfp->len, 1, 1, &dbmfp->addr) != 0) {
			mfp->can_mmap = 0;
			dbmfp->addr = NULL;
		}
	}

	LOCKHANDLE(dbmp, dbmp->mutexp);
	TAILQ_INSERT_TAIL(&dbmp->dbmfq, dbmfp, q);
	UNLOCKHANDLE(dbmp, dbmp->mutexp);

	*retp = dbmfp;
	return (0);

err:	/*
	 * Note that we do not have to free the thread mutex, because we
	 * never get to here after we have successfully allocated it.
	 */
	if (F_ISSET(dbmfp, MP_PATH_ALLOC))
		FREES(dbmfp->path);
	if (dbmfp->fd != -1)
		(void)__db_close(dbmfp->fd);
	if (dbmfp != NULL)
		FREE(dbmfp, sizeof(DB_MPOOLFILE));
	return (ret);
}

/*
 * __memp_mf_open --
 *	Open an MPOOLFILE.
 */
static int
__memp_mf_open(dbmp, dbmfp,
    ftype, pagesize, lsn_offset, pgcookie, fileid, istemp, retp)
	DB_MPOOL *dbmp;
	DB_MPOOLFILE *dbmfp;
	int ftype, lsn_offset, istemp;
	size_t pagesize;
	DBT *pgcookie;
	u_int8_t *fileid;
	MPOOLFILE **retp;
{
	MPOOLFILE *mfp;
	int ret;
	u_int8_t idbuf[DB_FILE_ID_LEN];
	void *p;

	/* Temporary files can't match previous files. */
	if (istemp)
		goto alloc;

	/*
	 * Get the file id if we weren't give one.  Generated file id's don't
	 * use timestamps, otherwise there'd be no chance of anyone joining
	 * the party.
	 */
	if (fileid == NULL) {
		if ((ret =
		    __db_fileid(dbmp->dbenv, dbmfp->path, 0, idbuf)) != 0)
			return (ret);
		fileid = idbuf;
	}

	/* Walk the list of MPOOLFILE's, looking for a matching file. */
	for (mfp = SH_TAILQ_FIRST(&dbmp->mp->mpfq, __mpoolfile);
	    mfp != NULL; mfp = SH_TAILQ_NEXT(mfp, q, __mpoolfile))
		if (!memcmp(fileid,
		    ADDR(dbmp, mfp->fileid_off), DB_FILE_ID_LEN)) {
			if (ftype != mfp->ftype ||
			    pagesize != mfp->stat.st_pagesize) {
				__db_err(dbmp->dbenv,
				    "%s: ftype or pagesize changed",
				    dbmfp->path);
				ret = EINVAL;
				mfp = NULL;
				goto ret1;
			}
			/* Found it: increment the reference count. */
			++mfp->ref;
			goto ret1;
		}

	/* Allocate a new MPOOLFILE. */
alloc:	if ((ret = __memp_ralloc(dbmp, sizeof(MPOOLFILE), NULL, &mfp)) != 0)
		goto ret1;

	/* Initialize the structure. */
	memset(mfp, 0, sizeof(MPOOLFILE));
	mfp->ref = 1;
	mfp->ftype = ftype;
	mfp->can_mmap = 1;
	mfp->lsn_off = lsn_offset;
	mfp->stat.st_pagesize = pagesize;

	/* Copy the file path into shared memory. */
	if ((ret = __memp_ralloc(dbmp,
	    strlen(dbmfp->path) + 1, &mfp->path_off, &p)) != 0)
		goto err;
	memcpy(p, dbmfp->path, strlen(dbmfp->path) + 1);

	/* Copy the file identification string into shared memory. */
	if (istemp)
		mfp->fileid_off = 0;
	else {
		if ((ret = __memp_ralloc(dbmp,
		    DB_FILE_ID_LEN, &mfp->fileid_off, &p)) != 0)
			goto err;
		memcpy(p, fileid, DB_FILE_ID_LEN);
	}

	/* Copy the page cookie into shared memory. */
	if (pgcookie == NULL || pgcookie->size == 0) {
		mfp->pgcookie_len = 0;
		mfp->pgcookie_off = 0;
	} else {
		if ((ret = __memp_ralloc(dbmp,
		    pgcookie->size, &mfp->pgcookie_off, &p)) != 0)
			goto err;
		memcpy(p, pgcookie->data, pgcookie->size);
		mfp->pgcookie_len = pgcookie->size;
	}

	/* Prepend the MPOOLFILE to the list of MPOOLFILE's. */
	SH_TAILQ_INSERT_HEAD(&dbmp->mp->mpfq, mfp, q, __mpoolfile);

	if (0) {
err:		if (mfp->path_off != 0)
			__db_shalloc_free(dbmp->addr,
			    ADDR(dbmp, mfp->path_off));
		if (!istemp)
			__db_shalloc_free(dbmp->addr,
			    ADDR(dbmp, mfp->fileid_off));
		if (mfp != NULL)
			__db_shalloc_free(dbmp->addr, mfp);
		mfp = NULL;
	}

ret1:	*retp = mfp;
	return (0);
}

/*
 * memp_fclose --
 *	Close a backing file for the memory pool.
 */
int
memp_fclose(dbmfp)
	DB_MPOOLFILE *dbmfp;
{
	DB_MPOOL *dbmp;
	int ret, t_ret;

	dbmp = dbmfp->dbmp;
	ret = 0;

	/* Complain if pinned blocks never returned. */
	if (dbmfp->pinref != 0)
		__db_err(dbmp->dbenv, "%s: close: %lu blocks left pinned",
		    dbmfp->path, (u_long)dbmfp->pinref);

	/* Remove the DB_MPOOLFILE structure from the list. */
	LOCKHANDLE(dbmp, dbmp->mutexp);
	TAILQ_REMOVE(&dbmp->dbmfq, dbmfp, q);
	UNLOCKHANDLE(dbmp, dbmp->mutexp);

	/* Close the underlying MPOOLFILE. */
	(void)__memp_mf_close(dbmp, dbmfp);

	/* Discard any mmap information. */
	if (dbmfp->addr != NULL &&
	    (ret = __db_munmap(dbmfp->addr, dbmfp->len)) != 0)
		__db_err(dbmp->dbenv, "%s: %s", dbmfp->path, strerror(ret));

	/* Close the file; temporary files may not yet have been created. */
	if (dbmfp->fd != -1 && (t_ret = __db_close(dbmfp->fd)) != 0) {
		__db_err(dbmp->dbenv, "%s: %s", dbmfp->path, strerror(t_ret));
		if (ret != 0)
			t_ret = ret;
	}

	/* Free memory. */
	if (F_ISSET(dbmfp, MP_PATH_ALLOC))
		FREES(dbmfp->path);
	if (dbmfp->mutexp != NULL) {
		LOCKREGION(dbmp);
		__db_shalloc_free(dbmp->addr, dbmfp->mutexp);
		UNLOCKREGION(dbmp);
	}

	/* Discard the DB_MPOOLFILE structure. */
	FREE(dbmfp, sizeof(DB_MPOOLFILE));

	return (ret);
}

/*
 * __memp_mf_close --
 *	Close down an MPOOLFILE.
 */
static int
__memp_mf_close(dbmp, dbmfp)
	DB_MPOOL *dbmp;
	DB_MPOOLFILE *dbmfp;
{
	BH *bhp, *nbhp;
	MPOOL *mp;
	MPOOLFILE *mfp;
	size_t mf_offset;

	mp = dbmp->mp;
	mfp = dbmfp->mfp;

	LOCKREGION(dbmp);

	/* If more than a single reference, simply decrement. */
	if (mfp->ref > 1) {
		--mfp->ref;
		goto ret1;
	}

	/*
	 * Move any BH's held by the file to the free list.  We don't free the
	 * memory itself because we may be discarding the memory pool, and it's
	 * fairly expensive to reintegrate the buffers back into the region for
	 * no purpose.
	 */
	mf_offset = OFFSET(dbmp, mfp);
	for (bhp = SH_TAILQ_FIRST(&mp->bhq, __bh); bhp != NULL; bhp = nbhp) {
		nbhp = SH_TAILQ_NEXT(bhp, q, __bh);

#ifdef DEBUG_NO_DIRTY
		/* Complain if we find any blocks that were left dirty. */
		if (F_ISSET(bhp, BH_DIRTY))
			__db_err(dbmp->dbenv,
			    "%s: close: pgno %lu left dirty; ref %lu",
			    dbmfp->path, (u_long)bhp->pgno, (u_long)bhp->ref);
#endif

		if (bhp->mf_offset == mf_offset) {
			__memp_bhfree(dbmp, mfp, bhp, 0);
			SH_TAILQ_INSERT_HEAD(&mp->bhfq, bhp, q, __bh);
		}
	}

	/* Delete from the list of MPOOLFILEs. */
	SH_TAILQ_REMOVE(&mp->mpfq, mfp, q, __mpoolfile);

	/* Free the space. */
	__db_shalloc_free(dbmp->addr, mfp);
	__db_shalloc_free(dbmp->addr, ADDR(dbmp, mfp->path_off));
	if (mfp->fileid_off != 0)
		__db_shalloc_free(dbmp->addr, ADDR(dbmp, mfp->fileid_off));
	if (mfp->pgcookie_off != 0)
		__db_shalloc_free(dbmp->addr, ADDR(dbmp, mfp->pgcookie_off));

ret1:	UNLOCKREGION(dbmp);
	return (0);
}
