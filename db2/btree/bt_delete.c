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
static const char sccsid[] = "@(#)bt_delete.c	10.43 (Sleepycat) 12/7/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"

/*
 * __bam_delete --
 *	Delete the items referenced by a key.
 *
 * PUBLIC: int __bam_delete __P((DB *, DB_TXN *, DBT *, u_int32_t));
 */
int
__bam_delete(dbp, txn, key, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	DBC *dbc;
	DBT data;
	u_int32_t f_init, f_next;
	int ret, t_ret;

	DB_PANIC_CHECK(dbp);

	/* Check for invalid flags. */
	if ((ret =
	    __db_delchk(dbp, key, flags, F_ISSET(dbp, DB_AM_RDONLY))) != 0)
		return (ret);

	/* Allocate a cursor. */
	if ((ret = dbp->cursor(dbp, txn, &dbc, DB_WRITELOCK)) != 0)
		return (ret);

	DEBUG_LWRITE(dbc, txn, "bam_delete", key, NULL, flags);

	/*
	 * Walk a cursor through the key/data pairs, deleting as we go.  Set
	 * the DB_DBT_USERMEM flag, as this might be a threaded application
	 * and the flags checking will catch us.  We don't actually want the
	 * keys or data, so request a partial of length 0.
	 */
	memset(&data, 0, sizeof(data));
	F_SET(&data, DB_DBT_USERMEM | DB_DBT_PARTIAL);

	/* If locking, set read-modify-write flag. */
	f_init = DB_SET;
	f_next = DB_NEXT_DUP;
	if (dbp->dbenv != NULL && dbp->dbenv->lk_info != NULL) {
		f_init |= DB_RMW;
		f_next |= DB_RMW;
	}

	/* Walk through the set of key/data pairs, deleting as we go. */
	if ((ret = dbc->c_get(dbc, key, &data, f_init)) != 0)
		goto err;
	for (;;) {
		if ((ret = dbc->c_del(dbc, 0)) != 0)
			goto err;
		if ((ret = dbc->c_get(dbc, key, &data, f_next)) != 0) {
			if (ret == DB_NOTFOUND) {
				ret = 0;
				break;
			}
			goto err;
		}
	}

err:	/* Discard the cursor. */
	if ((t_ret = dbc->c_close(dbc)) != 0 &&
	    (ret == 0 || ret == DB_NOTFOUND))
		ret = t_ret;

	return (ret);
}

/*
 * __bam_ditem --
 *	Delete one or more entries from a page.
 *
 * PUBLIC: int __bam_ditem __P((DBC *, PAGE *, u_int32_t));
 */
int
__bam_ditem(dbc, h, indx)
	DBC *dbc;
	PAGE *h;
	u_int32_t indx;
{
	BINTERNAL *bi;
	BKEYDATA *bk;
	BOVERFLOW *bo;
	DB *dbp;
	u_int32_t nbytes;
	int ret;

	dbp = dbc->dbp;

	switch (TYPE(h)) {
	case P_IBTREE:
		bi = GET_BINTERNAL(h, indx);
		switch (B_TYPE(bi->type)) {
		case B_DUPLICATE:
		case B_OVERFLOW:
			nbytes = BINTERNAL_SIZE(bi->len);
			bo = (BOVERFLOW *)bi->data;
			goto offpage;
		case B_KEYDATA:
			nbytes = BINTERNAL_SIZE(bi->len);
			break;
		default:
			return (__db_pgfmt(dbp, h->pgno));
		}
		break;
	case P_IRECNO:
		nbytes = RINTERNAL_SIZE;
		break;
	case P_LBTREE:
		/*
		 * If it's a duplicate key, discard the index and don't touch
		 * the actual page item.
		 *
		 * XXX
		 * This works because no data item can have an index matching
		 * any other index so even if the data item is in a key "slot",
		 * it won't match any other index.
		 */
		if ((indx % 2) == 0) {
			/*
			 * Check for a duplicate after us on the page.  NOTE:
			 * we have to delete the key item before deleting the
			 * data item, otherwise the "indx + P_INDX" calculation
			 * won't work!
			 */
			if (indx + P_INDX < (u_int32_t)NUM_ENT(h) &&
			    h->inp[indx] == h->inp[indx + P_INDX])
				return (__bam_adjindx(dbc,
				    h, indx, indx + O_INDX, 0));
			/*
			 * Check for a duplicate before us on the page.  It
			 * doesn't matter if we delete the key item before or
			 * after the data item for the purposes of this one.
			 */
			if (indx > 0 && h->inp[indx] == h->inp[indx - P_INDX])
				return (__bam_adjindx(dbc,
				    h, indx, indx - P_INDX, 0));
		}
		/* FALLTHROUGH */
	case P_LRECNO:
		bk = GET_BKEYDATA(h, indx);
		switch (B_TYPE(bk->type)) {
		case B_DUPLICATE:
		case B_OVERFLOW:
			nbytes = BOVERFLOW_SIZE;
			bo = GET_BOVERFLOW(h, indx);

offpage:		/* Delete duplicate/offpage chains. */
			if (B_TYPE(bo->type) == B_DUPLICATE) {
				if ((ret =
				    __db_ddup(dbc, bo->pgno, __bam_free)) != 0)
					return (ret);
			} else
				if ((ret =
				    __db_doff(dbc, bo->pgno, __bam_free)) != 0)
					return (ret);
			break;
		case B_KEYDATA:
			nbytes = BKEYDATA_SIZE(bk->len);
			break;
		default:
			return (__db_pgfmt(dbp, h->pgno));
		}
		break;
	default:
		return (__db_pgfmt(dbp, h->pgno));
	}

	/* Delete the item. */
	if ((ret = __db_ditem(dbc, h, indx, nbytes)) != 0)
		return (ret);

	/* Mark the page dirty. */
	return (memp_fset(dbp->mpf, h, DB_MPOOL_DIRTY));
}

/*
 * __bam_adjindx --
 *	Adjust an index on the page.
 *
 * PUBLIC: int __bam_adjindx __P((DBC *, PAGE *, u_int32_t, u_int32_t, int));
 */
int
__bam_adjindx(dbc, h, indx, indx_copy, is_insert)
	DBC *dbc;
	PAGE *h;
	u_int32_t indx, indx_copy;
	int is_insert;
{
	DB *dbp;
	db_indx_t copy;
	int ret;

	dbp = dbc->dbp;

	/* Log the change. */
	if (DB_LOGGING(dbc) &&
	    (ret = __bam_adj_log(dbp->dbenv->lg_info, dbc->txn, &LSN(h),
	    0, dbp->log_fileid, PGNO(h), &LSN(h), indx, indx_copy,
	    (u_int32_t)is_insert)) != 0)
		return (ret);

	if (is_insert) {
		copy = h->inp[indx_copy];
		if (indx != NUM_ENT(h))
			memmove(&h->inp[indx + O_INDX], &h->inp[indx],
			    sizeof(db_indx_t) * (NUM_ENT(h) - indx));
		h->inp[indx] = copy;
		++NUM_ENT(h);
	} else {
		--NUM_ENT(h);
		if (indx != NUM_ENT(h))
			memmove(&h->inp[indx], &h->inp[indx + O_INDX],
			    sizeof(db_indx_t) * (NUM_ENT(h) - indx));
	}

	/* Mark the page dirty. */
	ret = memp_fset(dbp->mpf, h, DB_MPOOL_DIRTY);

	/* Adjust the cursors. */
	__bam_ca_di(dbp, h->pgno, indx, is_insert ? 1 : -1);
	return (0);
}

/*
 * __bam_dpage --
 *	Delete a page from the tree.
 *
 * PUBLIC: int __bam_dpage __P((DBC *, const DBT *));
 */
int
__bam_dpage(dbc, key)
	DBC *dbc;
	const DBT *key;
{
	CURSOR *cp;
	DB *dbp;
	DB_LOCK lock;
	PAGE *h;
	db_pgno_t pgno;
	int level;		/* !!!: has to hold number of tree levels. */
	int exact, ret;

	dbp = dbc->dbp;
	cp = dbc->internal;
	ret = 0;

	/*
	 * The locking protocol is that we acquire locks by walking down the
	 * tree, to avoid the obvious deadlocks.
	 *
	 * Call __bam_search to reacquire the empty leaf page, but this time
	 * get both the leaf page and it's parent, locked.  Walk back up the
	 * tree, until we have the top pair of pages that we want to delete.
	 * Once we have the top page that we want to delete locked, lock the
	 * underlying pages and check to make sure they're still empty.  If
	 * they are, delete them.
	 */
	for (level = LEAFLEVEL;; ++level) {
		/* Acquire a page and its parent, locked. */
		if ((ret =
		    __bam_search(dbc, key, S_WRPAIR, level, NULL, &exact)) != 0)
			return (ret);

		/*
		 * If we reach the root or the page isn't going to be empty
		 * when we delete one record, quit.
		 */
		h = cp->csp[-1].page;
		if (h->pgno == PGNO_ROOT || NUM_ENT(h) != 1)
			break;

		/* Release the two locked pages. */
		(void)memp_fput(dbp->mpf, cp->csp[-1].page, 0);
		(void)__BT_TLPUT(dbc, cp->csp[-1].lock);
		(void)memp_fput(dbp->mpf, cp->csp[0].page, 0);
		(void)__BT_TLPUT(dbc, cp->csp[0].lock);
	}

	/*
	 * Leave the stack pointer one after the last entry, we may be about
	 * to push more items on the stack.
	 */
	++cp->csp;

	/*
	 * cp->csp[-2].page is the top page, which we're not going to delete,
	 * and cp->csp[-1].page is the first page we are going to delete.
	 *
	 * Walk down the chain, acquiring the rest of the pages until we've
	 * retrieved the leaf page.  If we find any pages that aren't going
	 * to be emptied by the delete, someone else added something while we
	 * were walking the tree, and we discontinue the delete.
	 */
	for (h = cp->csp[-1].page;;) {
		if (ISLEAF(h)) {
			if (NUM_ENT(h) != 0)
				goto release;
			break;
		} else
			if (NUM_ENT(h) != 1)
				goto release;

		/*
		 * Get the next page, write lock it and push it onto the stack.
		 * We know it's index 0, because it can only have one element.
		 */
		pgno = TYPE(h) == P_IBTREE ?
		    GET_BINTERNAL(h, 0)->pgno : GET_RINTERNAL(h, 0)->pgno;

		if ((ret = __bam_lget(dbc, 0, pgno, DB_LOCK_WRITE, &lock)) != 0)
			goto release;
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
			goto release;
		BT_STK_PUSH(cp, h, 0, lock, ret);
	}

	/* Adjust back to reference the last page on the stack. */
	BT_STK_POP(cp);

	/* Delete the pages. */
	return (__bam_dpages(dbc));

release:
	/* Adjust back to reference the last page on the stack. */
	BT_STK_POP(cp);

	/* Discard any locked pages and return. */
	__bam_stkrel(dbc, 0);

	return (ret);
}

/*
 * __bam_dpages --
 *	Delete a set of locked pages.
 *
 * PUBLIC: int __bam_dpages __P((DBC *));
 */
int
__bam_dpages(dbc)
	DBC *dbc;
{
	CURSOR *cp;
	DB *dbp;
	DBT a, b;
	DB_LOCK c_lock, p_lock;
	EPG *epg;
	PAGE *child, *parent;
	db_indx_t nitems;
	db_pgno_t pgno;
	db_recno_t rcnt;
	int done, ret;

	dbp = dbc->dbp;
	cp = dbc->internal;
	epg = cp->sp;

	/*
	 * !!!
	 * There is an interesting deadlock situation here.  We have to relink
	 * the leaf page chain around the leaf page being deleted.  Consider
	 * a cursor walking through the leaf pages, that has the previous page
	 * read-locked and is waiting on a lock for the page we're deleting.
	 * It will deadlock here.  This is a problem, because if our process is
	 * selected to resolve the deadlock, we'll leave an empty leaf page
	 * that we can never again access by walking down the tree.  So, before
	 * we unlink the subtree, we relink the leaf page chain.
	 */
	if ((ret = __db_relink(dbc, DB_REM_PAGE, cp->csp->page, NULL, 1)) != 0)
		goto release;

	/*
	 * We have the entire stack of deletable pages locked.
	 *
	 * Delete the highest page in the tree's reference to the underlying
	 * stack of pages.  Then, release that page, letting the rest of the
	 * tree get back to business.
	 */
	if ((ret = __bam_ditem(dbc, epg->page, epg->indx)) != 0) {
release:	(void)__bam_stkrel(dbc, 0);
		return (ret);
	}

	pgno = epg->page->pgno;
	nitems = NUM_ENT(epg->page);

	(void)memp_fput(dbp->mpf, epg->page, 0);
	(void)__BT_TLPUT(dbc, epg->lock);

	/*
	 * Free the rest of the stack of pages.
	 *
	 * !!!
	 * Don't bother checking for errors.  We've unlinked the subtree from
	 * the tree, and there's no possibility of recovery outside of doing
	 * TXN rollback.
	 */
	while (++epg <= cp->csp) {
		/*
		 * Delete page entries so they will be restored as part of
		 * recovery.
		 */
		if (NUM_ENT(epg->page) != 0)
			(void)__bam_ditem(dbc, epg->page, epg->indx);

		(void)__bam_free(dbc, epg->page);
		(void)__BT_TLPUT(dbc, epg->lock);
	}
	BT_STK_CLR(cp);

	/*
	 * Try and collapse the tree a level -- this is only applicable
	 * if we've deleted the next-to-last element from the root page.
	 *
	 * There are two cases when collapsing a tree.
	 *
	 * If we've just deleted the last item from the root page, there is no
	 * further work to be done.  The code above has emptied the root page
	 * and freed all pages below it.
	 */
	if (pgno != PGNO_ROOT || nitems != 1)
		return (0);

	/*
	 * If we just deleted the next-to-last item from the root page, the
	 * tree can collapse one or more levels.  While there remains only a
	 * single item on the root page, write lock the last page referenced
	 * by the root page and copy it over the root page.  If we can't get a
	 * write lock, that's okay, the tree just stays deeper than we'd like.
	 */
	for (done = 0; !done;) {
		/* Initialize. */
		parent = child = NULL;
		p_lock = c_lock = LOCK_INVALID;

		/* Lock the root. */
		pgno = PGNO_ROOT;
		if ((ret =
		    __bam_lget(dbc, 0, pgno, DB_LOCK_WRITE, &p_lock)) != 0)
			goto stop;
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &parent)) != 0)
			goto stop;

		if (NUM_ENT(parent) != 1 ||
		    (TYPE(parent) != P_IBTREE && TYPE(parent) != P_IRECNO))
			goto stop;

		pgno = TYPE(parent) == P_IBTREE ?
		    GET_BINTERNAL(parent, 0)->pgno :
		    GET_RINTERNAL(parent, 0)->pgno;

		/* Lock the child page. */
		if ((ret =
		    __bam_lget(dbc, 0, pgno, DB_LOCK_WRITE, &c_lock)) != 0)
			goto stop;
		if ((ret = memp_fget(dbp->mpf, &pgno, 0, &child)) != 0)
			goto stop;

		/* Log the change. */
		if (DB_LOGGING(dbc)) {
			memset(&a, 0, sizeof(a));
			a.data = child;
			a.size = dbp->pgsize;
			memset(&b, 0, sizeof(b));
			b.data = P_ENTRY(parent, 0);
			b.size = BINTERNAL_SIZE(((BINTERNAL *)b.data)->len);
			__bam_rsplit_log(dbp->dbenv->lg_info, dbc->txn,
			   &child->lsn, 0, dbp->log_fileid, child->pgno, &a,
			   RE_NREC(parent), &b, &parent->lsn);
		}

		/*
		 * Make the switch.
		 *
		 * One fixup -- if the tree has record numbers and we're not
		 * converting to a leaf page, we have to preserve the total
		 * record count.  Note that we are about to overwrite everything
		 * on the parent, including its LSN.  This is actually OK,
		 * because the above log message, which describes this update,
		 * stores its LSN on the child page.  When the child is copied
		 * to the parent, the correct LSN is going to copied into
		 * place in the parent.
		 */
		COMPQUIET(rcnt, 0);
		if (TYPE(child) == P_IRECNO ||
		    (TYPE(child) == P_IBTREE && F_ISSET(dbp, DB_BT_RECNUM)))
			rcnt = RE_NREC(parent);
		memcpy(parent, child, dbp->pgsize);
		parent->pgno = PGNO_ROOT;
		if (TYPE(child) == P_IRECNO ||
		    (TYPE(child) == P_IBTREE && F_ISSET(dbp, DB_BT_RECNUM)))
			RE_NREC_SET(parent, rcnt);

		/* Mark the pages dirty. */
		memp_fset(dbp->mpf, parent, DB_MPOOL_DIRTY);
		memp_fset(dbp->mpf, child, DB_MPOOL_DIRTY);

		/* Adjust the cursors. */
		__bam_ca_rsplit(dbp, child->pgno, PGNO_ROOT);

		/*
		 * Free the page copied onto the root page and discard its
		 * lock.  (The call to __bam_free() discards our reference
		 * to the page.)
		 */
		(void)__bam_free(dbc, child);
		child = NULL;

		if (0) {
stop:			done = 1;
		}
		if (p_lock != LOCK_INVALID)
			(void)__BT_TLPUT(dbc, p_lock);
		if (parent != NULL)
			memp_fput(dbp->mpf, parent, 0);
		if (c_lock != LOCK_INVALID)
			(void)__BT_TLPUT(dbc, c_lock);
		if (child != NULL)
			memp_fput(dbp->mpf, child, 0);
	}

	return (0);
}
