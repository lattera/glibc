/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)bt_cursor.c	10.27 (Sleepycat) 9/3/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"

static int __bam_c_close __P((DBC *));
static int __bam_c_del __P((DBC *, int));
static int __bam_c_first __P((DB *, CURSOR *));
static int __bam_c_get __P((DBC *, DBT *, DBT *, int));
static int __bam_c_last __P((DB *, CURSOR *));
static int __bam_c_next __P((DB *, CURSOR *, int));
static int __bam_c_physdel __P((DB *, CURSOR *, PAGE *));
static int __bam_c_prev __P((DB *, CURSOR *));
static int __bam_c_put __P((DBC *, DBT *, DBT *, int));
static int __bam_c_rget __P((DB *, CURSOR *, DBT *, DBT *, int));
static int __bam_c_search __P((DB *, CURSOR *, const DBT *, u_int, int, int *));

/* Discard the current page/lock held by a cursor. */
#undef	DISCARD
#define	DISCARD(dbp, cp) {						\
	(void)memp_fput(dbp->mpf, (cp)->page, 0);			\
	(cp)->page = NULL;						\
	(void)__BT_TLPUT((dbp), (cp)->lock);				\
	(cp)->lock = LOCK_INVALID;					\
}

/*
 * __bam_cursor --
 *	Interface to the cursor functions.
 *
 * PUBLIC: int __bam_cursor __P((DB *, DB_TXN *, DBC **));
 */
int
__bam_cursor(dbp, txn, dbcp)
	DB *dbp;
	DB_TXN *txn;
	DBC **dbcp;
{
	CURSOR *cp;
	DBC *dbc;

	DEBUG_LWRITE(dbp, txn, "bam_cursor", NULL, NULL, 0);

	if ((dbc = (DBC *)calloc(1, sizeof(DBC))) == NULL)
		return (ENOMEM);
	if ((cp = (CURSOR *)calloc(1, sizeof(CURSOR))) == NULL) {
		free(dbc);
		return (ENOMEM);
	}

	cp->dbc = dbc;
	cp->pgno = cp->dpgno = PGNO_INVALID;
	cp->lock = LOCK_INVALID;

	dbc->dbp = dbp;
	dbc->txn = txn;
	dbc->internal = cp;
	dbc->c_close = __bam_c_close;
	dbc->c_del = __bam_c_del;
	dbc->c_get = __bam_c_get;
	dbc->c_put = __bam_c_put;

	/* All cursor structures hang off the main DB structure. */
	DB_THREAD_LOCK(dbp);
	TAILQ_INSERT_HEAD(&dbp->curs_queue, dbc, links);
	DB_THREAD_UNLOCK(dbp);

	*dbcp = dbc;
	return (0);
}

/*
 * __bam_c_close --
 *	Close a single cursor.
 */
static int
__bam_c_close(dbc)
	DBC *dbc;
{
	DB *dbp;
	CURSOR *cp;
	int ret;

	DEBUG_LWRITE(dbc->dbp, dbc->txn, "bam_c_close", NULL, NULL, 0);

	GETHANDLE(dbc->dbp, dbc->txn, &dbp, ret);
	cp = dbc->internal;

	/* If a cursor key was deleted do the actual deletion.  */
	ret = F_ISSET(cp, C_DELETED) ?  __bam_c_physdel(dbp, cp, NULL) : 0;

	/* Discard any lock if we're not inside a transaction. */
	if (dbp->txn == NULL && cp->lock != LOCK_INVALID)
		(void)__BT_TLPUT(dbp, cp->lock);

	/* Remove the cursor from the queue. */
	DB_THREAD_LOCK(dbp);
	TAILQ_REMOVE(&dbp->curs_queue, dbc, links);
	DB_THREAD_UNLOCK(dbp);

	/* Discard the structures. */
	FREE(cp, sizeof(CURSOR));
	FREE(dbc, sizeof(DBC));

	PUTHANDLE(dbp);
	return (ret);
}

/*
 * __bam_c_del --
 *	Delete using a cursor.
 */
static int
__bam_c_del(dbc, flags)
	DBC *dbc;
	int flags;
{
	CURSOR *cp;
	DB *dbp;
	DB_LOCK lock;
	PAGE *h;
	db_pgno_t pgno;
	db_indx_t indx;
	int ret;

	DEBUG_LWRITE(dbc->dbp, dbc->txn, "bam_c_del", NULL, NULL, flags);

	cp = dbc->internal;

	/* Check for invalid flags. */
	if ((ret = __db_cdelchk(dbc->dbp, flags,
	    F_ISSET(dbc->dbp, DB_AM_RDONLY), cp->pgno != PGNO_INVALID)) != 0)
		return (ret);

	/* If already deleted, return failure. */
	if (F_ISSET(cp, C_DELETED | C_REPLACE))
		return (DB_KEYEMPTY);

	GETHANDLE(dbc->dbp, dbc->txn, &dbp, ret);

	/*
	 * We don't physically delete the record until the cursor moves,
	 * so we have to have a long-lived write lock on the page instead
	 * of a long-lived read lock.  Note, we have to have a read lock
	 * to even get here, so we simply discard it.
	 */
	if (F_ISSET(dbp, DB_AM_LOCKING) && cp->mode != DB_LOCK_WRITE) {
		if ((ret = __bam_lget(dbp,
		    0, cp->pgno, DB_LOCK_WRITE, &lock)) != 0)
			goto err;
		(void)__BT_TLPUT(dbp, cp->lock);
		cp->lock = lock;
		cp->mode = DB_LOCK_WRITE;
	}

	/*
	 * Acquire the underlying page (which may be different from the above
	 * page because it may be a duplicate page), and set the on-page and
	 * in-cursor delete flags.  We don't need to lock it as we've already
	 * write-locked the page leading to it.
	 */
	if (cp->dpgno == PGNO_INVALID) {
		pgno = cp->pgno;
		indx = cp->indx;
	} else {
		pgno = cp->dpgno;
		indx = cp->dindx;
	}

	if ((ret = __bam_pget(dbp, &h, &pgno, 0)) != 0)
		goto err;

	/* Log the change. */
	if (DB_LOGGING(dbp) &&
	    (ret = __bam_cdel_log(dbp->dbenv->lg_info, dbp->txn, &LSN(h),
	    0, dbp->log_fileid, PGNO(h), &LSN(h), indx)) != 0) {
		(void)memp_fput(dbp->mpf, h, 0);
		goto err;
	}

	/* Set the intent-to-delete flag on the page and in all cursors. */
	if (cp->dpgno == PGNO_INVALID)
		B_DSET(GET_BKEYDATA(h, indx + O_INDX)->type);
	else
		B_DSET(GET_BKEYDATA(h, indx)->type);
	(void)__bam_ca_delete(dbp, pgno, indx, NULL);

	ret = memp_fput(dbp->mpf, h, DB_MPOOL_DIRTY);

err:	PUTHANDLE(dbp);
	return (ret);
}

/*
 * __bam_get --
 *	Retrieve a key/data pair from the tree.
 *
 * PUBLIC: int __bam_get __P((DB *, DB_TXN *, DBT *, DBT *, int));
 */
int
__bam_get(argdbp, txn, key, data, flags)
	DB *argdbp;
	DB_TXN *txn;
	DBT *key, *data;
	int flags;
{
	DBC dbc;
	CURSOR cp;
	int ret;

	DEBUG_LREAD(argdbp, txn, "bam_get", key, NULL, flags);

	/* Check for invalid flags. */
	if ((ret = __db_getchk(argdbp, key, data, flags)) != 0)
		return (ret);

	/* Build a cursor. */
	memset(&cp, 0, sizeof(cp));
	cp.dbc = &dbc;
	cp.pgno = cp.dpgno = PGNO_INVALID;
	cp.lock = LOCK_INVALID;

	memset(&dbc, 0, sizeof(dbc));
	dbc.dbp = argdbp;
	dbc.txn = txn;
	dbc.internal = &cp;

	/* Get the key. */
	if ((ret = __bam_c_get(&dbc,
	    key, data, LF_ISSET(DB_SET_RECNO) ? DB_SET_RECNO : DB_SET)) != 0)
		return (ret);

	/* Discard any lock, the cursor didn't really exist. */
	if (cp.lock != LOCK_INVALID)
		(void)__BT_TLPUT(argdbp, cp.lock);

	return (0);
}

/*
 * __bam_c_get --
 *	Get using a cursor (btree).
 */
static int
__bam_c_get(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	int flags;
{
	BTREE *t;
	CURSOR *cp, copy;
	DB *dbp;
	PAGE *h;
	int exact, ret;

	DEBUG_LREAD(dbc->dbp, dbc->txn, "bam_c_get",
	    flags == DB_SET || flags == DB_SET_RANGE ? key : NULL,
	    NULL, flags);

	cp = dbc->internal;

	/* Check for invalid flags. */
	if ((ret = __db_cgetchk(dbc->dbp,
	    key, data, flags, cp->pgno != PGNO_INVALID)) != 0)
		return (ret);

	GETHANDLE(dbc->dbp, dbc->txn, &dbp, ret);
	t = dbp->internal;

	/*
	 * Break out the code to return a cursor's record number.  It
	 * has nothing to do with the cursor get code except that it's
	 * been rammed into the interface.
	 */
	if (LF_ISSET(DB_GET_RECNO)) {
		ret = __bam_c_rget(dbp, cp, key, data, flags);
		PUTHANDLE(dbp);
		return (ret);
	}

	/* Initialize the cursor for a new retrieval. */
	copy = *cp;
	cp->page = NULL;
	cp->lock = LOCK_INVALID;

	switch (flags) {
	case DB_CURRENT:
		/* It's not possible to return a deleted record. */
		if (F_ISSET(cp, C_DELETED | C_REPLACE)) {
			PUTHANDLE(dbp);
			return (DB_KEYEMPTY);
		}

		/* Get the page with the current item on it. */
		if ((ret = __bam_pget(dbp, &cp->page, &cp->pgno, 0)) != 0)
			goto err;
		break;
	case DB_NEXT:
		if (cp->pgno != PGNO_INVALID) {
			if ((ret = __bam_c_next(dbp, cp, 1)) != 0)
				goto err;
			break;
		}
		/* FALLTHROUGH */
	case DB_FIRST:
		if ((ret = __bam_c_first(dbp, cp)) != 0)
			goto err;
		break;
	case DB_PREV:
		if (cp->pgno != PGNO_INVALID) {
			if ((ret = __bam_c_prev(dbp, cp)) != 0)
				goto err;
			break;
		}
		/* FALLTHROUGH */
	case DB_LAST:
		if ((ret = __bam_c_last(dbp, cp)) != 0)
			goto err;
		break;
	case DB_SET_RECNO:
		exact = 1;
		if ((ret =
		    __bam_c_search(dbp, cp, key, S_FIND, 1, &exact)) != 0)
			goto err;
		break;
	case DB_SET:
		exact = 1;
		if ((ret =
		    __bam_c_search(dbp, cp, key, S_FIND, 0, &exact)) != 0)
			goto err;
		break;
	case DB_SET_RANGE:
		exact = 0;
		if ((ret =
		    __bam_c_search(dbp, cp, key, S_FIND, 0, &exact)) != 0)
			goto err;
		break;
	}

	/*
	 * Return the key if the user didn't give us one.  If we've moved to
	 * a duplicate page, we may no longer have a pointer to the main page,
	 * so we have to go get it.  We know that it's already read-locked,
	 * however, so we don't have to acquire a new lock.
	 */
	if (flags != DB_SET) {
		if (cp->dpgno != PGNO_INVALID) {
			if ((ret = __bam_pget(dbp, &h, &cp->pgno, 0)) != 0)
				goto err;
		} else
			h = cp->page;
		ret = __db_ret(dbp,
		    h, cp->indx, key, &t->bt_rkey.data, &t->bt_rkey.ulen);
		if (cp->dpgno != PGNO_INVALID)
			(void)memp_fput(dbp->mpf, h, 0);
		if (ret)
			goto err;
	}

	/* Return the data. */
	if ((ret = __db_ret(dbp, cp->page,
	    cp->dpgno == PGNO_INVALID ? cp->indx + O_INDX : cp->dindx,
	    data, &t->bt_rdata.data, &t->bt_rdata.ulen)) != 0)
		goto err;

	/*
	 * If the previous cursor record has been deleted, delete it.  The
	 * returned key isn't a deleted key, so clear the flag.
	 */
	if (F_ISSET(&copy, C_DELETED) && __bam_c_physdel(dbp, &copy, cp->page))
		goto err;
	F_CLR(cp, C_DELETED | C_REPLACE);

	/* Release the previous lock, if any. */
	if (copy.lock != LOCK_INVALID)
		(void)__BT_TLPUT(dbp, copy.lock);

	/* Release the pinned page. */
	ret = memp_fput(dbp->mpf, cp->page, 0);

	++t->lstat.bt_get;

	if (0) {
err:		if (cp->page != NULL)
			(void)memp_fput(dbp->mpf, cp->page, 0);
		if (cp->lock != LOCK_INVALID)
			(void)__BT_TLPUT(dbp, cp->lock);
		*cp = copy;
	}

	PUTHANDLE(dbp);
	return (ret);
}

/*
 * __bam_c_rget --
 *	Return the record number for a cursor.
 */
static int
__bam_c_rget(dbp, cp, key, data, flags)
	DB *dbp;
	CURSOR *cp;
	DBT *key, *data;
	int flags;
{
	BTREE *t;
	DBT dbt;
	db_recno_t recno;
	int exact, ret;

	/* Get the page with the current item on it. */
	if ((ret = __bam_pget(dbp, &cp->page, &cp->pgno, 0)) != 0)
		return (ret);

	/* Get a copy of the key. */
	memset(&dbt, 0, sizeof(DBT));
	dbt.flags = DB_DBT_MALLOC | DB_DBT_INTERNAL;
	if ((ret = __db_ret(dbp, cp->page, cp->indx, &dbt, NULL, NULL)) != 0)
		goto err;

	exact = 1;
	if ((ret = __bam_search(dbp, &dbt, S_FIND, 1, &recno, &exact)) != 0)
		goto err;

	t = dbp->internal;
	ret = __db_retcopy(data, &recno, sizeof(recno),
	    &t->bt_rdata.data, &t->bt_rdata.ulen, dbp->db_malloc);

	/* Release the stack. */
	__bam_stkrel(dbp);

err:	(void)memp_fput(dbp->mpf, cp->page, 0);
	free(dbt.data);
	return (ret);
}

/*
 * __bam_c_put --
 *	Put using a cursor.
 */
static int
__bam_c_put(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	int flags;
{
	BTREE *t;
	CURSOR *cp, copy;
	DB *dbp;
	DBT dbt;
	db_indx_t indx;
	db_pgno_t pgno;
	int exact, needkey, ret;
	void *arg;

	DEBUG_LWRITE(dbc->dbp, dbc->txn, "bam_c_put",
	    flags == DB_KEYFIRST || flags == DB_KEYLAST ? key : NULL,
	    data, flags);

	cp = dbc->internal;

	if ((ret = __db_cputchk(dbc->dbp, key, data, flags,
	    F_ISSET(dbc->dbp, DB_AM_RDONLY), cp->pgno != PGNO_INVALID)) != 0)
		return (ret);

	GETHANDLE(dbc->dbp, dbc->txn, &dbp, ret);
	t = dbp->internal;

	/* Initialize the cursor for a new retrieval. */
	copy = *cp;
	cp->page = NULL;
	cp->lock = LOCK_INVALID;

	/*
	 * To split, we need a valid key for the page.  Since it's a cursor,
	 * we have to build one.
	 */
	if (0) {
split:		if (needkey) {
			memset(&dbt, 0, sizeof(DBT));
			ret = __db_ret(dbp, cp->page, indx,
			    &dbt, &t->bt_rkey.data, &t->bt_rkey.ulen);

			DISCARD(dbp, cp);

			if (ret)
				goto err;
			arg = &dbt;
		} else {
			(void)__bam_stkrel(dbp);
			arg = key;
		}
		if ((ret = __bam_split(dbp, arg)) != 0)
			goto err;
	}

	/* If there's no key supplied, use the cursor. */
	if (flags == DB_KEYFIRST || flags == DB_KEYLAST)
		needkey = 0;
	else {
		needkey = 1;
		if (cp->dpgno == PGNO_INVALID) {
			pgno = cp->pgno;
			indx = cp->indx;
		} else {
			pgno = cp->dpgno;
			indx = cp->dindx;
		}
		/* Acquire the current page. */
		if ((ret = __bam_lget(dbp,
		    0, cp->pgno, DB_LOCK_WRITE, &cp->lock)) != 0)
			goto err;
		if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
			goto err;
	}

	ret = 0;
	switch (flags) {
	case DB_AFTER:
	case DB_BEFORE:
	case DB_CURRENT:
		if ((ret = __bam_iitem(dbp, &cp->page,
		    &indx, key, data, flags, 0)) == DB_NEEDSPLIT)
			goto split;
		break;
	case DB_KEYFIRST:
		exact = 0;
		if ((ret =
		    __bam_c_search(dbp, cp, key, S_KEYFIRST, 0, &exact)) != 0)
			goto err;

		indx = cp->dpgno == PGNO_INVALID ? cp->indx : cp->dindx;
		if ((ret = __bam_iitem(dbp, &cp->page, &indx, key,
		    data, DB_BEFORE, exact ? 0 : BI_NEWKEY)) == DB_NEEDSPLIT)
			goto split;
		if (ret)
			goto err;
		break;
	case DB_KEYLAST:
		exact = 0;
		if ((ret =
		    __bam_c_search(dbp, cp, key, S_KEYLAST, 0, &exact)) != 0)
			goto err;

		indx = cp->dpgno == PGNO_INVALID ? cp->indx : cp->dindx;
		if ((ret = __bam_iitem(dbp, &cp->page, &indx, key,
		    data, DB_AFTER, exact ? 0 : BI_NEWKEY)) == DB_NEEDSPLIT)
			goto split;
		break;
	}
	if (ret)
		goto err;

	/*
	 * Update the cursor to point to the new entry.  The new entry was
	 * stored on the current page, because we split pages until it was
	 * possible.
	 */
	if (cp->dpgno == PGNO_INVALID)
		cp->indx = indx;
	else
		cp->dindx = indx;

	/*
	 * If the previous cursor record has been deleted, delete it.  The
	 * returned key isn't a deleted key, so clear the flag.
	 */
	if (F_ISSET(&copy, C_DELETED) &&
	    (ret = __bam_c_physdel(dbp, &copy, cp->page)) != 0)
		goto err;
	F_CLR(cp, C_DELETED | C_REPLACE);

	/* Release the previous lock, if any. */
	if (copy.lock != LOCK_INVALID)
		(void)__BT_TLPUT(dbp, copy.lock);

	/* Discard the pinned page. */
	ret = memp_fput(dbp->mpf, cp->page, 0);
	if (0) {
err:		if (cp->page != NULL)
			(void)memp_fput(dbp->mpf, cp->page, 0);
		if (cp->lock != LOCK_INVALID)
			(void)__BT_TLPUT(dbp, cp->lock);
		*cp = copy;
	}

	PUTHANDLE(dbp);
	return (ret);
}

/*
 * __bam_c_first --
 *	Return the first record.
 */
static int
__bam_c_first(dbp, cp)
	DB *dbp;
	CURSOR *cp;
{
	db_pgno_t pgno;
	int ret;

	/* Walk down the left-hand side of the tree. */
	for (pgno = PGNO_ROOT;;) {
		if ((ret =
		    __bam_lget(dbp, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
			return (ret);

		/* If we find a leaf page, we're done. */
		if (ISLEAF(cp->page))
			break;

		pgno = GET_BINTERNAL(cp->page, 0)->pgno;
		DISCARD(dbp, cp);
	}

	cp->pgno = cp->page->pgno;
	cp->indx = 0;
	cp->dpgno = PGNO_INVALID;

	/* If it's an empty page or a deleted record, go to the next one. */
	if (NUM_ENT(cp->page) == 0 ||
	    B_DISSET(GET_BKEYDATA(cp->page, cp->indx + O_INDX)->type))
		if ((ret = __bam_c_next(dbp, cp, 0)) != 0)
			return (ret);

	/* If it's a duplicate reference, go to the first entry. */
	if ((ret = __bam_ovfl_chk(dbp, cp, O_INDX, 0)) != 0)
		return (ret);

	/* If it's a deleted record, go to the next one. */
	if (cp->dpgno != PGNO_INVALID &&
	    B_DISSET(GET_BKEYDATA(cp->page, cp->dindx)->type))
		if ((ret = __bam_c_next(dbp, cp, 0)) != 0)
			return (ret);
	return (0);
}

/*
 * __bam_c_last --
 *	Return the last record.
 */
static int
__bam_c_last(dbp, cp)
	DB *dbp;
	CURSOR *cp;
{
	db_pgno_t pgno;
	int ret;

	/* Walk down the right-hand side of the tree. */
	for (pgno = PGNO_ROOT;;) {
		if ((ret =
		    __bam_lget(dbp, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
			return (ret);

		/* If we find a leaf page, we're done. */
		if (ISLEAF(cp->page))
			break;

		pgno =
		    GET_BINTERNAL(cp->page, NUM_ENT(cp->page) - O_INDX)->pgno;
		DISCARD(dbp, cp);
	}

	cp->pgno = cp->page->pgno;
	cp->indx = NUM_ENT(cp->page) == 0 ? 0 : NUM_ENT(cp->page) - P_INDX;
	cp->dpgno = PGNO_INVALID;

	/* If it's an empty page or a deleted record, go to the previous one. */
	if (NUM_ENT(cp->page) == 0 ||
	    B_DISSET(GET_BKEYDATA(cp->page, cp->indx + O_INDX)->type))
		if ((ret = __bam_c_prev(dbp, cp)) != 0)
			return (ret);

	/* If it's a duplicate reference, go to the last entry. */
	if ((ret = __bam_ovfl_chk(dbp, cp, cp->indx + O_INDX, 1)) != 0)
		return (ret);

	/* If it's a deleted record, go to the previous one. */
	if (cp->dpgno != PGNO_INVALID &&
	    B_DISSET(GET_BKEYDATA(cp->page, cp->dindx)->type))
		if ((ret = __bam_c_prev(dbp, cp)) != 0)
			return (ret);
	return (0);
}

/*
 * __bam_c_next --
 *	Move to the next record.
 */
static int
__bam_c_next(dbp, cp, initial_move)
	DB *dbp;
	CURSOR *cp;
	int initial_move;
{
	db_indx_t adjust, indx;
	db_pgno_t pgno;
	int ret;

	/*
	 * We're either moving through a page of duplicates or a btree leaf
	 * page.
	 */
	if (cp->dpgno == PGNO_INVALID) {
		adjust = dbp->type == DB_BTREE ? P_INDX : O_INDX;
		pgno = cp->pgno;
		indx = cp->indx;
	} else {
		adjust = O_INDX;
		pgno = cp->dpgno;
		indx = cp->dindx;
	}
	if (cp->page == NULL) {
		if ((ret =
		    __bam_lget(dbp, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
			return (ret);
	}

	/*
	 * If at the end of the page, move to a subsequent page.
	 *
	 * !!!
	 * Check for >= NUM_ENT.  If we're here as the result of a search that
	 * landed us on NUM_ENT, we'll increment indx before we test.
	 *
	 * !!!
	 * This code handles empty pages and pages with only deleted entries.
	 */
	if (initial_move)
		indx += adjust;
	for (;;) {
		if (indx >= NUM_ENT(cp->page)) {
			pgno = cp->page->next_pgno;
			DISCARD(dbp, cp);

			/*
			 * If we're in a btree leaf page, we've reached the end
			 * of the tree.  If we've reached the end of a page of
			 * duplicates, continue from the btree leaf page where
			 * we found this page of duplicates.
			 */
			if (pgno == PGNO_INVALID) {
				/* If in a btree leaf page, it's EOF. */
				if (cp->dpgno == PGNO_INVALID)
					return (DB_NOTFOUND);

				/* Continue from the last btree leaf page. */
				cp->dpgno = PGNO_INVALID;

				adjust = P_INDX;
				pgno = cp->pgno;
				indx = cp->indx + P_INDX;
			} else
				indx = 0;

			if ((ret = __bam_lget(dbp,
			    0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
				return (ret);
			if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
				return (ret);
			continue;
		}

		/* Ignore deleted records. */
		if (dbp->type == DB_BTREE &&
		    ((cp->dpgno == PGNO_INVALID &&
		    B_DISSET(GET_BKEYDATA(cp->page, indx + O_INDX)->type)) ||
		    (cp->dpgno != PGNO_INVALID &&
		    B_DISSET(GET_BKEYDATA(cp->page, indx)->type)))) {
			indx += adjust;
			continue;
		}

		/*
		 * If we're not in a duplicates page, check to see if we've
		 * found a page of duplicates, in which case we move to the
		 * first entry.
		 */
		if (cp->dpgno == PGNO_INVALID) {
			cp->pgno = cp->page->pgno;
			cp->indx = indx;

			if ((ret =
			    __bam_ovfl_chk(dbp, cp, indx + O_INDX, 0)) != 0)
				return (ret);
			if (cp->dpgno != PGNO_INVALID) {
				indx = cp->dindx;
				adjust = O_INDX;
				continue;
			}
		} else {
			cp->dpgno = cp->page->pgno;
			cp->dindx = indx;
		}
		break;
	}
	return (0);
}

/*
 * __bam_c_prev --
 *	Move to the previous record.
 */
static int
__bam_c_prev(dbp, cp)
	DB *dbp;
	CURSOR *cp;
{
	db_indx_t indx, adjust;
	db_pgno_t pgno;
	int ret, set_indx;

	/*
	 * We're either moving through a page of duplicates or a btree leaf
	 * page.
	 */
	if (cp->dpgno == PGNO_INVALID) {
		adjust = dbp->type == DB_BTREE ? P_INDX : O_INDX;
		pgno = cp->pgno;
		indx = cp->indx;
	} else {
		adjust = O_INDX;
		pgno = cp->dpgno;
		indx = cp->dindx;
	}
	if (cp->page == NULL) {
		if ((ret =
		    __bam_lget(dbp, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
			return (ret);
	}

	/*
	 * If at the beginning of the page, move to any previous one.
	 *
	 * !!!
         * This code handles empty pages and pages with only deleted entries.
	 */
	for (;;) {
		if (indx == 0) {
			pgno = cp->page->prev_pgno;
			DISCARD(dbp, cp);

			/*
			 * If we're in a btree leaf page, we've reached the
			 * beginning of the tree.  If we've reached the first
			 * of a page of duplicates, continue from the btree
			 * leaf page where we found this page of duplicates.
			 */
			if (pgno == PGNO_INVALID) {
				/* If in a btree leaf page, it's SOF. */
				if (cp->dpgno == PGNO_INVALID)
					return (DB_NOTFOUND);

				/* Continue from the last btree leaf page. */
				cp->dpgno = PGNO_INVALID;

				adjust = P_INDX;
				pgno = cp->pgno;
				indx = cp->indx;
				set_indx = 0;
			} else
				set_indx = 1;

			if ((ret = __bam_lget(dbp,
			    0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
				return (ret);
			if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
				return (ret);

			if (set_indx)
				indx = NUM_ENT(cp->page);
			if (indx == 0)
				continue;
		}

		/* Ignore deleted records. */
		indx -= adjust;
		if (dbp->type == DB_BTREE &&
		    ((cp->dpgno == PGNO_INVALID &&
		    B_DISSET(GET_BKEYDATA(cp->page, indx + O_INDX)->type)) ||
		    (cp->dpgno != PGNO_INVALID &&
		    B_DISSET(GET_BKEYDATA(cp->page, indx)->type))))
			continue;

		/*
		 * If we're not in a duplicates page, check to see if we've
		 * found a page of duplicates, in which case we move to the
		 * last entry.
		 */
		if (cp->dpgno == PGNO_INVALID) {
			cp->pgno = cp->page->pgno;
			cp->indx = indx;

			if ((ret =
			    __bam_ovfl_chk(dbp, cp, indx + O_INDX, 1)) != 0)
				return (ret);
			if (cp->dpgno != PGNO_INVALID) {
				indx = cp->dindx + O_INDX;
				adjust = O_INDX;
				continue;
			}
		} else {
			cp->dpgno = cp->page->pgno;
			cp->dindx = indx;
		}
		break;
	}
	return (0);
}

/*
 * __bam_c_search --
 *	Move to a specified record.
 */
static int
__bam_c_search(dbp, cp, key, flags, isrecno, exactp)
	DB *dbp;
	CURSOR *cp;
	const DBT *key;
	u_int flags;
	int isrecno, *exactp;
{
	BTREE *t;
	db_recno_t recno;
	int needexact, ret;

	t = dbp->internal;
	needexact = *exactp;

	/*
	 * Find any matching record; the search function pins the page.  Make
	 * sure it's a valid key (__bam_search may return an index just past
	 * the end of a page) and return it.
	 */
	if (isrecno) {
		if ((ret = __ram_getno(dbp, key, &recno, 0)) != 0)
			return (ret);
		ret = __bam_rsearch(dbp, &recno, flags, 1, exactp);
	} else
		ret = __bam_search(dbp, key, flags, 1, NULL, exactp);
	if (ret != 0)
		return (ret);

	cp->page = t->bt_csp->page;
	cp->pgno = cp->page->pgno;
	cp->indx = t->bt_csp->indx;
	cp->lock = t->bt_csp->lock;
	cp->dpgno = PGNO_INVALID;

	/*
	 * If we have an exact match, make sure that we're not looking at a
	 * chain of duplicates -- if so, move to an entry in that chain.
	 */
	if (*exactp) {
		if ((ret = __bam_ovfl_chk(dbp,
		    cp, cp->indx + O_INDX, LF_ISSET(S_DUPLAST))) != 0)
			return (ret);
	} else
		if (needexact)
			return (DB_NOTFOUND);

	/* If past the end of a page, find the next entry. */
	if (cp->indx == NUM_ENT(cp->page) &&
	    (ret = __bam_c_next(dbp, cp, 0)) != 0)
		return (ret);

	/* If it's a deleted record, go to the next or previous one. */
	if (cp->dpgno != PGNO_INVALID &&
	    B_DISSET(GET_BKEYDATA(cp->page, cp->dindx)->type))
		if (flags == S_KEYLAST) {
			if ((ret = __bam_c_prev(dbp, cp)) != 0)
				return (ret);
		} else
			if ((ret = __bam_c_next(dbp, cp, 0)) != 0)
				return (ret);
	return (0);
}

/*
 * __bam_ovfl_chk --
 *	Check for an overflow record, and if found, move to the correct
 *	record.
 *
 * PUBLIC: int __bam_ovfl_chk __P((DB *, CURSOR *, u_int32_t, int));
 */
int
__bam_ovfl_chk(dbp, cp, indx, to_end)
	DB *dbp;
	CURSOR *cp;
	u_int32_t indx;
	int to_end;
{
	BOVERFLOW *bo;
	db_pgno_t pgno;
	int ret;

	/* Check for an overflow entry. */
	bo = GET_BOVERFLOW(cp->page, indx);
	if (B_TYPE(bo->type) != B_DUPLICATE)
		return (0);

	/*
	 * If we find one, go to the duplicates page, and optionally move
	 * to the last record on that page.
	 *
	 * XXX
	 * We don't lock duplicates pages, we've already got the correct
	 * lock on the main page.
	 */
	pgno = bo->pgno;
	if ((ret = memp_fput(dbp->mpf, cp->page, 0)) != 0)
		return (ret);
	cp->page = NULL;
	if (to_end) {
		if ((ret = __db_dend(dbp, pgno, &cp->page)) != 0)
			return (ret);
		indx = NUM_ENT(cp->page) - O_INDX;
	} else {
		if ((ret = __bam_pget(dbp, &cp->page, &pgno, 0)) != 0)
			return (ret);
		indx = 0;
	}

	/* Update the duplicate entry in the cursor. */
	cp->dpgno = cp->page->pgno;
	cp->dindx = indx;

	return (0);
}

#ifdef DEBUG
/*
 * __bam_cprint --
 *	Display the current btree cursor list.
 */
int
__bam_cprint(dbp)
	DB *dbp;
{
	CURSOR *cp;
	DBC *dbc;

	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->curs_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		fprintf(stderr,
		    "%#0x: page: %lu index: %lu dpage %lu dindex: %lu",
		    (u_int)cp, (u_long)cp->pgno, (u_long)cp->indx,
		    (u_long)cp->dpgno, (u_long)cp->dindx);
		if (F_ISSET(cp, C_DELETED))
			fprintf(stderr, "(deleted)");
		fprintf(stderr, "\n");
	}
	DB_THREAD_UNLOCK(dbp);
	return (0);
}
#endif /* DEBUG */

/*
 * __bam_ca_delete --
 * 	Check if any of the cursors refer to the item we are about to delete.
 *	We'll return the number of cursors that refer to the item in question.
 *	If a cursor does refer to the item, then we set its deleted bit.
 *
 * PUBLIC: int __bam_ca_delete __P((DB *, db_pgno_t, u_int32_t, CURSOR *));
 */
int
__bam_ca_delete(dbp, pgno, indx, curs)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	CURSOR *curs;
{
	DBC *dbc;
	CURSOR *cp;
	int count;

	/*
	 * Adjust the cursors.  We don't have to review the cursors for any
	 * process other than the current one, because we have the page write
	 * locked at this point, and any other process had better be using a
	 * different locker ID, meaning that only cursors in our process can
	 * be on the page.
	 *
	 * It's possible for multiple cursors within the thread to have write
	 * locks on the same page, but, cursors within a thread must be single
	 * threaded, so all we're locking here is the cursor linked list.
	 *
	 * indx refers to the first of what might be a duplicate set.  The
	 * cursor passed in is the one initiating the delete, so we don't
	 * want to count it.
	 */
	DB_THREAD_LOCK(dbp);
	for (count = 0, dbc = TAILQ_FIRST(&dbp->curs_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if ((curs != cp &&
		    cp->pgno == pgno && cp->indx == indx) ||
		    (cp->dpgno == pgno && cp->dindx == indx)) {
			++count;
			F_SET(cp, C_DELETED);
		}
	}
	DB_THREAD_UNLOCK(dbp);
	return (count);
}

/*
 * __bam_ca_di --
 *	Adjust the cursors during a delete or insert.
 *
 * PUBLIC: void __bam_ca_di __P((DB *, db_pgno_t, u_int32_t, int));
 */
void
__bam_ca_di(dbp, pgno, indx, value)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	int value;
{
	CURSOR *cp;
	DBC *dbc;

	/* Recno is responsible for its own adjustments. */
	if (dbp->type == DB_RECNO)
		return;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->curs_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if (cp->pgno == pgno && cp->indx >= indx)
			cp->indx += value;
		if (cp->dpgno == pgno && cp->dindx >= indx)
			cp->dindx += value;
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_dup --
 *	Adjust the cursors when moving data items to a duplicates page.
 *
 * PUBLIC: void __bam_ca_dup __P((DB *,
 * PUBLIC:    db_pgno_t, u_int32_t, u_int32_t, db_pgno_t, u_int32_t));
 */
void
__bam_ca_dup(dbp, fpgno, first, fi, tpgno, ti)
	DB *dbp;
	db_pgno_t fpgno, tpgno;
	u_int32_t first, fi, ti;
{
	CURSOR *cp;
	DBC *dbc;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * No need to test duplicates, this only gets called when moving
	 * leaf page data items onto a duplicates page.
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->curs_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		/*
		 * Ignore matching entries that have already been moved,
		 * we move from the same location on the leaf page more
		 * than once.
		 */
		if (cp->dpgno == PGNO_INVALID &&
		    cp->pgno == fpgno && cp->indx == fi) {
			cp->indx = first;
			cp->dpgno = tpgno;
			cp->dindx = ti;
		}
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_move --
 *	Adjust the cursors when moving data items to another page.
 *
 * PUBLIC: void __bam_ca_move __P((DB *, BTREE *, db_pgno_t, db_pgno_t));
 */
void
__bam_ca_move(dbp, t, fpgno, tpgno)
	DB *dbp;
	BTREE *t;
	db_pgno_t fpgno, tpgno;
{
	CURSOR *cp;
	DBC *dbc;

	/* Recno is responsible for its own adjustments. */
	if (dbp->type == DB_RECNO)
		return;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * No need to test duplicates, this only gets called when copying
	 * over the root page with a leaf or internal page.
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->curs_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if (cp->pgno == fpgno)
			cp->pgno = tpgno;
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_replace --
 * 	Check if any of the cursors refer to the item we are about to replace.
 *	If so, their flags should be changed from deleted to replaced.
 *
 * PUBLIC: void __bam_ca_replace
 * PUBLIC:    __P((DB *, db_pgno_t, u_int32_t, ca_replace_arg));
 */
void
__bam_ca_replace(dbp, pgno, indx, pass)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	ca_replace_arg pass;
{
	CURSOR *cp;
	DBC *dbc;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * Find any cursors that have logically deleted a record we're about
	 * to overwrite.
	 *
	 * Pass == REPLACE_SETUP:
	 *	Set C_REPLACE_SETUP so we can find the cursors again.
	 *
	 * Pass == REPLACE_SUCCESS:
	 *	Clear C_DELETED and C_REPLACE_SETUP, set C_REPLACE, the
	 *	overwrite was successful.
	 *
	 * Pass == REPLACE_FAILED:
	 *	Clear C_REPLACE_SETUP, the overwrite failed.
	 *
	 * For REPLACE_SUCCESS and REPLACE_FAILED, we reset the indx value
	 * for the cursor as it may have been changed by other cursor update
	 * routines as the item was deleted/inserted.
	 */
	DB_THREAD_LOCK(dbp);
	switch (pass) {
	case REPLACE_SETUP:			/* Setup. */
		for (dbc = TAILQ_FIRST(&dbp->curs_queue);
		    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
			cp = (CURSOR *)dbc->internal;
			if ((cp->pgno == pgno && cp->indx == indx) ||
			    (cp->dpgno == pgno && cp->dindx == indx))
				F_SET(cp, C_REPLACE_SETUP);
		}
		break;
	case REPLACE_SUCCESS:			/* Overwrite succeeded. */
		for (dbc = TAILQ_FIRST(&dbp->curs_queue);
		    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
			cp = (CURSOR *)dbc->internal;
			if (F_ISSET(cp, C_REPLACE_SETUP)) {
				if (cp->dpgno == pgno)
					cp->dindx = indx;
				if (cp->pgno == pgno)
					cp->indx = indx;
				F_SET(cp, C_REPLACE);
				F_CLR(cp, C_DELETED | C_REPLACE_SETUP);
			}
		}
		break;
	case REPLACE_FAILED:			/* Overwrite failed. */
		for (dbc = TAILQ_FIRST(&dbp->curs_queue);
		    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
			cp = (CURSOR *)dbc->internal;
			if (F_ISSET(cp, C_REPLACE_SETUP)) {
				if (cp->dpgno == pgno)
					cp->dindx = indx;
				if (cp->pgno == pgno)
					cp->indx = indx;
				F_CLR(cp, C_REPLACE_SETUP);
			}
		}
		break;
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_split --
 *	Adjust the cursors when splitting a page.
 *
 * PUBLIC: void __bam_ca_split __P((DB *,
 * PUBLIC:    db_pgno_t, db_pgno_t, db_pgno_t, u_int32_t, int));
 */
void
__bam_ca_split(dbp, ppgno, lpgno, rpgno, split_indx, cleft)
	DB *dbp;
	db_pgno_t ppgno, lpgno, rpgno;
	u_int32_t split_indx;
	int cleft;
{
	DBC *dbc;
	CURSOR *cp;

	/* Recno is responsible for its own adjustments. */
	if (dbp->type == DB_RECNO)
		return;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * If splitting the page that a cursor was on, the cursor has to be
	 * adjusted to point to the same record as before the split.  Most
	 * of the time we don't adjust pointers to the left page, because
	 * we're going to copy its contents back over the original page.  If
	 * the cursor is on the right page, it is decremented by the number of
	 * records split to the left page.
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->curs_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if (cp->pgno == ppgno)
			if (cp->indx < split_indx) {
				if (cleft)
					cp->pgno = lpgno;
			} else {
				cp->pgno = rpgno;
				cp->indx -= split_indx;
			}
		if (cp->dpgno == ppgno)
			if (cp->dindx < split_indx) {
				if (cleft)
					cp->dpgno = lpgno;
			} else {
				cp->dpgno = rpgno;
				cp->dindx -= split_indx;
			}
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_c_physdel --
 *	Actually do the cursor deletion.
 */
static int
__bam_c_physdel(dbp, cp, h)
	DB *dbp;
	CURSOR *cp;
	PAGE *h;
{
	BOVERFLOW bo;
	BTREE *t;
	DBT dbt;
	DB_LOCK lock;
	db_indx_t indx;
	db_pgno_t pgno, next_pgno, prev_pgno;
	int local, ret;

	t = dbp->internal;
	ret = 0;

	/* Figure out what we're deleting. */
	if (cp->dpgno == PGNO_INVALID) {
		pgno = cp->pgno;
		indx = cp->indx;
	} else {
		pgno = cp->dpgno;
		indx = cp->dindx;
	}

	/*
	 * If the item is referenced by another cursor, leave it up to that
	 * cursor to do the delete.
	 */
	if (__bam_ca_delete(dbp, pgno, indx, cp) != 0)
		return (0);

	/*
	 * If we don't already have the page locked, get it and delete the
	 * items.
	 */
	if ((h == NULL || h->pgno != pgno)) {
		if ((ret = __bam_lget(dbp, 0, pgno, DB_LOCK_WRITE, &lock)) != 0)
			return (ret);
		if ((ret = __bam_pget(dbp, &h, &pgno, 0)) != 0)
			return (ret);
		local = 1;
	} else
		local = 0;

	/*
	 * If we're deleting a duplicate entry, call the common code to do
	 * the work.
	 */
	if (TYPE(h) == P_DUPLICATE) {
		pgno = PGNO(h);
		prev_pgno = PREV_PGNO(h);
		next_pgno = NEXT_PGNO(h);
		if ((ret = __db_drem(dbp, &h, indx, __bam_free)) != 0)
			goto err;

		/*
		 * There are 4 cases:
		 *
		 * 1. We removed an item on a page, but there are more items
		 *    on the page.
		 * 2. We removed the last item on a page, removing the last
		 *    duplicate.
		 * 3. We removed the last item on a page, but there is a
		 *    following page of duplicates.
		 * 4. We removed the last item on a page, but there is a
		 *    previous page of duplicates.
		 *
		 * In case 1, h != NULL, h->pgno == pgno
		 * In case 2, h == NULL,
		 *    prev_pgno == PGNO_INVALID, next_pgno == PGNO_INVALID
		 * In case 3, h != NULL, next_pgno != PGNO_INVALID
		 * In case 4, h == NULL, prev_pgno != PGNO_INVALID
		 *
		 * In case 1, there's nothing else to do.
		 * In case 2, remove the entry from the parent page.
		 * In case 3 or 4, if the deleted page was the first in a chain
		 *    of duplicate pages, update the parent page's entry.
		 *
		 * Test:
		 *	If there were previous pages of duplicates or we didn't
		 *	empty the current page of duplicates, we don't need to
		 *	touch the parent page.
		 */
		if (PREV_PGNO(h) != PGNO_INVALID ||
		    (h != NULL && pgno == h->pgno))
			goto done;

		/*
		 * Release any page we're holding and the lock on the deleted
		 * page.
		 */
		if (local) {
			if (h != NULL)
				(void)memp_fput(dbp->mpf, h, 0);
			(void)__BT_TLPUT(dbp, lock);
			local = 0;
		}

		/* Acquire the parent page. */
		if ((ret =
		    __bam_lget(dbp, 0, cp->pgno, DB_LOCK_WRITE, &lock)) != 0)
			goto err;
		if ((ret = __bam_pget(dbp, &h, &cp->pgno, 0)) != 0) {
			(void)__BT_TLPUT(dbp, lock);
			goto err;
		}
		local = 1;

		/*
		 * If we deleted the last duplicate, we can fall out and do a
		 * normal btree delete in the context of the parent page.  If
		 * not, we have to update the parent's page.
		 */
		indx = cp->indx;
		if (next_pgno != PGNO_INVALID) {
			/*
			 * Copy, delete, update and re-insert the parent page's
			 * entry.
			 */
			bo = *GET_BOVERFLOW(h, indx);
			(void)__db_ditem(dbp, h, indx, BOVERFLOW_SIZE);
			bo.pgno = next_pgno;
			memset(&dbt, 0, sizeof(dbt));
			dbt.data = &bo;
			dbt.size = BOVERFLOW_SIZE;
			(void)__db_pitem(dbp,
			    h, indx, BOVERFLOW_SIZE, &dbt, NULL);

			/* Discard the parent page. */
			(void)memp_fput(dbp->mpf, h, 0);
			(void)__BT_TLPUT(dbp, lock);
			local = 0;

			goto done;
		}
	}

	/* Otherwise, do a normal btree delete. */
	if ((ret = __bam_ditem(dbp, h, indx)) != 0)
		goto err;
	if ((ret = __bam_ditem(dbp, h, indx)) != 0)
		goto err;

	/*
	 * If the page is empty, delete it.  To delete a leaf page we need a
	 * copy of a key from the page.  We use the first one that was there,
	 * since it's the last key that the page held.  We malloc the page
	 * information instead of using the return key/data memory because
	 * we've already set them -- the reason that we've already set them
	 * is because we're (potentially) about to do a reverse split, which
	 * would make our saved page information useless.
	 *
	 * XXX
	 * The following operations to delete a page might deadlock.  I think
	 * that's OK.  The problem is if we're deleting an item because we're
	 * closing cursors because we've already deadlocked and want to call
	 * txn_abort().  If we fail due to deadlock, we'll leave an locked
	 * empty page in the tree, which won't be empty long because we're
	 * going to undo the delete.
	 */
	if (NUM_ENT(h) == 0 && h->pgno != PGNO_ROOT) {
		memset(&dbt, 0, sizeof(DBT));
		dbt.flags = DB_DBT_MALLOC | DB_DBT_INTERNAL;
		if ((ret = __db_ret(dbp, h, 0, &dbt, NULL, NULL)) != 0)
			goto err;

		if (local) {
			(void)memp_fput(dbp->mpf, h, 0);
			(void)__BT_TLPUT(dbp, lock);
			local = 0;
		}

		ret = __bam_dpage(dbp, &dbt);
		free(dbt.data);
	}

err:
done:	if (local) {
		(void)memp_fput(dbp->mpf, h, 0);
		(void)__BT_TLPUT(dbp, lock);
	}

	if (ret == 0)
		++t->lstat.bt_deleted;
	return (ret);
}
