/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_region.c	10.11 (Sleepycat) 8/2/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

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
 * __memp_ralloc --
 *	Allocate some space in the mpool region.
 *
 * PUBLIC: int __memp_ralloc __P((DB_MPOOL *, size_t, size_t *, void *));
 */
int
__memp_ralloc(dbmp, len, offsetp, retp)
	DB_MPOOL *dbmp;
	size_t len, *offsetp;
	void *retp;
{
	BH *bhp, *nbhp;
	MPOOL *mp;
	MPOOLFILE *mfp;
	size_t fsize, total;
	int nomore, restart, ret, wrote;
	void *p;

	mp = dbmp->mp;

	nomore = 0;
alloc:	if ((ret = __db_shalloc(dbmp->addr, len, MUTEX_ALIGNMENT, &p)) == 0) {
		if (offsetp != NULL)
			*offsetp = OFFSET(dbmp, p);
		*(void **)retp = p;
		return (0);
	}
	if (nomore) {
		__db_err(dbmp->dbenv, "%s", strerror(ret));
		return (ret);
	}

	/* Look for a buffer on the free list that's the right size. */
	for (bhp =
	    SH_TAILQ_FIRST(&mp->bhfq, __bh); bhp != NULL; bhp = nbhp) {
		nbhp = SH_TAILQ_NEXT(bhp, q, __bh);

		if (__db_shsizeof(bhp) == len) {
			SH_TAILQ_REMOVE(&mp->bhfq, bhp, q, __bh);
			if (offsetp != NULL)
				*offsetp = OFFSET(dbmp, bhp);
			*(void **)retp = bhp;
			return (0);
		}
	}

	/* Discard from the free list until we've freed enough memory. */
	total = 0;
	for (bhp =
	    SH_TAILQ_FIRST(&mp->bhfq, __bh); bhp != NULL; bhp = nbhp) {
		nbhp = SH_TAILQ_NEXT(bhp, q, __bh);

		SH_TAILQ_REMOVE(&mp->bhfq, bhp, q, __bh);
		__db_shalloc_free(dbmp->addr, bhp);

		/*
		 * Retry as soon as we've freed up sufficient space.  If we
		 * have to coalesce of memory to satisfy the request, don't
		 * try until it's likely (possible?) that we'll succeed.
		 */
		total += fsize = __db_shsizeof(bhp);
		if (fsize >= len || total >= 3 * len)
			goto alloc;
	}

retry:	/* Find a buffer we can flush; pure LRU. */
	total = 0;
	for (bhp =
	    SH_TAILQ_FIRST(&mp->bhq, __bh); bhp != NULL; bhp = nbhp) {
		nbhp = SH_TAILQ_NEXT(bhp, q, __bh);

		/* Ignore pinned or locked (I/O in progress) buffers. */
		if (bhp->ref != 0 || F_ISSET(bhp, BH_LOCKED))
			continue;

		/* Find the associated MPOOLFILE. */
		mfp = ADDR(dbmp, bhp->mf_offset);

		/*
		 * Write the page if it's dirty.
		 *
		 * If we wrote the page, fall through and free the buffer.  We
		 * don't have to rewalk the list to acquire the buffer because
		 * it was never available for any other process to modify it.
		 * If we didn't write the page, but we discarded and reacquired
		 * the region lock, restart the buffer list walk.  If we neither
		 * wrote the buffer nor discarded the region lock, continue down
		 * the buffer list.
		 */
		if (F_ISSET(bhp, BH_DIRTY)) {
			if ((ret = __memp_bhwrite(dbmp,
			    mfp, bhp, &restart, &wrote)) != 0)
				return (ret);

			/*
			 * It's possible that another process wants this buffer
			 * and incremented the ref count while we were writing
			 * it.
			 */
			if (bhp->ref != 0)
				goto retry;

			if (wrote)
				++mp->stat.st_rw_evict;
			else {
				if (restart)
					goto retry;
				else
					continue;
			}
		} else
			++mp->stat.st_ro_evict;

		/*
		 * Check to see if the buffer is the size we're looking for.
		 * If it is, simply reuse it.
		 */
		total += fsize = __db_shsizeof(bhp);
		if (fsize == len) {
			__memp_bhfree(dbmp, mfp, bhp, 0);

			if (offsetp != NULL)
				*offsetp = OFFSET(dbmp, bhp);
			*(void **)retp = bhp;
			return (0);
		}

		/* Free the buffer. */
		__memp_bhfree(dbmp, mfp, bhp, 1);

		/*
		 * Retry as soon as we've freed up sufficient space.  If we
		 * have to coalesce of memory to satisfy the request, don't
		 * try until it's likely (possible?) that we'll succeed.
		 */
		if (fsize >= len || total >= 3 * len)
			goto alloc;

		/* Restart the walk if we discarded the region lock. */
		if (restart)
			goto retry;
	}
	nomore = 1;
	goto alloc;
}

/*
 * __memp_ropen --
 *	Attach to, and optionally create, the mpool region.
 *
 * PUBLIC: int __memp_ropen
 * PUBLIC:    __P((DB_MPOOL *, const char *, size_t, int, int));
 */
int
__memp_ropen(dbmp, path, cachesize, mode, flags)
	DB_MPOOL *dbmp;
	const char *path;
	size_t cachesize;
	int mode, flags;
{
	MPOOL *mp;
	size_t rlen;
	int fd, newregion, ret, retry_cnt;

	/*
	 * Unlike other DB subsystems, mpool can't simply grow the region
	 * because it returns pointers into the region to its clients.  To
	 * "grow" the region, we'd have to allocate a new region and then
	 * store a region number in the structures that reference regional
	 * objects.  It's reasonable that we fail regardless, as clients
	 * shouldn't have every page in the region pinned, so the only
	 * "failure" mode should be a performance penalty because we don't
	 * find a page in the cache that we'd like to have found.
	 *
	 * Up the user's cachesize by 25% to account for our overhead.
	 */
	if (cachesize < DB_CACHESIZE_MIN)
		if (cachesize == 0)
			cachesize = DB_CACHESIZE_DEF;
		else
			cachesize = DB_CACHESIZE_MIN;
	rlen = cachesize + cachesize / 4;

	/* Map in the region. */
	retry_cnt = newregion = 0;
retry:	if (LF_ISSET(DB_CREATE)) {
		/*
		 * If it's a private mpool, use malloc, it's a lot faster than
		 * instantiating a region.
		 *
		 * XXX
		 * If we're doing locking and don't have spinlocks for this
		 * architecture, we'd have to instantiate the file, we need
		 * the file descriptor for locking.  However, it should not
		 * be possible for DB_THREAD to be set if HAVE_SPINLOCKS aren't
		 * defined.
		 */
		if (F_ISSET(dbmp, MP_ISPRIVATE))
			ret = (dbmp->maddr = malloc(rlen)) == NULL ? ENOMEM : 0;
		else
			ret = __db_rcreate(dbmp->dbenv, DB_APP_NONE, path,
			    DB_DEFAULT_MPOOL_FILE, mode, rlen, &fd,
			    &dbmp->maddr);
		if (ret == 0) {
			/* Put the MPOOL structure first in the region. */
			mp = dbmp->maddr;

			SH_TAILQ_INIT(&mp->bhq);
			SH_TAILQ_INIT(&mp->bhfq);
			SH_TAILQ_INIT(&mp->mpfq);

			/* Initialize the rest of the region as free space. */
			dbmp->addr = (u_int8_t *)dbmp->maddr + sizeof(MPOOL);
			__db_shalloc_init(dbmp->addr, rlen - sizeof(MPOOL));

			/*
			 *
			 * Pretend that the cache will be broken up into 4K
			 * pages, and that we want to keep it under, say, 10
			 * pages on each chain.  This means a 256MB cache will
			 * allocate ~6500 offset pairs.
			 */
			mp->htab_buckets =
			    __db_tablesize((cachesize / (4 * 1024)) / 10);

			/* Allocate hash table space and initialize it. */
			if ((ret = __db_shalloc(dbmp->addr,
			    mp->htab_buckets * sizeof(DB_HASHTAB),
			    0, &dbmp->htab)) != 0)
				goto err;
			__db_hashinit(dbmp->htab, mp->htab_buckets);
			mp->htab = OFFSET(dbmp, dbmp->htab);

			memset(&mp->stat, 0, sizeof(mp->stat));
			mp->stat.st_cachesize = cachesize;

			mp->flags = 0;

			newregion = 1;
		} else if (ret != EEXIST)
			return (ret);
	}

	/* If we didn't or couldn't create the region, try and join it. */
	if (!newregion &&
	    (ret = __db_ropen(dbmp->dbenv, DB_APP_NONE,
	    path, DB_DEFAULT_MPOOL_FILE, 0, &fd, &dbmp->maddr)) != 0) {
		/*
		 * If we failed because the file wasn't available, wait a
		 * second and try again.
		 */
		if (ret == EAGAIN && ++retry_cnt < 3) {
			(void)__db_sleep(1, 0);
			goto retry;
		}
		return (ret);
	}

	/* Set up the common pointers. */
	dbmp->mp = dbmp->maddr;
	dbmp->addr = (u_int8_t *)dbmp->maddr + sizeof(MPOOL);

	/*
	 * If not already locked, lock the region -- if it's a new region,
	 * then either __db_rcreate() locked it for us or we malloc'd it
	 * instead of creating a region, neither of which requires locking
	 * here.
	 */
	if (!newregion)
		LOCKREGION(dbmp);

	/*
	 * Get the hash table address; it's on the shared page, so we have
	 * to lock first.
	 */
	dbmp->htab = ADDR(dbmp, dbmp->mp->htab);

	dbmp->fd = fd;

	/* If we locked the region, release it now. */
	if (!F_ISSET(dbmp, MP_ISPRIVATE))
		UNLOCKREGION(dbmp);
	return (0);

err:	if (fd != -1) {
		dbmp->fd = fd;
		(void)__memp_rclose(dbmp);
	}

	if (newregion)
		(void)memp_unlink(path, 1, dbmp->dbenv);
	return (ret);
}

/*
 * __memp_rclose --
 *	Close the mpool region.
 *
 * PUBLIC: int __memp_rclose __P((DB_MPOOL *));
 */
int
__memp_rclose(dbmp)
	DB_MPOOL *dbmp;
{
	if (F_ISSET(dbmp, MP_ISPRIVATE)) {
		free(dbmp->maddr);
		return (0);
	}
	return (__db_rclose(dbmp->dbenv, dbmp->fd, dbmp->maddr));
}
