/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)bt_page.c	10.17 (Sleepycat) 1/3/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"

/*
 * __bam_new --
 *	Get a new page, preferably from the freelist.
 *
 * PUBLIC: int __bam_new __P((DBC *, u_int32_t, PAGE **));
 */
int
__bam_new(dbc, type, pagepp)
	DBC *dbc;
	u_int32_t type;
	PAGE **pagepp;
{
	BTMETA *meta;
	DB *dbp;
	DB_LOCK metalock;
	PAGE *h;
	db_pgno_t pgno;
	int ret;

	dbp = dbc->dbp;
	meta = NULL;
	h = NULL;
	metalock = LOCK_INVALID;

	pgno = PGNO_METADATA;
	if ((ret = __bam_lget(dbc, 0, pgno, DB_LOCK_WRITE, &metalock)) != 0)
		goto err;
	if ((ret = memp_fget(dbp->mpf, &pgno, 0, (PAGE **)&meta)) != 0)
		goto err;

	if (meta->free == PGNO_INVALID) {
		if ((ret = memp_fget(dbp->mpf, &pgno, DB_MPOOL_NEW, &h)) != 0)
			goto err;
		ZERO_LSN(h->lsn);
		h->pgno = pgno;
	} else {
		pgno = meta->free;
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
			goto err;
		meta->free = h->next_pgno;
	}

	/* Log the change. */
	if (DB_LOGGING(dbc)) {
		if ((ret = __bam_pg_alloc_log(dbp->dbenv->lg_info, dbc->txn,
		    &meta->lsn, 0, dbp->log_fileid, &meta->lsn, &h->lsn,
		    h->pgno, (u_int32_t)type, meta->free)) != 0)
			goto err;
		LSN(h) = LSN(meta);
	}

	(void)memp_fput(dbp->mpf, (PAGE *)meta, DB_MPOOL_DIRTY);
	(void)__BT_TLPUT(dbc, metalock);

	P_INIT(h, dbp->pgsize, h->pgno, PGNO_INVALID, PGNO_INVALID, 0, type);
	*pagepp = h;
	return (0);

err:	if (h != NULL)
		(void)memp_fput(dbp->mpf, h, 0);
	if (meta != NULL)
		(void)memp_fput(dbp->mpf, meta, 0);
	if (metalock != LOCK_INVALID)
		(void)__BT_TLPUT(dbc, metalock);
	return (ret);
}

/*
 * __bam_lput --
 *	The standard lock put call.
 *
 * PUBLIC: int __bam_lput __P((DBC *, DB_LOCK));
 */
int
__bam_lput(dbc, lock)
	DBC *dbc;
	DB_LOCK lock;
{
	return (__BT_LPUT(dbc, lock));
}

/*
 * __bam_free --
 *	Add a page to the head of the freelist.
 *
 * PUBLIC: int __bam_free __P((DBC *, PAGE *));
 */
int
__bam_free(dbc, h)
	DBC *dbc;
	PAGE *h;
{
	BTMETA *meta;
	DB *dbp;
	DBT ldbt;
	DB_LOCK metalock;
	db_pgno_t pgno;
	u_int32_t dirty_flag;
	int ret, t_ret;

	dbp = dbc->dbp;

	/*
	 * Retrieve the metadata page and insert the page at the head of
	 * the free list.  If either the lock get or page get routines
	 * fail, then we need to put the page with which we were called
	 * back because our caller assumes we take care of it.
	 */
	dirty_flag = 0;
	pgno = PGNO_METADATA;
	if ((ret = __bam_lget(dbc, 0, pgno, DB_LOCK_WRITE, &metalock)) != 0)
		goto err;
	if ((ret = memp_fget(dbp->mpf, &pgno, 0, (PAGE **)&meta)) != 0) {
		(void)__BT_TLPUT(dbc, metalock);
		goto err;
	}

	/* Log the change. */
	if (DB_LOGGING(dbc)) {
		memset(&ldbt, 0, sizeof(ldbt));
		ldbt.data = h;
		ldbt.size = P_OVERHEAD;
		if ((ret = __bam_pg_free_log(dbp->dbenv->lg_info,
		    dbc->txn, &meta->lsn, 0, dbp->log_fileid, h->pgno,
		    &meta->lsn, &ldbt, meta->free)) != 0) {
			(void)memp_fput(dbp->mpf, (PAGE *)meta, 0);
			(void)__BT_TLPUT(dbc, metalock);
			return (ret);
		}
		LSN(h) = LSN(meta);
	}

	/*
	 * The page should have nothing interesting on it, re-initialize it,
	 * leaving only the page number and the LSN.
	 */
#ifdef DIAGNOSTIC
	{ db_pgno_t __pgno; DB_LSN __lsn;
		__pgno = h->pgno;
		__lsn = h->lsn;
		memset(h, 0xdb, dbp->pgsize);
		h->pgno = __pgno;
		h->lsn = __lsn;
	}
#endif
	P_INIT(h, dbp->pgsize, h->pgno, PGNO_INVALID, meta->free, 0, P_INVALID);

	/* Link the page on the metadata free list. */
	meta->free = h->pgno;

	/* Discard the metadata page. */
	ret = memp_fput(dbp->mpf, (PAGE *)meta, DB_MPOOL_DIRTY);
	if ((t_ret = __BT_TLPUT(dbc, metalock)) != 0)
		ret = t_ret;

	/* Discard the caller's page reference. */
	dirty_flag = DB_MPOOL_DIRTY;
err:	if ((t_ret = memp_fput(dbp->mpf, h, dirty_flag)) != 0 && ret == 0)
		ret = t_ret;

	/*
	 * XXX
	 * We have to unlock the caller's page in the caller!
	 */
	return (ret);
}

#ifdef DEBUG
/*
 * __bam_lt --
 *	Print out the list of locks currently held by a cursor.
 *
 * PUBLIC: int __bam_lt __P((DBC *));
 */
int
__bam_lt(dbc)
	DBC *dbc;
{
	DB *dbp;
	DB_LOCKREQ req;

	dbp = dbc->dbp;
	if (F_ISSET(dbp, DB_AM_LOCKING)) {
		req.op = DB_LOCK_DUMP;
		lock_vec(dbp->dbenv->lk_info, dbc->locker, 0, &req, 1, NULL);
	}
	return (0);
}
#endif

/*
 * __bam_lget --
 *	The standard lock get call.
 *
 * PUBLIC: int __bam_lget
 * PUBLIC:    __P((DBC *, int, db_pgno_t, db_lockmode_t, DB_LOCK *));
 */
int
__bam_lget(dbc, do_couple, pgno, mode, lockp)
	DBC *dbc;
	int do_couple;
	db_pgno_t pgno;
	db_lockmode_t mode;
	DB_LOCK *lockp;
{
	DB *dbp;
	DB_LOCKREQ couple[2];
	int ret;

	dbp = dbc->dbp;

	if (!F_ISSET(dbp, DB_AM_LOCKING)) {
		*lockp = LOCK_INVALID;
		return (0);
	}

	dbc->lock.pgno = pgno;

	/*
	 * If the object not currently locked, acquire the lock and return,
	 * otherwise, lock couple.  If we fail and it's not a system error,
	 * convert to EAGAIN.
	 */
	if (do_couple) {
		couple[0].op = DB_LOCK_GET;
		couple[0].obj = &dbc->lock_dbt;
		couple[0].mode = mode;
		couple[1].op = DB_LOCK_PUT;
		couple[1].lock = *lockp;

		if (dbc->txn == NULL)
			ret = lock_vec(dbp->dbenv->lk_info,
			    dbc->locker, 0, couple, 2, NULL);
		else
			ret = lock_tvec(dbp->dbenv->lk_info,
			    dbc->txn, 0, couple, 2, NULL);
		if (ret != 0) {
			/* If we fail, discard the lock we held. */
			__BT_LPUT(dbc, *lockp);

			return (ret < 0 ? EAGAIN : ret);
		}
		*lockp = couple[0].lock;
	} else {
		if (dbc->txn == NULL)
			ret = lock_get(dbp->dbenv->lk_info,
			    dbc->locker, 0, &dbc->lock_dbt, mode, lockp);
		else
			ret = lock_tget(dbp->dbenv->lk_info,
			    dbc->txn, 0, &dbc->lock_dbt, mode, lockp);
		 return (ret < 0 ? EAGAIN : ret);
	}
	return (0);
}
