/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)bt_cursor.c	10.81 (Sleepycat) 12/16/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"
#include "shqueue.h"
#include "db_shash.h"
#include "lock.h"
#include "lock_ext.h"

static int __bam_c_close __P((DBC *));
static int __bam_c_del __P((DBC *, u_int32_t));
static int __bam_c_destroy __P((DBC *));
static int __bam_c_first __P((DBC *, CURSOR *));
static int __bam_c_get __P((DBC *, DBT *, DBT *, u_int32_t));
static int __bam_c_getstack __P((DBC *, CURSOR *));
static int __bam_c_last __P((DBC *, CURSOR *));
static int __bam_c_next __P((DBC *, CURSOR *, int));
static int __bam_c_physdel __P((DBC *, CURSOR *, PAGE *));
static int __bam_c_prev __P((DBC *, CURSOR *));
static int __bam_c_put __P((DBC *, DBT *, DBT *, u_int32_t));
static void __bam_c_reset __P((CURSOR *));
static int __bam_c_rget __P((DBC *, DBT *, u_int32_t));
static int __bam_c_search __P((DBC *, CURSOR *, const DBT *, u_int32_t, int *));
static int __bam_dsearch __P((DBC *, CURSOR *,  DBT *, u_int32_t *));

/* Discard the current page/lock held by a cursor. */
#undef	DISCARD
#define	DISCARD(dbc, cp) {						\
	if ((cp)->page != NULL) {					\
		(void)memp_fput((dbc)->dbp->mpf, (cp)->page, 0);	\
		(cp)->page = NULL;					\
	}								\
	if ((cp)->lock != LOCK_INVALID) {				\
		(void)__BT_TLPUT((dbc), (cp)->lock);			\
		(cp)->lock = LOCK_INVALID;				\
	}								\
}

/* If the cursor references a deleted record. */
#undef	IS_CUR_DELETED
#define	IS_CUR_DELETED(cp)						\
	(((cp)->dpgno == PGNO_INVALID &&				\
	B_DISSET(GET_BKEYDATA((cp)->page,				\
	(cp)->indx + O_INDX)->type)) ||					\
	((cp)->dpgno != PGNO_INVALID &&					\
	B_DISSET(GET_BKEYDATA((cp)->page, (cp)->dindx)->type)))

/* If the cursor and index combination references a deleted record. */
#undef	IS_DELETED
#define	IS_DELETED(cp, indx)						\
	(((cp)->dpgno == PGNO_INVALID &&				\
	B_DISSET(GET_BKEYDATA((cp)->page, (indx) + O_INDX)->type)) ||	\
	((cp)->dpgno != PGNO_INVALID &&					\
	B_DISSET(GET_BKEYDATA((cp)->page, (indx))->type)))

/*
 * Test to see if two cursors could point to duplicates of the same key,
 * whether on-page or off-page.  The leaf page numbers must be the same
 * in both cases.  In the case of off-page duplicates, the key indices
 * on the leaf page will be the same.  In the case of on-page duplicates,
 * the duplicate page number must not be set, and the key index offsets
 * must be the same.  For the last test, as the saved copy of the cursor
 * will not have a valid page pointer, we use the cursor's.
 */
#undef	POSSIBLE_DUPLICATE
#define	POSSIBLE_DUPLICATE(cursor, saved_copy)				\
	((cursor)->pgno == (saved_copy).pgno &&				\
	((cursor)->indx == (saved_copy).indx ||				\
	((cursor)->dpgno == PGNO_INVALID &&				\
	    (saved_copy).dpgno == PGNO_INVALID &&			\
	    (cursor)->page->inp[(cursor)->indx] ==			\
	    (cursor)->page->inp[(saved_copy).indx])))

/*
 * __bam_c_reset --
 *	Initialize internal cursor structure.
 */
static void
__bam_c_reset(cp)
	CURSOR *cp;
{
	cp->sp = cp->csp = cp->stack;
	cp->esp = cp->stack + sizeof(cp->stack) / sizeof(cp->stack[0]);
	cp->page = NULL;
	cp->pgno = PGNO_INVALID;
	cp->indx = 0;
	cp->dpgno = PGNO_INVALID;
	cp->dindx = 0;
	cp->lock = LOCK_INVALID;
	cp->mode = DB_LOCK_NG;
	cp->recno = RECNO_OOB;
	cp->flags = 0;
}

/*
 * __bam_c_init --
 *	Initialize the access private portion of a cursor
 *
 * PUBLIC: int __bam_c_init __P((DBC *));
 */
int
__bam_c_init(dbc)
	DBC *dbc;
{
	DB *dbp;
	CURSOR *cp;
	int ret;

	if ((ret = __os_calloc(1, sizeof(CURSOR), &cp)) != 0)
		return (ret);

	dbp = dbc->dbp;
	cp->dbc = dbc;

	/*
	 * Logical record numbers are always the same size, and we don't want
	 * to have to check for space every time we return one.  Allocate it
	 * in advance.
	 */
	if (dbp->type == DB_RECNO || F_ISSET(dbp, DB_BT_RECNUM)) {
		if ((ret = __os_malloc(sizeof(db_recno_t),
		    NULL, &dbc->rkey.data)) != 0) {
			__os_free(cp, sizeof(CURSOR));
			return (ret);
		}
		dbc->rkey.ulen = sizeof(db_recno_t);
	}

	/* Initialize methods. */
	dbc->internal = cp;
	if (dbp->type == DB_BTREE) {
		dbc->c_am_close = __bam_c_close;
		dbc->c_am_destroy = __bam_c_destroy;
		dbc->c_del = __bam_c_del;
		dbc->c_get = __bam_c_get;
		dbc->c_put = __bam_c_put;
	} else {
		dbc->c_am_close = __bam_c_close;
		dbc->c_am_destroy = __bam_c_destroy;
		dbc->c_del = __ram_c_del;
		dbc->c_get = __ram_c_get;
		dbc->c_put = __ram_c_put;
	}

	/* Initialize dynamic information. */
	__bam_c_reset(cp);

	return (0);
}

/*
 * __bam_c_close --
 *	Close down the cursor from a single use.
 */
static int
__bam_c_close(dbc)
	DBC *dbc;
{
	CURSOR *cp;
	DB *dbp;
	int ret;

	dbp = dbc->dbp;
	cp = dbc->internal;
	ret = 0;

	/*
	 * If a cursor deleted a btree key, perform the actual deletion.
	 * (Recno keys are either deleted immediately or never deleted.)
	 */
	if (dbp->type == DB_BTREE && F_ISSET(cp, C_DELETED))
		ret = __bam_c_physdel(dbc, cp, NULL);

	/* Discard any locks not acquired inside of a transaction. */
	if (cp->lock != LOCK_INVALID) {
		(void)__BT_TLPUT(dbc, cp->lock);
		cp->lock = LOCK_INVALID;
	}

	/* Sanity checks. */
#ifdef DIAGNOSTIC
	if (cp->csp != cp->stack)
		__db_err(dbp->dbenv, "btree cursor close: stack not empty");
#endif

	/* Initialize dynamic information. */
	__bam_c_reset(cp);

	return (ret);
}

/*
 * __bam_c_destroy --
 *	Close a single cursor -- internal version.
 */
static int
__bam_c_destroy(dbc)
	DBC *dbc;
{
	/* Discard the structures. */
	__os_free(dbc->internal, sizeof(CURSOR));

	return (0);
}

/*
 * __bam_c_del --
 *	Delete using a cursor.
 */
static int
__bam_c_del(dbc, flags)
	DBC *dbc;
	u_int32_t flags;
{
	CURSOR *cp;
	DB *dbp;
	DB_LOCK lock;
	PAGE *h;
	db_pgno_t pgno;
	db_indx_t indx;
	int ret;

	dbp = dbc->dbp;
	cp = dbc->internal;
	h = NULL;

	DB_PANIC_CHECK(dbp);

	/* Check for invalid flags. */
	if ((ret = __db_cdelchk(dbp, flags,
	    F_ISSET(dbp, DB_AM_RDONLY), cp->pgno != PGNO_INVALID)) != 0)
		return (ret);

	/*
	 * If we are running CDB, this had better be either a write
	 * cursor or an immediate writer.
	 */
	if (F_ISSET(dbp, DB_AM_CDB))
		if (!F_ISSET(dbc, DBC_RMW | DBC_WRITER))
			return (EINVAL);

	DEBUG_LWRITE(dbc, dbc->txn, "bam_c_del", NULL, NULL, flags);

	/* If already deleted, return failure. */
	if (F_ISSET(cp, C_DELETED))
		return (DB_KEYEMPTY);

	/*
	 * We don't physically delete the record until the cursor moves,
	 * so we have to have a long-lived write lock on the page instead
	 * of a long-lived read lock.  Note, we have to have a read lock
	 * to even get here, so we simply discard it.
	 */
	if (F_ISSET(dbp, DB_AM_LOCKING) && cp->mode != DB_LOCK_WRITE) {
		if ((ret = __bam_lget(dbc,
		    0, cp->pgno, DB_LOCK_WRITE, &lock)) != 0)
			goto err;
		(void)__BT_TLPUT(dbc, cp->lock);
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

	if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
		goto err;

	/* Log the change. */
	if (DB_LOGGING(dbc) &&
	    (ret = __bam_cdel_log(dbp->dbenv->lg_info, dbc->txn, &LSN(h),
	    0, dbp->log_fileid, PGNO(h), &LSN(h), indx)) != 0) {
		(void)memp_fput(dbp->mpf, h, 0);
		goto err;
	}

	/*
	 * Set the intent-to-delete flag on the page and update all cursors. */
	if (cp->dpgno == PGNO_INVALID)
		B_DSET(GET_BKEYDATA(h, indx + O_INDX)->type);
	else
		B_DSET(GET_BKEYDATA(h, indx)->type);
	(void)__bam_ca_delete(dbp, pgno, indx, 1);

	ret = memp_fput(dbp->mpf, h, DB_MPOOL_DIRTY);
	h = NULL;

	/*
	 * If the tree has record numbers, we have to adjust the counts.
	 *
	 * !!!
	 * This test is right -- we don't yet support duplicates and record
	 * numbers in the same tree, so ignore duplicates if DB_BT_RECNUM
	 * set.
	 */
	if (F_ISSET(dbp, DB_BT_RECNUM)) {
		if ((ret = __bam_c_getstack(dbc, cp)) != 0)
			goto err;
		if ((ret = __bam_adjust(dbc, -1)) != 0)
			goto err;
		(void)__bam_stkrel(dbc, 0);
	}

err:	if (h != NULL)
		(void)memp_fput(dbp->mpf, h, 0);
	return (ret);
}

/*
 * __bam_c_get --
 *	Get using a cursor (btree).
 */
static int
__bam_c_get(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	u_int32_t flags;
{
	CURSOR *cp, copy, start;
	DB *dbp;
	PAGE *h;
	int exact, ret, tmp_rmw;

	dbp = dbc->dbp;
	cp = dbc->internal;

	DB_PANIC_CHECK(dbp);

	/* Check for invalid flags. */
	if ((ret = __db_cgetchk(dbp,
	    key, data, flags, cp->pgno != PGNO_INVALID)) != 0)
		return (ret);

	/* Clear OR'd in additional bits so we can check for flag equality. */
	tmp_rmw = 0;
	if (LF_ISSET(DB_RMW)) {
		if (!F_ISSET(dbp, DB_AM_CDB)) {
			tmp_rmw = 1;
			F_SET(dbc, DBC_RMW);
		}
		LF_CLR(DB_RMW);
	}

	DEBUG_LREAD(dbc, dbc->txn, "bam_c_get",
	    flags == DB_SET || flags == DB_SET_RANGE ? key : NULL, NULL, flags);

	/*
	 * Return a cursor's record number.  It has nothing to do with the
	 * cursor get code except that it's been rammed into the interface.
	 */
	if (flags == DB_GET_RECNO) {
		ret = __bam_c_rget(dbc, data, flags);
		if (tmp_rmw)
			F_CLR(dbc, DBC_RMW);
		return (ret);
	}

	/*
	 * Initialize the cursor for a new retrieval.  Clear the cursor's
	 * page pointer, it was set before this operation, and no longer
	 * has any meaning.
	 */
	cp->page = NULL;
	copy = *cp;
	cp->lock = LOCK_INVALID;

	switch (flags) {
	case DB_CURRENT:
		/* It's not possible to return a deleted record. */
		if (F_ISSET(cp, C_DELETED)) {
			ret = DB_KEYEMPTY;
			goto err;
		}

		/* Acquire the current page. */
		if ((ret = __bam_lget(dbc,
		    0, cp->pgno, DB_LOCK_READ, &cp->lock)) == 0)
			ret = memp_fget(dbp->mpf,
			    cp->dpgno == PGNO_INVALID ? &cp->pgno : &cp->dpgno,
			    0, &cp->page);
		if (ret != 0)
			goto err;
		break;
	case DB_NEXT_DUP:
		if (cp->pgno == PGNO_INVALID) {
			ret = EINVAL;
			goto err;
		}
		if ((ret = __bam_c_next(dbc, cp, 1)) != 0)
			goto err;

		/* Make sure we didn't go past the end of the duplicates. */
		if (!POSSIBLE_DUPLICATE(cp, copy)) {
			ret = DB_NOTFOUND;
			goto err;
		}
		break;
	case DB_NEXT:
		if (cp->pgno != PGNO_INVALID) {
			if ((ret = __bam_c_next(dbc, cp, 1)) != 0)
				goto err;
			break;
		}
		/* FALLTHROUGH */
	case DB_FIRST:
		if ((ret = __bam_c_first(dbc, cp)) != 0)
			goto err;
		break;
	case DB_PREV:
		if (cp->pgno != PGNO_INVALID) {
			if ((ret = __bam_c_prev(dbc, cp)) != 0)
				goto err;
			break;
		}
		/* FALLTHROUGH */
	case DB_LAST:
		if ((ret = __bam_c_last(dbc, cp)) != 0)
			goto err;
		break;
	case DB_SET:
		if ((ret = __bam_c_search(dbc, cp, key, flags, &exact)) != 0)
			goto err;

		/*
		 * We cannot currently be referencing a deleted record, but we
		 * may be referencing off-page duplicates.
		 *
		 * If we're referencing off-page duplicates, move off-page.
		 * If we moved off-page, move to the next non-deleted record.  
		 * If we moved to the next non-deleted record, check to make
		 * sure we didn't switch records because our current record
		 * had no non-deleted data items.
		 */
		start = *cp;
		if ((ret = __bam_dup(dbc, cp, cp->indx, 0)) != 0)
			goto err;
		if (cp->dpgno != PGNO_INVALID && IS_CUR_DELETED(cp)) {
			if ((ret = __bam_c_next(dbc, cp, 0)) != 0)
				goto err;
			if (!POSSIBLE_DUPLICATE(cp, start)) {
				ret = DB_NOTFOUND;
				goto err;
			}
		}
		break;
	case DB_SET_RECNO:
		if ((ret = __bam_c_search(dbc, cp, key, flags, &exact)) != 0)
			goto err;
		break;
	case DB_GET_BOTH:
		if (F_ISSET(dbc, DBC_CONTINUE | DBC_KEYSET)) {
			/* Acquire the current page. */
			if ((ret = memp_fget(dbp->mpf,
			    cp->dpgno == PGNO_INVALID ? &cp->pgno : &cp->dpgno,
			    0, &cp->page)) != 0)
				goto err;

			/* If DBC_CONTINUE, move to the next item. */
			if (F_ISSET(dbc, DBC_CONTINUE) &&
			    (ret = __bam_c_next(dbc, cp, 1)) != 0)
				goto err;
		} else {
			if ((ret =
			    __bam_c_search(dbc, cp, key, flags, &exact)) != 0)
				goto err;

			/*
			 * We may be referencing a duplicates page.  Move to
			 * the first duplicate.
			 */
			if ((ret = __bam_dup(dbc, cp, cp->indx, 0)) != 0)
				goto err;
		}

		/* Search for a matching entry. */
		if ((ret = __bam_dsearch(dbc, cp, data, NULL)) != 0)
			goto err;

		/* Ignore deleted entries. */
		if (IS_CUR_DELETED(cp)) {
			ret = DB_NOTFOUND;
			goto err;
		}
		break;
	case DB_SET_RANGE:
		if ((ret = __bam_c_search(dbc, cp, key, flags, &exact)) != 0)
			goto err;

		/*
		 * As we didn't require an exact match, the search function
		 * may have returned an entry past the end of the page.  If
		 * so, move to the next entry.
		 */
		if (cp->indx == NUM_ENT(cp->page) &&
		    (ret = __bam_c_next(dbc, cp, 0)) != 0)
			goto err;

		/*
		 * We may be referencing off-page duplicates, if so, move
		 * off-page.
		 */
		if ((ret = __bam_dup(dbc, cp, cp->indx, 0)) != 0)
			goto err;

		/*
		 * We may be referencing a deleted record, if so, move to
		 * the next non-deleted record.
		 */
		if (IS_CUR_DELETED(cp) && (ret = __bam_c_next(dbc, cp, 0)) != 0)
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
			if ((ret = memp_fget(dbp->mpf, &cp->pgno, 0, &h)) != 0)
				goto err;
		} else
			h = cp->page;
		ret = __db_ret(dbp,
		    h, cp->indx, key, &dbc->rkey.data, &dbc->rkey.ulen);
		if (cp->dpgno != PGNO_INVALID)
			(void)memp_fput(dbp->mpf, h, 0);
		if (ret)
			goto err;
	}

	/* Return the data. */
	if ((ret = __db_ret(dbp, cp->page,
	    cp->dpgno == PGNO_INVALID ? cp->indx + O_INDX : cp->dindx,
	    data, &dbc->rdata.data, &dbc->rdata.ulen)) != 0)
		goto err;

	/*
	 * If the previous cursor record has been deleted, physically delete
	 * the entry from the page.  We clear the deleted flag before we call
	 * the underlying delete routine so that, if an error occurs, and we
	 * restore the cursor, the deleted flag is cleared.  This is because,
	 * if we manage to physically modify the page, and then restore the
	 * cursor, we might try to repeat the page modification when closing
	 * the cursor.
	 */
	if (F_ISSET(&copy, C_DELETED)) {
		F_CLR(&copy, C_DELETED);
		if ((ret = __bam_c_physdel(dbc, &copy, cp->page)) != 0)
			goto err;
	}
	F_CLR(cp, C_DELETED);

	/* Release the previous lock, if any; the current lock is retained. */
	if (copy.lock != LOCK_INVALID)
		(void)__BT_TLPUT(dbc, copy.lock);

	/* Release the current page. */
	if ((ret = memp_fput(dbp->mpf, cp->page, 0)) != 0)
		goto err;

	if (0) {
err:		if (cp->page != NULL)
			(void)memp_fput(dbp->mpf, cp->page, 0);
		if (cp->lock != LOCK_INVALID)
			(void)__BT_TLPUT(dbc, cp->lock);
		*cp = copy;
	}

	/* Release temporary lock upgrade. */
	if (tmp_rmw)
		F_CLR(dbc, DBC_RMW);

	return (ret);
}

/*
 * __bam_dsearch --
 *	Search for a matching data item (or the first data item that's
 *	equal to or greater than the one we're searching for).
 */
static int
__bam_dsearch(dbc, cp, data, iflagp)
	DBC *dbc;
	CURSOR *cp;
	DBT *data;
	u_int32_t *iflagp;
{
	DB *dbp;
	CURSOR copy, last;
	int cmp, ret;

	dbp = dbc->dbp;

	/*
	 * If iflagp is non-NULL, we're doing an insert.
	 *
	 * If the duplicates are off-page, use the duplicate search routine.
	 */
	if (cp->dpgno != PGNO_INVALID) {
		if ((ret = __db_dsearch(dbc, iflagp != NULL,
		    data, cp->dpgno, &cp->dindx, &cp->page, &cmp)) != 0)
			return (ret);
		cp->dpgno = cp->page->pgno;

		if (iflagp == NULL) {
			if (cmp != 0)
				return (DB_NOTFOUND);
			return (0);
		}
		*iflagp = DB_BEFORE;
		return (0);
	}

	/* Otherwise, do the search ourselves. */
	copy = *cp;
	for (;;) {
		/* Save the last interesting cursor position. */
		last = *cp;

		/* See if the data item matches the one we're looking for. */
		if ((cmp = __bam_cmp(dbp, data, cp->page, cp->indx + O_INDX,
		    dbp->dup_compare == NULL ?
		    __bam_defcmp : dbp->dup_compare)) == 0) {
			if (iflagp != NULL)
				*iflagp = DB_AFTER;
			return (0);
		}

		/*
		 * If duplicate entries are sorted, we're done if we find a
		 * page entry that sorts greater than the application item.
		 * If doing an insert, return success, otherwise DB_NOTFOUND.
		 */
		if (dbp->dup_compare != NULL && cmp < 0) {
			if (iflagp == NULL)
				return (DB_NOTFOUND);
			*iflagp = DB_BEFORE;
			return (0);
		}

		/*
		 * Move to the next item.  If we reach the end of the page and
		 * we're doing an insert, set the cursor to the last item and
		 * set the referenced memory location so callers know to insert
		 * after the item, instead of before it.  If not inserting, we
		 * return DB_NOTFOUND.
		 */
		if ((cp->indx += P_INDX) >= NUM_ENT(cp->page)) {
			if (iflagp == NULL)
				return (DB_NOTFOUND);
			goto use_last;
		}

		/*
		 * Make sure we didn't go past the end of the duplicates.  The
		 * error conditions are the same as above.
		 */
		if (!POSSIBLE_DUPLICATE(cp, copy)) {
			if (iflagp == NULL)
				 return (DB_NOTFOUND);
use_last:		*cp = last;
			*iflagp = DB_AFTER;
			return (0);
		}
	}
	/* NOTREACHED */
}

/*
 * __bam_c_rget --
 *	Return the record number for a cursor.
 */
static int
__bam_c_rget(dbc, data, flags)
	DBC *dbc;
	DBT *data;
	u_int32_t flags;
{
	CURSOR *cp;
	DB *dbp;
	DBT dbt;
	db_recno_t recno;
	int exact, ret;

	COMPQUIET(flags, 0);
	dbp = dbc->dbp;
	cp = dbc->internal;

	/* Get the page with the current item on it. */
	if ((ret = memp_fget(dbp->mpf, &cp->pgno, 0, &cp->page)) != 0)
		return (ret);

	/* Get a copy of the key. */
	memset(&dbt, 0, sizeof(DBT));
	dbt.flags = DB_DBT_MALLOC | DB_DBT_INTERNAL;
	if ((ret = __db_ret(dbp, cp->page, cp->indx, &dbt, NULL, NULL)) != 0)
		goto err;

	exact = 1;
	if ((ret = __bam_search(dbc, &dbt,
	    F_ISSET(dbc, DBC_RMW) ? S_FIND_WR : S_FIND,
	    1, &recno, &exact)) != 0)
		goto err;

	ret = __db_retcopy(data, &recno, sizeof(recno),
	    &dbc->rdata.data, &dbc->rdata.ulen, dbp->db_malloc);

	/* Release the stack. */
	__bam_stkrel(dbc, 0);

err:	(void)memp_fput(dbp->mpf, cp->page, 0);
	__os_free(dbt.data, dbt.size);
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
	u_int32_t flags;
{
	CURSOR *cp, copy;
	DB *dbp;
	DBT dbt;
	db_indx_t indx;
	db_pgno_t pgno;
	u_int32_t iiflags, iiop;
	int exact, needkey, ret, stack;
	void *arg;

	dbp = dbc->dbp;
	cp = dbc->internal;

	DB_PANIC_CHECK(dbp);

	DEBUG_LWRITE(dbc, dbc->txn, "bam_c_put",
	    flags == DB_KEYFIRST || flags == DB_KEYLAST ? key : NULL,
	    data, flags);

	if ((ret = __db_cputchk(dbp, key, data, flags,
	    F_ISSET(dbp, DB_AM_RDONLY), cp->pgno != PGNO_INVALID)) != 0)
		return (ret);

	/*
	 * If we are running CDB, this had better be either a write
	 * cursor or an immediate writer.  If it's a regular writer,
	 * that means we have an IWRITE lock and we need to upgrade
	 * it to a write lock.
	 */
	if (F_ISSET(dbp, DB_AM_CDB)) {
		if (!F_ISSET(dbc, DBC_RMW | DBC_WRITER))
			return (EINVAL);

		if (F_ISSET(dbc, DBC_RMW) &&
		    (ret = lock_get(dbp->dbenv->lk_info, dbc->locker,
		    DB_LOCK_UPGRADE, &dbc->lock_dbt, DB_LOCK_WRITE,
		    &dbc->mylock)) != 0)
			return (EAGAIN);
	}

	if (0) {
split:		/*
		 * To split, we need a valid key for the page.  Since it's a
		 * cursor, we have to build one.
		 *
		 * Acquire a copy of a key from the page.
		 */
		if (needkey) {
			memset(&dbt, 0, sizeof(DBT));
			if ((ret = __db_ret(dbp, cp->page, indx,
			    &dbt, &dbc->rkey.data, &dbc->rkey.ulen)) != 0)
				goto err;
			arg = &dbt;
		} else
			arg = key;

		/*
		 * Discard any locks and pinned pages (the locks are discarded
		 * even if we're running with transactions, as they lock pages
		 * that we're sorry we ever acquired).  If stack is set and the
		 * cursor entries are valid, they point to the same entries as
		 * the stack, don't free them twice.
		 */
		if (stack) {
			(void)__bam_stkrel(dbc, 1);
			stack = 0;
		} else
			DISCARD(dbc, cp);

		/*
		 * Restore the cursor to its original value.  This is necessary
		 * for two reasons.  First, we are about to copy it in case of
		 * error, again.  Second, we adjust cursors during the split,
		 * and we have to ensure this cursor is adjusted appropriately,
		 * along with all the other cursors.
		 */
		*cp = copy;

		if ((ret = __bam_split(dbc, arg)) != 0)
			goto err;
	}

	/*
	 * Initialize the cursor for a new retrieval.  Clear the cursor's
	 * page pointer, it was set before this operation, and no longer
	 * has any meaning.
	 */
	cp->page = NULL;
	copy = *cp;
	cp->lock = LOCK_INVALID;

	iiflags = needkey = ret = stack = 0;
	switch (flags) {
	case DB_AFTER:
	case DB_BEFORE:
	case DB_CURRENT:
		needkey = 1;
		if (cp->dpgno == PGNO_INVALID) {
			pgno = cp->pgno;
			indx = cp->indx;
		} else {
			pgno = cp->dpgno;
			indx = cp->dindx;
		}

		/*
		 * !!!
		 * This test is right -- we don't yet support duplicates and
		 * record numbers in the same tree, so ignore duplicates if
		 * DB_BT_RECNUM set.
		 */
		if (F_ISSET(dbp, DB_BT_RECNUM) &&
		    (flags != DB_CURRENT || F_ISSET(cp, C_DELETED))) {
			/* Acquire a complete stack. */
			if ((ret = __bam_c_getstack(dbc, cp)) != 0)
				goto err;
			cp->page = cp->csp->page;

			stack = 1;
			iiflags = BI_DOINCR;
		} else {
			/* Acquire the current page. */
			if ((ret = __bam_lget(dbc,
			    0, cp->pgno, DB_LOCK_WRITE, &cp->lock)) == 0)
				ret = memp_fget(dbp->mpf, &pgno, 0, &cp->page);
			if (ret != 0)
				goto err;

			iiflags = 0;
		}

		/*
		 * If the user has specified a duplicate comparison function,
		 * we return an error if DB_CURRENT was specified and the
		 * replacement data doesn't compare equal to the current data.
		 * This stops apps from screwing up the duplicate sort order.
		 */
		if (flags == DB_CURRENT && dbp->dup_compare != NULL)
			if (__bam_cmp(dbp, data,
			    cp->page, indx, dbp->dup_compare) != 0) {
				ret = EINVAL;
				goto err;
			}

		iiop = flags;
		break;
	case DB_KEYFIRST:
	case DB_KEYLAST:
		/*
		 * If we have a duplicate comparison function, we position to
		 * the first of any on-page duplicates, and use __bam_dsearch
		 * to search for the right slot.  Otherwise, we position to
		 * the first/last of any on-page duplicates based on the flag
		 * value.
		 */
		if ((ret = __bam_c_search(dbc, cp, key,
		    flags == DB_KEYFIRST || dbp->dup_compare != NULL ?
		    DB_KEYFIRST : DB_KEYLAST, &exact)) != 0)
			goto err;
		stack = 1;

		/*
		 * If an exact match:
		 *	If duplicates aren't supported, replace the current
		 *	item.  (When implementing the DB->put function, our
		 *	caller has already checked the DB_NOOVERWRITE flag.)
		 *
		 *	If there's a duplicate comparison function, find the
		 *	correct slot for this duplicate item.
		 *
		 *	If there's no duplicate comparison function, set the
		 *	insert flag based on the argument flags.
		 *
		 * If there's no match, the search function returned the
		 * smallest slot greater than the key, use it.
		 */
		if (exact) {
			if (F_ISSET(dbp, DB_AM_DUP)) {
				/*
				 * If at off-page duplicate page, move to the
				 * first or last entry -- if a comparison
				 * function was specified, start searching at
				 * the first entry.  Otherwise, move based on
				 * the DB_KEYFIRST/DB_KEYLAST flags.
				 */
				if ((ret = __bam_dup(dbc, cp, cp->indx,
				    dbp->dup_compare == NULL &&
				    flags != DB_KEYFIRST)) != 0)
					goto err;

				/*
				 * If there's a comparison function, search for
				 * the correct slot.  Otherwise, set the insert
				 * flag based on the argment flag.
				 */
				if (dbp->dup_compare == NULL)
					iiop = flags == DB_KEYFIRST ?
					    DB_BEFORE : DB_AFTER;
				else
					if ((ret = __bam_dsearch(dbc,
					    cp, data, &iiop)) != 0)
						goto err;
			} else
				iiop = DB_CURRENT;
			iiflags = 0;
		} else {
			iiop = DB_BEFORE;
			iiflags = BI_NEWKEY;
		}

		if (cp->dpgno == PGNO_INVALID) {
			pgno = cp->pgno;
			indx = cp->indx;
		} else {
			pgno = cp->dpgno;
			indx = cp->dindx;
		}
		break;
	}

	ret = __bam_iitem(dbc, &cp->page, &indx, key, data, iiop, iiflags);

	if (ret == DB_NEEDSPLIT)
		goto split;
	if (ret != 0)
		goto err;

	/*
	 * Reset any cursors referencing this item that might have the item
	 * marked for deletion.
	 */
	if (iiop == DB_CURRENT) {
		(void)__bam_ca_delete(dbp, pgno, indx, 0);

		/*
		 * It's also possible that we are the cursor that had the
		 * item marked for deletion, in which case we want to make
		 * sure that we don't delete it because we had the delete
		 * flag set already.
		 */
		if (cp->pgno == copy.pgno && cp->indx == copy.indx &&
		    cp->dpgno == copy.dpgno && cp->dindx == copy.dindx)
			F_CLR(&copy, C_DELETED);
	}

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
	 * If the previous cursor record has been deleted, physically delete
	 * the entry from the page.  We clear the deleted flag before we call
	 * the underlying delete routine so that, if an error occurs, and we
	 * restore the cursor, the deleted flag is cleared.  This is because,
	 * if we manage to physically modify the page, and then restore the
	 * cursor, we might try to repeat the page modification when closing
	 * the cursor.
	 */
	if (F_ISSET(&copy, C_DELETED)) {
		F_CLR(&copy, C_DELETED);
		if ((ret = __bam_c_physdel(dbc, &copy, cp->page)) != 0)
			goto err;
	}
	F_CLR(cp, C_DELETED);

	/* Release the previous lock, if any; the current lock is retained. */
	if (copy.lock != LOCK_INVALID)
		(void)__BT_TLPUT(dbc, copy.lock);

	/*
	 * Discard any pages pinned in the tree and their locks, except for
	 * the leaf page, for which we only discard the pin, not the lock.
	 *
	 * Note, the leaf page participated in the stack we acquired, and so
	 * we have to adjust the stack as necessary.  If there was only a
	 * single page on the stack, we don't have to free further stack pages.
	 */
	if (stack && BT_STK_POP(cp) != NULL)
		(void)__bam_stkrel(dbc, 0);

	/* Release the current page. */
	if ((ret = memp_fput(dbp->mpf, cp->page, 0)) != 0)
		goto err;

	if (0) {
err:		/* Discard any pinned pages. */
		if (stack)
			(void)__bam_stkrel(dbc, 0);
		else
			DISCARD(dbc, cp);
		*cp = copy;
	}

	if (F_ISSET(dbp, DB_AM_CDB) && F_ISSET(dbc, DBC_RMW))
		(void)__lock_downgrade(dbp->dbenv->lk_info, dbc->mylock,
		    DB_LOCK_IWRITE, 0);

	return (ret);
}

/*
 * __bam_c_first --
 *	Return the first record.
 */
static int
__bam_c_first(dbc, cp)
	DBC *dbc;
	CURSOR *cp;
{
	DB *dbp;
	db_pgno_t pgno;
	int ret;

	dbp = dbc->dbp;

	/* Walk down the left-hand side of the tree. */
	for (pgno = PGNO_ROOT;;) {
		if ((ret =
		    __bam_lget(dbc, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
			return (ret);

		/* If we find a leaf page, we're done. */
		if (ISLEAF(cp->page))
			break;

		pgno = GET_BINTERNAL(cp->page, 0)->pgno;
		DISCARD(dbc, cp);
	}

	cp->pgno = cp->page->pgno;
	cp->indx = 0;
	cp->dpgno = PGNO_INVALID;

	/* Check for duplicates. */
	if ((ret = __bam_dup(dbc, cp, cp->indx, 0)) != 0)
		return (ret);

	/* If on an empty page or a deleted record, move to the next one. */
	if (NUM_ENT(cp->page) == 0 || IS_CUR_DELETED(cp))
		if ((ret = __bam_c_next(dbc, cp, 0)) != 0)
			return (ret);

	return (0);
}

/*
 * __bam_c_last --
 *	Return the last record.
 */
static int
__bam_c_last(dbc, cp)
	DBC *dbc;
	CURSOR *cp;
{
	DB *dbp;
	db_pgno_t pgno;
	int ret;

	dbp = dbc->dbp;

	/* Walk down the right-hand side of the tree. */
	for (pgno = PGNO_ROOT;;) {
		if ((ret =
		    __bam_lget(dbc, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
			return (ret);

		/* If we find a leaf page, we're done. */
		if (ISLEAF(cp->page))
			break;

		pgno =
		    GET_BINTERNAL(cp->page, NUM_ENT(cp->page) - O_INDX)->pgno;
		DISCARD(dbc, cp);
	}

	cp->pgno = cp->page->pgno;
	cp->indx = NUM_ENT(cp->page) == 0 ? 0 : NUM_ENT(cp->page) - P_INDX;
	cp->dpgno = PGNO_INVALID;

	/* Check for duplicates. */
	if ((ret = __bam_dup(dbc, cp, cp->indx, 1)) != 0)
		return (ret);

	/* If on an empty page or a deleted record, move to the next one. */
	if (NUM_ENT(cp->page) == 0 || IS_CUR_DELETED(cp))
		if ((ret = __bam_c_prev(dbc, cp)) != 0)
			return (ret);

	return (0);
}

/*
 * __bam_c_next --
 *	Move to the next record.
 */
static int
__bam_c_next(dbc, cp, initial_move)
	DBC *dbc;
	CURSOR *cp;
	int initial_move;
{
	DB *dbp;
	db_indx_t adjust, indx;
	db_pgno_t pgno;
	int ret;

	dbp = dbc->dbp;

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
		    __bam_lget(dbc, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
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
			/*
			 * If we're in a btree leaf page, we've reached the end
			 * of the tree.  If we've reached the end of a page of
			 * duplicates, continue from the btree leaf page where
			 * we found this page of duplicates.
			 */
			pgno = cp->page->next_pgno;
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

			DISCARD(dbc, cp);
			if ((ret = __bam_lget(dbc,
			    0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
				return (ret);
			if ((ret =
			    memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
				return (ret);
			continue;
		}

		/* Ignore deleted records. */
		if (IS_DELETED(cp, indx)) {
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

			if ((ret = __bam_dup(dbc, cp, indx, 0)) != 0)
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
__bam_c_prev(dbc, cp)
	DBC *dbc;
	CURSOR *cp;
{
	DB *dbp;
	db_indx_t indx, adjust;
	db_pgno_t pgno;
	int ret, set_indx;

	dbp = dbc->dbp;

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
		    __bam_lget(dbc, 0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
			return (ret);
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
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
			/*
			 * If we're in a btree leaf page, we've reached the
			 * beginning of the tree.  If we've reached the first
			 * of a page of duplicates, continue from the btree
			 * leaf page where we found this page of duplicates.
			 */
			pgno = cp->page->prev_pgno;
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

			DISCARD(dbc, cp);
			if ((ret = __bam_lget(dbc,
			    0, pgno, DB_LOCK_READ, &cp->lock)) != 0)
				return (ret);
			if ((ret =
			    memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
				return (ret);

			if (set_indx)
				indx = NUM_ENT(cp->page);
			if (indx == 0)
				continue;
		}

		/* Ignore deleted records. */
		indx -= adjust;
		if (IS_DELETED(cp, indx))
			continue;

		/*
		 * If we're not in a duplicates page, check to see if we've
		 * found a page of duplicates, in which case we move to the
		 * last entry.
		 */
		if (cp->dpgno == PGNO_INVALID) {
			cp->pgno = cp->page->pgno;
			cp->indx = indx;

			if ((ret = __bam_dup(dbc, cp, indx, 1)) != 0)
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
__bam_c_search(dbc, cp, key, flags, exactp)
	DBC *dbc;
	CURSOR *cp;
	const DBT *key;
	u_int32_t flags;
	int *exactp;
{
	BTREE *t;
	DB *dbp;
	DB_LOCK lock;
	PAGE *h;
	db_recno_t recno;
	db_indx_t indx;
	u_int32_t sflags;
	int cmp, needexact, ret;

	dbp = dbc->dbp;
	t = dbp->internal;

	/* Find an entry in the database. */
	switch (flags) {
	case DB_SET_RECNO:
		if ((ret = __ram_getno(dbc, key, &recno, 0)) != 0)
			return (ret);
		sflags = F_ISSET(dbc, DBC_RMW) ? S_FIND_WR : S_FIND;
		needexact = *exactp = 1;
		ret = __bam_rsearch(dbc, &recno, sflags, 1, exactp);
		break;
	case DB_SET:
	case DB_GET_BOTH:
		sflags = F_ISSET(dbc, DBC_RMW) ? S_FIND_WR : S_FIND;
		needexact = *exactp = 1;
		goto search;
	case DB_SET_RANGE:
		sflags = F_ISSET(dbc, DBC_RMW) ? S_FIND_WR : S_FIND;
		needexact = *exactp = 0;
		goto search;
	case DB_KEYFIRST:
		sflags = S_KEYFIRST;
		goto fast_search;
	case DB_KEYLAST:
		sflags = S_KEYLAST;
fast_search:	needexact = *exactp = 0;
		/*
		 * If the application has a history of inserting into the first
		 * or last pages of the database, we check those pages first to
		 * avoid doing a full search.
		 *
		 * Record numbers can't be fast-tracked, the entire tree has to
		 * be locked.
		 */
		h = NULL;
		lock = LOCK_INVALID;
		if (F_ISSET(dbp, DB_BT_RECNUM))
			goto search;

		/* Check if the application has a history of sorted input. */
		if (t->bt_lpgno == PGNO_INVALID)
			goto search;

		/*
		 * Lock and retrieve the page on which we did the last insert.
		 * It's okay if it doesn't exist, or if it's not the page type
		 * we expected, it just means that the world changed.
		 */
		if (__bam_lget(dbc, 0, t->bt_lpgno, DB_LOCK_WRITE, &lock))
			goto fast_miss;
		if (memp_fget(dbp->mpf, &t->bt_lpgno, 0, &h))
			goto fast_miss;
		if (TYPE(h) != P_LBTREE)
			goto fast_miss;
		if (NUM_ENT(h) == 0)
			goto fast_miss;

		/*
		 * What we do here is test to see if we're at the beginning or
		 * end of the tree and if the new item sorts before/after the
		 * first/last page entry.  We don't try and catch inserts into
		 * the middle of the tree (although we could, as long as there
		 * were two keys on the page and we saved both the index and
		 * the page number of the last insert).
		 */
		if (h->next_pgno == PGNO_INVALID) {
			indx = NUM_ENT(h) - P_INDX;
			if ((cmp =
			    __bam_cmp(dbp, key, h, indx, t->bt_compare)) < 0)
				goto try_begin;
			if (cmp > 0) {
				indx += P_INDX;
				goto fast_hit;
			}

			/*
			 * Found a duplicate.  If doing DB_KEYLAST, we're at
			 * the correct position, otherwise, move to the first
			 * of the duplicates.
			 */
			if (flags == DB_KEYLAST)
				goto fast_hit;
			for (;
			    indx > 0 && h->inp[indx - P_INDX] == h->inp[indx];
			    indx -= P_INDX)
				;
			goto fast_hit;
		}
try_begin:	if (h->prev_pgno == PGNO_INVALID) {
			indx = 0;
			if ((cmp =
			    __bam_cmp(dbp, key, h, indx, t->bt_compare)) > 0)
				goto fast_miss;
			if (cmp < 0)
				goto fast_hit;
			/*
			 * Found a duplicate.  If doing DB_KEYFIRST, we're at
			 * the correct position, otherwise, move to the last
			 * of the duplicates.
			 */
			if (flags == DB_KEYFIRST)
				goto fast_hit;
			for (;
			    indx < (db_indx_t)(NUM_ENT(h) - P_INDX) &&
			    h->inp[indx] == h->inp[indx + P_INDX];
			    indx += P_INDX)
				;
			goto fast_hit;
		}
		goto fast_miss;

fast_hit:	/* Set the exact match flag, we may have found a duplicate. */
		*exactp = cmp == 0;

		/* Enter the entry in the stack. */
		BT_STK_CLR(cp);
		BT_STK_ENTER(cp, h, indx, lock, ret);
		break;

fast_miss:	if (h != NULL)
			(void)memp_fput(dbp->mpf, h, 0);
		if (lock != LOCK_INVALID)
			(void)__BT_LPUT(dbc, lock);

search:		ret = __bam_search(dbc, key, sflags, 1, NULL, exactp);
		break;
	default:				/* XXX: Impossible. */
		abort();
		/* NOTREACHED */
	}
	if (ret != 0)
		return (ret);

	/*
	 * Initialize the cursor to reference it.  This has to be done
	 * before we return (even with DB_NOTFOUND) because we have to
	 * free the page(s) we locked in __bam_search.
	 */
	cp->page = cp->csp->page;
	cp->pgno = cp->csp->page->pgno;
	cp->indx = cp->csp->indx;
	cp->lock = cp->csp->lock;
	cp->dpgno = PGNO_INVALID;

	/*
	 * If we inserted a key into the first or last slot of the tree,
	 * remember where it was so we can do it more quickly next time.
	 */
	if (flags == DB_KEYFIRST || flags == DB_KEYLAST)
		t->bt_lpgno =
		    ((cp->page->next_pgno == PGNO_INVALID &&
		    cp->indx >= NUM_ENT(cp->page)) ||
		    (cp->page->prev_pgno == PGNO_INVALID && cp->indx == 0)) ?
		    cp->pgno : PGNO_INVALID;

	/* If we need an exact match and didn't find one, we're done. */
	if (needexact && *exactp == 0)
		return (DB_NOTFOUND);

	return (0);
}

/*
 * __bam_dup --
 *	Check for an off-page duplicates entry, and if found, move to the
 *	first or last entry.
 *
 * PUBLIC: int __bam_dup __P((DBC *, CURSOR *, u_int32_t, int));
 */
int
__bam_dup(dbc, cp, indx, last_dup)
	DBC *dbc;
	CURSOR *cp;
	u_int32_t indx;
	int last_dup;
{
	BOVERFLOW *bo;
	DB *dbp;
	db_pgno_t pgno;
	int ret;

	dbp = dbc->dbp;

	/*
	 * Check for an overflow entry.  If we find one, move to the
	 * duplicates page, and optionally move to the last record on
	 * that page.
	 *
	 * !!!
	 * We don't lock duplicates pages, we've already got the correct
	 * lock on the main page.
	 */
	bo = GET_BOVERFLOW(cp->page, indx + O_INDX);
	if (B_TYPE(bo->type) != B_DUPLICATE)
		return (0);

	pgno = bo->pgno;
	if ((ret = memp_fput(dbp->mpf, cp->page, 0)) != 0)
		return (ret);
	cp->page = NULL;
	if (last_dup) {
		if ((ret = __db_dend(dbc, pgno, &cp->page)) != 0)
			return (ret);
		indx = NUM_ENT(cp->page) - O_INDX;
	} else {
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &cp->page)) != 0)
			return (ret);
		indx = 0;
	}

	/* Update the cursor's duplicate information. */
	cp->dpgno = cp->page->pgno;
	cp->dindx = indx;

	return (0);
}

/*
 * __bam_c_physdel --
 *	Actually do the cursor deletion.
 */
static int
__bam_c_physdel(dbc, cp, h)
	DBC *dbc;
	CURSOR *cp;
	PAGE *h;
{
	enum { DELETE_ITEM, DELETE_PAGE, NOTHING_FURTHER } cmd;
	BOVERFLOW bo;
	DB *dbp;
	DBT dbt;
	DB_LOCK lock;
	db_indx_t indx;
	db_pgno_t pgno, next_pgno, prev_pgno;
	int delete_page, local_page, ret;

	dbp = dbc->dbp;

	delete_page = ret = 0;

	/* Figure out what we're deleting. */
	if (cp->dpgno == PGNO_INVALID) {
		pgno = cp->pgno;
		indx = cp->indx;
	} else {
		pgno = cp->dpgno;
		indx = cp->dindx;
	}

	/*
	 * If the item is referenced by another cursor, set that cursor's
	 * delete flag and leave it up to it to do the delete.
	 *
	 * !!!
	 * This test for > 0 is a tricky.  There are two ways that we can
	 * be called here.  Either we are closing the cursor or we've moved
	 * off the page with the deleted entry.  In the first case, we've
	 * already removed the cursor from the active queue, so we won't see
	 * it in __bam_ca_delete. In the second case, it will be on a different
	 * item, so we won't bother with it in __bam_ca_delete.
	 */
	if (__bam_ca_delete(dbp, pgno, indx, 1) > 0)
		return (0);

	/*
	 * If this is concurrent DB, upgrade the lock if necessary.
	 */
	if (F_ISSET(dbp, DB_AM_CDB) && F_ISSET(dbc, DBC_RMW) &&
	    (ret = lock_get(dbp->dbenv->lk_info,
	    dbc->locker, DB_LOCK_UPGRADE, &dbc->lock_dbt, DB_LOCK_WRITE,
	    &dbc->mylock)) != 0)
		return (EAGAIN);

	/*
	 * If we don't already have the page locked, get it and delete the
	 * items.
	 */
	if ((h == NULL || h->pgno != pgno)) {
		if ((ret = __bam_lget(dbc, 0, pgno, DB_LOCK_WRITE, &lock)) != 0)
			return (ret);
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
			return (ret);
		local_page = 1;
	} else
		local_page = 0;

	/*
	 * If we're deleting a duplicate entry and there are other duplicate
	 * entries remaining, call the common code to do the work and fix up
	 * the parent page as necessary.  Otherwise, do a normal btree delete.
	 *
	 * There are 5 possible cases:
	 *
	 * 1. It's not a duplicate item: do a normal btree delete.
	 * 2. It's a duplicate item:
	 *	2a: We delete an item from a page of duplicates, but there are
	 *	    more items on the page.
	 *      2b: We delete the last item from a page of duplicates, deleting
	 *	    the last duplicate.
	 *      2c: We delete the last item from a page of duplicates, but there
	 *	    is a previous page of duplicates.
	 *      2d: We delete the last item from a page of duplicates, but there
	 *	    is a following page of duplicates.
	 *
	 * In the case of:
	 *
	 *  1: There's nothing further to do.
	 * 2a: There's nothing further to do.
	 * 2b: Do the normal btree delete instead of a duplicate delete, as
	 *     that deletes both the duplicate chain and the parent page's
	 *     entry.
	 * 2c: There's nothing further to do.
	 * 2d: Delete the duplicate, and update the parent page's entry.
	 */
	if (TYPE(h) == P_DUPLICATE) {
		pgno = PGNO(h);
		prev_pgno = PREV_PGNO(h);
		next_pgno = NEXT_PGNO(h);

		if (NUM_ENT(h) == 1 &&
		    prev_pgno == PGNO_INVALID && next_pgno == PGNO_INVALID)
			cmd = DELETE_PAGE;
		else {
			cmd = DELETE_ITEM;

			/* Delete the duplicate. */
			if ((ret = __db_drem(dbc, &h, indx, __bam_free)) != 0)
				goto err;

			/*
			 * 2a: h != NULL, h->pgno == pgno
			 * 2b: We don't reach this clause, as the above test
			 *     was true.
			 * 2c: h == NULL, prev_pgno != PGNO_INVALID
			 * 2d: h != NULL, next_pgno != PGNO_INVALID
			 *
			 * Test for 2a and 2c: if we didn't empty the current
			 * page or there was a previous page of duplicates, we
			 * don't need to touch the parent page.
			 */
			if ((h != NULL && pgno == h->pgno) ||
			    prev_pgno != PGNO_INVALID)
				cmd = NOTHING_FURTHER;
		}

		/*
		 * Release any page we're holding and its lock.
		 *
		 * !!!
		 * If there is no subsequent page in the duplicate chain, then
		 * __db_drem will have put page "h" and set it to NULL.
		*/
		if (local_page) {
			if (h != NULL)
				(void)memp_fput(dbp->mpf, h, 0);
			(void)__BT_TLPUT(dbc, lock);
			local_page = 0;
		}

		if (cmd == NOTHING_FURTHER)
			goto done;

		/* Acquire the parent page and switch the index to its entry. */
		if ((ret =
		    __bam_lget(dbc, 0, cp->pgno, DB_LOCK_WRITE, &lock)) != 0)
			goto err;
		if ((ret = memp_fget(dbp->mpf, &cp->pgno, 0, &h)) != 0) {
			(void)__BT_TLPUT(dbc, lock);
			goto err;
		}
		local_page = 1;
		indx = cp->indx;

		if (cmd == DELETE_PAGE)
			goto btd;

		/*
		 * Copy, delete, update, add-back the parent page's data entry.
		 *
		 * XXX
		 * This may be a performance/logging problem.  We should add a
		 * log message which simply logs/updates a random set of bytes
		 * on a page, and use it instead of doing a delete/add pair.
		 */
		indx += O_INDX;
		bo = *GET_BOVERFLOW(h, indx);
		(void)__db_ditem(dbc, h, indx, BOVERFLOW_SIZE);
		bo.pgno = next_pgno;
		memset(&dbt, 0, sizeof(dbt));
		dbt.data = &bo;
		dbt.size = BOVERFLOW_SIZE;
		(void)__db_pitem(dbc, h, indx, BOVERFLOW_SIZE, &dbt, NULL);
		(void)memp_fset(dbp->mpf, h, DB_MPOOL_DIRTY);
		goto done;
	}

btd:	/*
	 * If the page is going to be emptied, delete it.  To delete a leaf
	 * page we need a copy of a key from the page.  We use the 0th page
	 * index since it's the last key that the page held.
	 *
	 * We malloc the page information instead of using the return key/data
	 * memory because we've already set them -- the reason we've already
	 * set them is because we're (potentially) about to do a reverse split,
	 * which would make our saved page information useless.
	 *
	 * !!!
	 * The following operations to delete a page might deadlock.  I think
	 * that's OK.  The problem is if we're deleting an item because we're
	 * closing cursors because we've already deadlocked and want to call
	 * txn_abort().  If we fail due to deadlock, we leave a locked empty
	 * page in the tree, which won't be empty long because we're going to
	 * undo the delete.
	 */
	if (NUM_ENT(h) == 2 && h->pgno != PGNO_ROOT) {
		memset(&dbt, 0, sizeof(DBT));
		dbt.flags = DB_DBT_MALLOC | DB_DBT_INTERNAL;
		if ((ret = __db_ret(dbp, h, 0, &dbt, NULL, NULL)) != 0)
			goto err;
		delete_page = 1;
	}

	/*
	 * Do a normal btree delete.
	 *
	 * !!!
	 * Delete the key item first, otherwise the duplicate checks in
	 * __bam_ditem() won't work!
	 */
	if ((ret = __bam_ditem(dbc, h, indx)) != 0)
		goto err;
	if ((ret = __bam_ditem(dbc, h, indx)) != 0)
		goto err;

	/* Discard any remaining locks/pages. */
	if (local_page) {
		(void)memp_fput(dbp->mpf, h, 0);
		(void)__BT_TLPUT(dbc, lock);
		local_page = 0;
	}

	/* Delete the page if it was emptied. */
	if (delete_page)
		ret = __bam_dpage(dbc, &dbt);

err:
done:	if (delete_page)
		__os_free(dbt.data, dbt.size);

	if (local_page) {
		/*
		 * It's possible for h to be NULL, as __db_drem may have
		 * been relinking pages by the time that it deadlocked.
		 */
		if (h != NULL)
			(void)memp_fput(dbp->mpf, h, 0);
		(void)__BT_TLPUT(dbc, lock);
	}

	if (F_ISSET(dbp, DB_AM_CDB) && F_ISSET(dbc, DBC_RMW))
		(void)__lock_downgrade(dbp->dbenv->lk_info, dbc->mylock,
		    DB_LOCK_IWRITE, 0);

	return (ret);
}

/*
 * __bam_c_getstack --
 *	Acquire a full stack for a cursor.
 */
static int
__bam_c_getstack(dbc, cp)
	DBC *dbc;
	CURSOR *cp;
{
	DB *dbp;
	DBT dbt;
	PAGE *h;
	db_pgno_t pgno;
	int exact, ret;

	dbp = dbc->dbp;
	h = NULL;
	memset(&dbt, 0, sizeof(DBT));
	ret = 0;

	/* Get the page with the current item on it. */
	pgno = cp->pgno;
	if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
		return (ret);

	/* Get a copy of a key from the page. */
	dbt.flags = DB_DBT_MALLOC | DB_DBT_INTERNAL;
	if ((ret = __db_ret(dbp, h, 0, &dbt, NULL, NULL)) != 0)
		goto err;

	/* Get a write-locked stack for that page. */
	exact = 0;
	ret = __bam_search(dbc, &dbt, S_KEYFIRST, 1, NULL, &exact);

	/* We no longer need the key or the page. */
err:	if (h != NULL)
		(void)memp_fput(dbp->mpf, h, 0);
	if (dbt.data != NULL)
		__os_free(dbt.data, dbt.size);
	return (ret);
}
