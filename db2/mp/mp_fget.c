/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_fget.c	10.22 (Sleepycat) 8/19/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "common_ext.h"

int __sleep_on_every_page_get;		/* XXX: thread debugging option. */

/*
 * memp_fget --
 *	Get a page from the file.
 */
int
memp_fget(dbmfp, pgnoaddr, flags, addrp)
	DB_MPOOLFILE *dbmfp;
	db_pgno_t *pgnoaddr;
	u_long flags;
	void *addrp;
{
	BH *bhp, *tbhp;
	DB_MPOOL *dbmp;
	MPOOL *mp;
	MPOOLFILE *mfp;
	db_pgno_t lastpgno;
	size_t bucket, mf_offset;
	off_t size;
	u_long cnt;
	int b_incr, b_inserted, readonly_alloc, ret;
	void *addr;

	dbmp = dbmfp->dbmp;

	/*
	 * Validate arguments.
	 *
	 * !!!
	 * Don't test for DB_MPOOL_CREATE and DB_MPOOL_NEW flags for readonly
	 * files here, and create non-existent pages in readonly files if the
	 * flags are set, later.  The reason is that the hash access method
	 * wants to get empty pages that don't really exist in readonly files.
	 * The only alternative is for hash to write the last "bucket" all the
	 * time, which we don't want to do because one of our big goals in life
	 * is to keep database files small.  It's sleazy as hell, but we catch
	 * any attempt to actually write the file in memp_fput().
	 */
#define	OKFLAGS	(DB_MPOOL_CREATE | DB_MPOOL_LAST | DB_MPOOL_NEW)
	if (flags != 0) {
		if ((ret =
		    __db_fchk(dbmp->dbenv, "memp_fget", flags, OKFLAGS)) != 0)
			return (ret);

		switch (flags) {
		case DB_MPOOL_CREATE:
		case DB_MPOOL_LAST:
		case DB_MPOOL_NEW:
		case 0:
			break;
		default:
			return (__db_ferr(dbmp->dbenv, "memp_fget", 1));
		}
	}

#ifdef DEBUG
	/*
	 * XXX
	 * We want to switch threads as often as possible.  Sleep every time
	 * we get a new page to make it more likely.
	 */
	if (__sleep_on_every_page_get && (dbmp->dbenv == NULL ||
	    dbmp->dbenv->db_yield == NULL || dbmp->dbenv->db_yield() != 0))
		__db_sleep(0, 1);
#endif

	mp = dbmp->mp;
	mfp = dbmfp->mfp;
	mf_offset = OFFSET(dbmp, mfp);
	addr = NULL;
	bhp = NULL;
	b_incr = b_inserted = readonly_alloc = ret = 0;

	LOCKREGION(dbmp);

	/*
	 * If mmap'ing the file, just return a pointer.  However, if another
	 * process has opened the file for writing since we mmap'd it, start
	 * playing the game by their rules, i.e. everything goes through the
	 * cache.  All pages previously returned should be safe, as long as
	 * a locking protocol was observed.
	 *
	 * XXX
	 * We don't discard the map because we don't know when all of the
	 * pages will have been discarded from the process' address space.
	 * It would be possible to do so by reference counting the open
	 * pages from the mmap, but it's unclear to me that it's worth it.
	 */
	if (dbmfp->addr != NULL && dbmfp->mfp->can_mmap) {
		lastpgno = dbmfp->len == 0 ?
		    0 : (dbmfp->len - 1) / mfp->stat.st_pagesize;
		if (LF_ISSET(DB_MPOOL_LAST))
			*pgnoaddr = lastpgno;
		else {
			/*
			 * !!!
			 * Allocate a page that can never really exist.  See
			 * the comment above about non-existent pages and the
			 * hash access method.
			 */
			if (LF_ISSET(DB_MPOOL_CREATE | DB_MPOOL_NEW))
				readonly_alloc = 1;
			else if (*pgnoaddr > lastpgno) {
				__db_err(dbmp->dbenv,
				    "%s: page %lu doesn't exist",
				    dbmfp->path, (u_long)*pgnoaddr);
				ret = EINVAL;
				goto err;
			}
		}
		if (!readonly_alloc) {
			addr = ADDR(dbmfp, *pgnoaddr * mfp->stat.st_pagesize);

			++mp->stat.st_map;
			++mfp->stat.st_map;

			goto mapret;
		}
	}

	/*
	 * If requesting the last page or a new page, find the last page.  The
	 * tricky thing is that the user may have created a page already that's
	 * after any page that exists in the file.
	 */
	if (LF_ISSET(DB_MPOOL_LAST | DB_MPOOL_NEW)) {
		/*
		 * Temporary files may not yet have been created.
		 *
		 * Don't lock -- there are no atomicity issues for stat(2).
		 */
		if (dbmfp->fd == -1)
			size = 0;
		else if ((ret = __db_stat(dbmp->dbenv,
		    dbmfp->path, dbmfp->fd, &size, NULL)) != 0)
			goto err;

		*pgnoaddr = size == 0 ? 0 : (size - 1) / mfp->stat.st_pagesize;

		/*
		 * Walk the list of BH's, looking for later pages.  Save the
		 * pointer if a later page is found so that we don't have to
		 * search the list twice.
		 *
		 * If requesting a new page, return the page one after the last
		 * page -- which we'll have to create.
		 */
		for (tbhp = SH_TAILQ_FIRST(&mp->bhq, __bh);
		    tbhp != NULL; tbhp = SH_TAILQ_NEXT(tbhp, q, __bh))
			if (tbhp->pgno >= *pgnoaddr &&
			    tbhp->mf_offset == mf_offset) {
				bhp = tbhp;
				*pgnoaddr = bhp->pgno;
			}
		if (LF_ISSET(DB_MPOOL_NEW))
			++*pgnoaddr;
	}

	/* If we already found the right buffer, return it. */
	if (LF_ISSET(DB_MPOOL_LAST) && bhp != NULL) {
		addr = bhp->buf;
		goto found;
	}

	/* If we haven't checked the BH list yet, do the search. */
	if (!LF_ISSET(DB_MPOOL_LAST | DB_MPOOL_NEW)) {
		++mp->stat.st_hash_searches;
		bucket = BUCKET(mp, mf_offset, *pgnoaddr);
		for (cnt = 0,
		    bhp = SH_TAILQ_FIRST(&dbmp->htab[bucket], __bh);
		    bhp != NULL; bhp = SH_TAILQ_NEXT(bhp, mq, __bh)) {
			++cnt;
			if (bhp->pgno == *pgnoaddr &&
			    bhp->mf_offset == mf_offset) {
				addr = bhp->buf;
				if (cnt > mp->stat.st_hash_longest)
					mp->stat.st_hash_longest = cnt;
				mp->stat.st_hash_examined += cnt;
				goto found;
			}
		}
		if (cnt > mp->stat.st_hash_longest)
			mp->stat.st_hash_longest = cnt;
		mp->stat.st_hash_examined += cnt;
	}

	/*
	 * Allocate a new buffer header and data space, and mark the contents
	 * as useless.
	 */
	if ((ret = __memp_ralloc(dbmp, sizeof(BH) -
	    sizeof(u_int8_t) + mfp->stat.st_pagesize, NULL, &bhp)) != 0)
		goto err;
	addr = bhp->buf;
#ifdef DEBUG
	if ((ALIGNTYPE)addr & (sizeof(size_t) - 1)) {
		__db_err(dbmp->dbenv,
		    "Internal error: BH data NOT size_t aligned.");
		abort();
	}
#endif
	memset(bhp, 0, sizeof(BH));
	LOCKINIT(dbmp, &bhp->mutex);

	/*
	 * Prepend the bucket header to the head of the appropriate MPOOL
	 * bucket hash list.  Append the bucket header to the tail of the
	 * MPOOL LRU chain.
	 *
	 * We have to do this before we read in the page so we can discard
	 * our region lock without screwing up the world.
	 */
	bucket = BUCKET(mp, mf_offset, *pgnoaddr);
	SH_TAILQ_INSERT_HEAD(&dbmp->htab[bucket], bhp, mq, __bh);
	SH_TAILQ_INSERT_TAIL(&mp->bhq, bhp, q);
	b_inserted = 1;

	/* Set the page number, and associated MPOOLFILE. */
	bhp->mf_offset = mf_offset;
	bhp->pgno = *pgnoaddr;

	/*
	 * If we know we created the page, zero it out and continue.
	 *
	 * !!!
	 * Note: DB_MPOOL_NEW deliberately doesn't call the pgin function.
	 * If DB_MPOOL_CREATE is used, then the application's pgin function
	 * has to be able to handle pages of 0's -- if it uses DB_MPOOL_NEW,
	 * it can detect all of its page creates, and not bother.
	 *
	 * Otherwise, read the page into memory, optionally creating it if
	 * DB_MPOOL_CREATE is set.
	 *
	 * Increment the reference count for created buffers, but importantly,
	 * increment the reference count for buffers we're about to read so
	 * that the buffer can't move.
	 */
	++bhp->ref;
	b_incr = 1;

	if (LF_ISSET(DB_MPOOL_NEW))
		memset(addr, 0, mfp->stat.st_pagesize);
	else {
		/*
		 * It's possible for the read function to fail, which means
		 * that we fail as well.
		 */
reread:		if ((ret = __memp_pgread(dbmfp,
		    bhp, LF_ISSET(DB_MPOOL_CREATE | DB_MPOOL_NEW))) != 0)
			goto err;

		/*
		 * !!!
		 * The __memp_pgread call discarded and reacquired the region
		 * lock.  Because the buffer reference count was incremented
		 * before the region lock was discarded the buffer didn't move.
		 */
		++mp->stat.st_cache_miss;
		++mfp->stat.st_cache_miss;
	}

	if (0) {
found:		/* Increment the reference count. */
		if (bhp->ref == UINT16_T_MAX) {
			__db_err(dbmp->dbenv,
			    "%s: too many references to page %lu",
			    dbmfp->path, bhp->pgno);
			ret = EAGAIN;
			goto err;
		}
		++bhp->ref;
		b_incr = 1;

		/*
	 	 * Any found buffer might be trouble.
		 *
		 * BH_LOCKED --
		 * I/O in progress, wait for it to finish.  Because the buffer
		 * reference count was incremented before the region lock was
		 * discarded we know the buffer didn't move.
		 */
		if (F_ISSET(bhp, BH_LOCKED)) {
			UNLOCKREGION(dbmp);
			LOCKBUFFER(dbmp, bhp);
			/* Waiting for I/O to finish... */
			UNLOCKBUFFER(dbmp, bhp);
			LOCKREGION(dbmp);
		}

		/*
		 * BH_TRASH --
		 * The buffer is garbage.
		 */
		if (F_ISSET(bhp, BH_TRASH))
			goto reread;

		/*
		 * BH_CALLPGIN --
		 * The buffer was written, and the contents need to be
		 * converted again.
		 */
		if (F_ISSET(bhp, BH_CALLPGIN)) {
			if ((ret = __memp_pg(dbmfp, bhp, 1)) != 0)
				goto err;
			F_CLR(bhp, BH_CALLPGIN);
		}

		++mp->stat.st_cache_hit;
		++mfp->stat.st_cache_hit;
	}

mapret:	LOCKHANDLE(dbmp, &dbmfp->mutex);
	++dbmfp->pinref;
	UNLOCKHANDLE(dbmp, &dbmfp->mutex);

	if (0) {
err:		/*
		 * If no other process is already waiting on a created buffer,
		 * go ahead and discard it, it's not useful.
		 */
		if (b_incr)
			--bhp->ref;
		if (b_inserted && bhp->ref == 0)
			__memp_bhfree(dbmp, mfp, bhp, 1);
	}

	UNLOCKREGION(dbmp);

	*(void **)addrp = addr;
	return (ret);
}
