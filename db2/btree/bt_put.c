/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
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
static const char sccsid[] = "@(#)bt_put.c	10.23 (Sleepycat) 8/22/97";
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

static int __bam_fixed __P((BTREE *, DBT *));
static int __bam_lookup __P((DB *, DBT *, int *));
static int __bam_ndup __P((DB *, PAGE *, u_int32_t));
static int __bam_partial __P((DB *, DBT *, PAGE *, u_int32_t));

/*
 * __bam_put --
 *	Add a new key/data pair or replace an existing pair (btree).
 *
 * PUBLIC: int __bam_put __P((DB *, DB_TXN *, DBT *, DBT *, int));
 */
int
__bam_put(argdbp, txn, key, data, flags)
	DB *argdbp;
	DB_TXN *txn;
	DBT *key, *data;
	int flags;
{
	BTREE *t;
	CURSOR c;
	DB *dbp;
	PAGE *h;
	db_indx_t indx;
	int exact, iflags, newkey, replace, ret, stack;

	DEBUG_LWRITE(argdbp, txn, "bam_put", key, data, flags);

	/* Check flags. */
	if ((ret = __db_putchk(argdbp, key, data, flags,
	    F_ISSET(argdbp, DB_AM_RDONLY), F_ISSET(argdbp, DB_AM_DUP))) != 0)
		return (ret);

	GETHANDLE(argdbp, txn, &dbp, ret);
	t = dbp->internal;

retry:	/*
	 * Find the location at which to insert.  The call to bt_lookup()
	 * leaves the returned page pinned.
	 */
	if ((ret = __bam_lookup(dbp, key, &exact)) != 0) {
		PUTHANDLE(dbp);
		return (ret);
	}
	h = t->bt_csp->page;
	indx = t->bt_csp->indx;
	stack = 1;

	/*
	 * If an identical key is already in the tree, and DB_NOOVERWRITE is
	 * set, an error is returned.  If an identical key is already in the
	 * tree and DB_NOOVERWRITE is not set, the key is either added (when
	 * duplicates are permitted) or an error is returned.  The exception
	 * is when the item located is referenced by a cursor and marked for
	 * deletion, in which case we permit the overwrite and flag the cursor.
	 */
	replace = 0;
	if (exact && flags == DB_NOOVERWRITE) {
		if (!GET_BKEYDATA(h, indx + O_INDX)->deleted) {
			ret = DB_KEYEXIST;
			goto err;
		}
		replace = 1;
		__bam_ca_replace(dbp, h->pgno, indx, REPLACE_SETUP);
	}

	/*
	 * If we're inserting into the first or last page of the tree,
	 * remember where we did it so we can do fast lookup next time.
	 *
	 * XXX
	 * Does reverse order still work (did it ever!?!?)
	 */
	t->bt_lpgno =
	    h->next_pgno == PGNO_INVALID || h->prev_pgno == PGNO_INVALID ?
	    h->pgno : PGNO_INVALID;

	/*
	 * Select the arguments for __bam_iitem() and do the insert.  If the
	 * key is an exact match, we're either adding a new duplicate at the
	 * end of the duplicate set, or we're replacing the data item with a
	 * new data item.  If the key isn't an exact match, we're inserting
	 * a new key/data pair, before the search location.
	 */
	newkey = dbp->type == DB_BTREE && !exact;
	if (exact) {
		if (F_ISSET(dbp, DB_AM_DUP)) {
			/*
			 * Make sure that we're not looking at a page of
			 * duplicates -- if so, move to the last entry on
			 * that page.
			 */
			c.page = h;
			c.pgno = h->pgno;
			c.indx = indx;
			c.dpgno = PGNO_INVALID;
			c.dindx = 0;
			if ((ret =
			    __bam_ovfl_chk(dbp, &c, indx + O_INDX, 1)) != 0)
				goto err;
			if (c.dpgno != PGNO_INVALID) {
				/*
				 * XXX
				 * The __bam_ovfl_chk() routine memp_fput() the
				 * current page and acquired a new one, but did
				 * not do anything about the lock we're holding.
				 */
				t->bt_csp->page = h = c.page;
				indx = c.dindx;
			}
			iflags = DB_AFTER;
		} else
			iflags = DB_CURRENT;
	} else
		iflags = DB_BEFORE;

	/*
	 * The pages we're using may be modified by __bam_iitem(), so make
	 * sure we reset the stack.
	 */
	ret = __bam_iitem(dbp,
	    &h, &indx, key, data, iflags, newkey ? BI_NEWKEY : 0);
	t->bt_csp->page = h;
	t->bt_csp->indx = indx;

	switch (ret) {
	case 0:
		/*
		 * Done.  Clean up the cursor, and, if we're doing record
		 * numbers, adjust the internal page counts.
		 */
		if (replace)
			__bam_ca_replace(dbp, h->pgno, indx, REPLACE_SUCCESS);

		if (!replace && F_ISSET(dbp, DB_BT_RECNUM))
			ret = __bam_adjust(dbp, t, 1);
		break;
	case DB_NEEDSPLIT:
		/*
		 * We have to split the page.  Back out the cursor setup,
		 * discard the stack of pages, and do the split.
		 */
		if (replace) {
			replace = 0;
			__bam_ca_replace(dbp, h->pgno, indx, REPLACE_FAILED);
		}

		(void)__bam_stkrel(dbp);
		stack = 0;

		if ((ret = __bam_split(dbp, key)) != 0)
			break;

		goto retry;
		/* NOTREACHED */
	default:
		if (replace)
			__bam_ca_replace(dbp, h->pgno, indx, REPLACE_FAILED);
		break;
	}

err:	if (stack)
		(void)__bam_stkrel(dbp);

	PUTHANDLE(dbp);
	return (ret);
}

/*
 * __bam_lookup --
 *	Find the right location in the tree for the key.
 */
static int
__bam_lookup(dbp, key, exactp)
	DB *dbp;
	DBT *key;
	int *exactp;
{
	BTREE *t;
	DB_LOCK lock;
	EPG e;
	PAGE *h;
	db_indx_t indx;
	int cmp, ret;

	t = dbp->internal;
	h = NULL;

	/*
	 * Record numbers can't be fast-tracked, we have to lock the entire
	 * tree.
	 */
	if (F_ISSET(dbp, DB_BT_RECNUM))
		goto slow;

	/* Check to see if we've been seeing sorted input. */
	if (t->bt_lpgno == PGNO_INVALID)
		goto slow;

	/*
	 * Retrieve the page on which we did the last insert.  It's okay if
	 * it doesn't exist, or if it's not the page type we expect, it just
	 * means that the world changed.
	 */
	if (__bam_lget(dbp, 0, t->bt_lpgno, DB_LOCK_WRITE, &lock))
		goto miss;
	if (__bam_pget(dbp, &h, &t->bt_lpgno, 0)) {
		(void)__BT_LPUT(dbp, lock);
		goto miss;
	}
	if (TYPE(h) != P_LBTREE)
		goto miss;
	if (NUM_ENT(h) == 0)
		goto miss;

	/*
	 * We have to be at the end or beginning of the tree to know that
	 * we're inserting in a sort order.  If that's the case and we're
	 * in the right order in comparison to the first/last key/data pair,
	 * we have the right position.
	 */
	if (h->next_pgno == PGNO_INVALID) {
		e.page = h;
		e.indx = NUM_ENT(h) - P_INDX;
		if ((cmp = __bam_cmp(dbp, key, &e)) >= 0) {
			if (cmp > 0)
				e.indx += P_INDX;
			goto fast;
		}
	}
	if (h->prev_pgno == PGNO_INVALID) {
		e.page = h;
		e.indx = 0;
		if ((cmp = __bam_cmp(dbp, key, &e)) <= 0) {
			/*
			 * We're doing a put, so we want to insert as the last
			 * of any set of duplicates.
			 */
			if (cmp == 0) {
				for (indx = 0;
				    indx < (db_indx_t)(NUM_ENT(h) - P_INDX) &&
				    h->inp[indx] == h->inp[indx + P_INDX];
				    indx += P_INDX);
				e.indx = indx;
			}
			goto fast;
		}
	}
	goto miss;

	/* Set the exact match flag in case we've already inserted this key. */
fast:	*exactp = cmp == 0;

	/* Enter the entry in the stack. */
	BT_STK_CLR(t);
	BT_STK_ENTER(t, e.page, e.indx, lock, ret);
	if (ret != 0)
		return (ret);

	++t->lstat.bt_cache_hit;
	return (0);

miss:	++t->lstat.bt_cache_miss;
	if (h != NULL) {
		(void)memp_fput(dbp->mpf, h, 0);
		(void)__BT_LPUT(dbp, lock);
	}

slow:	return (__bam_search(dbp, key, S_INSERT, 1, NULL, exactp));
}

/*
 * OVPUT --
 *	Copy an overflow item onto a page.
 */
#undef	OVPUT
#define	OVPUT(h, indx, bo) do {						\
	DBT __hdr;							\
	memset(&__hdr, 0, sizeof(__hdr));				\
	__hdr.data = &bo;						\
	__hdr.size = BOVERFLOW_SIZE;					\
	if ((ret = __db_pitem(dbp,					\
	    h, indx, BOVERFLOW_SIZE, &__hdr, NULL)) != 0)		\
		return (ret);						\
} while (0)

/*
 * __bam_iitem --
 *	Insert an item into the tree.
 *
 * PUBLIC: int __bam_iitem __P((DB *,
 * PUBLIC:    PAGE **, db_indx_t *, DBT *, DBT *, int, int));
 */
int
__bam_iitem(dbp, hp, indxp, key, data, op, flags)
	DB *dbp;
	PAGE **hp;
	db_indx_t *indxp;
	DBT *key, *data;
	int op, flags;
{
	BTREE *t;
	BKEYDATA *bk;
	BOVERFLOW kbo, dbo;
	DBT tdbt;
	PAGE *h;
	db_indx_t indx;
	u_int32_t have_bytes, need_bytes, needed;
	int bigkey, bigdata, dcopy, dupadjust, ret;

	t = dbp->internal;
	h = *hp;
	indx = *indxp;

	dupadjust = 0;
	bk = NULL;			/* XXX: Shut the compiler up. */

	/*
	 * If it's a page of duplicates, call the common code to do the work.
	 *
	 * !!!
	 * Here's where the hp and indxp are important.  The duplicate code
	 * may decide to rework/rearrange the pages and indices we're using,
	 * so the caller must understand that the stack has to change.
	 */
	if (TYPE(h) == P_DUPLICATE) {
		/* Adjust the index for the new item if it's a DB_AFTER op. */
		if (op == DB_AFTER)
			++*indxp;

		/* Remove the current item if it's a DB_CURRENT op. */
		if (op == DB_CURRENT && (ret = __db_ditem(dbp, *hp, *indxp,
		    BKEYDATA_SIZE(GET_BKEYDATA(*hp, *indxp)->len))) != 0)
			return (ret);

		/* Put the new/replacement item onto the page. */
		return (__db_dput(dbp, data, hp, indxp, __bam_new));
	}

	/*
	 * XXX
	 * Handle partial puts.
	 *
	 * This is truly awful from a performance standput.  We don't optimize
	 * for partial puts at all, we delete the record and add it back in,
	 * regardless of size or if we're simply overwriting current data.
	 * The hash access method does this a lot better than we do, and we're
	 * eventually going to have to fix it.
	 */
	if (F_ISSET(data, DB_DBT_PARTIAL)) {
		tdbt = *data;
		if ((ret = __bam_partial(dbp, &tdbt, h, indx)) != 0)
			return (ret);
		data = &tdbt;
	}

	/* If it's a short fixed-length record, fix it up. */
	if (F_ISSET(dbp, DB_RE_FIXEDLEN) && data->size != t->bt_recno->re_len) {
		tdbt = *data;
		if ((ret = __bam_fixed(t, &tdbt)) != 0)
			return (ret);
		data = &tdbt;
	}

	/*
	 * If the key or data item won't fit on a page, store it in the
	 * overflow pages.
	 *
	 * !!!
	 * From this point on, we have to recover the allocated overflow
	 * pages on error.
	 */
	bigkey = bigdata = 0;
	if (LF_ISSET(BI_NEWKEY) && key->size > t->bt_ovflsize) {
		kbo.deleted = 0;
		kbo.type = B_OVERFLOW;
		kbo.tlen = key->size;
		if ((ret = __db_poff(dbp, key, &kbo.pgno, __bam_new)) != 0)
			goto err;
		bigkey = 1;
	}
	if (data->size > t->bt_ovflsize) {
		dbo.deleted = 0;
		dbo.type = B_OVERFLOW;
		dbo.tlen = data->size;
		if ((ret = __db_poff(dbp, data, &dbo.pgno, __bam_new)) != 0)
			goto err;
		bigdata = 1;
	}

	dcopy = 0;
	needed = 0;
	if (LF_ISSET(BI_NEWKEY)) {
		/* If BI_NEWKEY is set we're adding a new key and data pair. */
		if (bigkey)
			needed += BOVERFLOW_PSIZE;
		else
			needed += BKEYDATA_PSIZE(key->size);
		if (bigdata)
			needed += BOVERFLOW_PSIZE;
		else
			needed += BKEYDATA_PSIZE(data->size);
	} else {
		/*
		 * We're either overwriting the data item of a key/data pair
		 * or we're adding the data item only, i.e. a new duplicate.
		 */
		if (op == DB_CURRENT) {
			bk = GET_BKEYDATA(h,
			    indx + (TYPE(h) == P_LBTREE ? O_INDX : 0));
			if (bk->type == B_OVERFLOW)
				have_bytes = BOVERFLOW_PSIZE;
			else
				have_bytes = BKEYDATA_PSIZE(bk->len);
			need_bytes = 0;
		} else {
			have_bytes = 0;
			need_bytes = sizeof(db_indx_t);
		}
		if (bigdata)
			need_bytes += BOVERFLOW_PSIZE;
		else
			need_bytes += BKEYDATA_PSIZE(data->size);

		/*
		 * If we're overwriting a data item, we copy it if it's not a
		 * special record type and it's the same size (including any
		 * alignment) and do a delete/insert otherwise.
		 */
		if (op == DB_CURRENT && !bigdata &&
		    bk->type == B_KEYDATA && have_bytes == need_bytes)
			dcopy = 1;
		if (have_bytes < need_bytes)
			needed += need_bytes - have_bytes;
	}

	/*
	 * If there's not enough room, or the user has put a ceiling on the
	 * number of keys permitted in the page, split the page.
	 *
	 * XXX
	 * The t->bt_maxkey test here may be insufficient -- do we have to
	 * check in the btree split code, so we don't undo it there!?!?
	 */
	if (P_FREESPACE(h) < needed ||
	    (t->bt_maxkey != 0 && NUM_ENT(h) > t->bt_maxkey)) {
		ret = DB_NEEDSPLIT;
		goto err;
	}

	/*
	 * The code breaks it up into six cases:
	 *
	 * 1. Append a new key/data pair.
	 * 2. Insert a new key/data pair.
	 * 3. Copy the data item.
	 * 4. Delete/insert the data item.
	 * 5. Append a new data item.
	 * 6. Insert a new data item.
	 */
	if (LF_ISSET(BI_NEWKEY)) {
		switch (op) {
		case DB_AFTER:		/* 1. Append a new key/data pair. */
			indx += 2;
			*indxp += 2;
			break;
		case DB_BEFORE:		/* 2. Insert a new key/data pair. */
			break;
		default:
			abort();
		}

		/* Add the key. */
		if (bigkey)
			OVPUT(h, indx, kbo);
		else {
			DBT __data;
			memset(&__data, 0, sizeof(__data));
			__data.data = key->data;
			__data.size = key->size;
			if ((ret = __db_pitem(dbp, h, indx,
			    BKEYDATA_SIZE(key->size), NULL, &__data)) != 0)
				goto err;
		}
		++indx;
	} else {
		switch (op) {
		case DB_CURRENT:	/* 3. Copy the data item. */
			/*
			 * If we're not logging and it's possible, overwrite
			 * the current item.
			 *
			 * XXX
			 * We should add a separate logging message so that
			 * we can do this anytime it's possible, including
			 * for partial record puts.
			 */
			if (dcopy && !DB_LOGGING(dbp)) {
				bk->len = data->size;
				memcpy(bk->data, data->data, data->size);
				goto done;
			}
					/* 4. Delete/insert the data item. */
			if (TYPE(h) == P_LBTREE)
				++indx;
			if ((ret = __bam_ditem(dbp, h, indx)) != 0)
				goto err;
			break;
		case DB_AFTER:		/* 5. Append a new data item. */
			if (TYPE(h) == P_LBTREE) {
				/*
				 * Adjust the cursor and copy in the key for
				 * the duplicate.
				 */
				if ((ret = __bam_adjindx(dbp,
				    h, indx + P_INDX, indx, 1)) != 0)
					goto err;

				indx += 3;
				dupadjust = 1;

				*indxp += 2;
			} else {
				++indx;
				__bam_ca_di(dbp, h->pgno, indx, 1);

				*indxp += 1;
			}
			break;
		case DB_BEFORE:		/* 6. Insert a new data item. */
			if (TYPE(h) == P_LBTREE) {
				/*
				 * Adjust the cursor and copy in the key for
				 * the duplicate.
				 */
				if ((ret =
				    __bam_adjindx(dbp, h, indx, indx, 1)) != 0)
					goto err;

				++indx;
				dupadjust = 1;
			} else
				__bam_ca_di(dbp, h->pgno, indx, 1);
			break;
		default:
			abort();
		}
	}

	/* Add the data. */
	if (bigdata)
		OVPUT(h, indx, dbo);
	else {
		BKEYDATA __bk;
		DBT __hdr, __data;
		memset(&__data, 0, sizeof(__data));
		__data.data = data->data;
		__data.size = data->size;

		if (LF_ISSET(BI_DELETED)) {
			__bk.len = __data.size;
			__bk.deleted = 1;
			__bk.type = B_KEYDATA;
			__hdr.data = &__bk;
			__hdr.size = SSZA(BKEYDATA, data);
			ret = __db_pitem(dbp, h, indx,
			    BKEYDATA_SIZE(__data.size), &__hdr, &__data);
		} else
			ret = __db_pitem(dbp, h, indx,
			    BKEYDATA_SIZE(data->size), NULL, &__data);
		if (ret != 0)
			goto err;
	}

done:	++t->lstat.bt_added;

	ret = memp_fset(dbp->mpf, h, DB_MPOOL_DIRTY);

	/*
	 * If the page is at least 50% full, and we added a duplicate, see if
	 * that set of duplicates takes up at least 25% of the space.  If it
	 * does, move it off onto its own page.
	 */
	if (dupadjust && P_FREESPACE(h) <= dbp->pgsize / 2) {
		--indx;
		if ((ret = __bam_ndup(dbp, h, indx)) != 0)
			goto err;
	}

	if (t->bt_recno != NULL)
		F_SET(t->bt_recno, RECNO_MODIFIED);

	if (0) {
err:		if (bigkey)
			(void)__db_doff(dbp, kbo.pgno, __bam_free);
		if (bigdata)
			(void)__db_doff(dbp, dbo.pgno, __bam_free);
	}
	return (ret);
}

/*
 * __bam_ndup --
 *	Check to see if the duplicate set at indx should have its own page.
 *	If it should, create it.
 */
static int
__bam_ndup(dbp, h, indx)
	DB *dbp;
	PAGE *h;
	u_int32_t indx;
{
	BKEYDATA *bk;
	BOVERFLOW bo;
	DBT hdr;
	PAGE *cp;
	db_indx_t cnt, cpindx, first, sz;
	int ret;

	while (indx > 0 && h->inp[indx] == h->inp[indx - P_INDX])
		indx -= P_INDX;
	for (cnt = 0, sz = 0, first = indx;; ++cnt, indx += P_INDX) {
		if (indx >= NUM_ENT(h) || h->inp[first] != h->inp[indx])
			break;
		bk = GET_BKEYDATA(h, indx);
		sz += bk->type == B_KEYDATA ?
		    BKEYDATA_PSIZE(bk->len) : BOVERFLOW_PSIZE;
		bk = GET_BKEYDATA(h, indx + O_INDX);
		sz += bk->type == B_KEYDATA ?
		    BKEYDATA_PSIZE(bk->len) : BOVERFLOW_PSIZE;
	}

	/*
	 * If this set of duplicates is using more than 25% of the page, move
	 * them off.  The choice of 25% is a WAG, but it has to be small enough
	 * that we can always split regardless of the presence of duplicates.
	 */
	if (sz < dbp->pgsize / 4)
		return (0);

	/* Get a new page. */
	if ((ret = __bam_new(dbp, P_DUPLICATE, &cp)) != 0)
		return (ret);

	/*
	 * Move this set of duplicates off the page.  First points to the first
	 * key of the first duplicate key/data pair, cnt is the number of pairs
	 * we're dealing with.
	 */
	memset(&hdr, 0, sizeof(hdr));
	for (indx = first + O_INDX, cpindx = 0;; ++cpindx) {
		/* Copy the entry to the new page. */
		bk = GET_BKEYDATA(h, indx);
		hdr.data = bk;
		hdr.size = bk->type == B_KEYDATA ?
		    BKEYDATA_SIZE(bk->len) : BOVERFLOW_SIZE;
		if ((ret =
		    __db_pitem(dbp, cp, cpindx, hdr.size, &hdr, NULL)) != 0)
			goto err;

		/*
		 * Move cursors referencing the old entry to the new entry.
		 * Done after the page put because __db_pitem() adjusts
		 * cursors on the new page, and before the delete because
		 * __db_ditem adjusts cursors on the old page.
		 */
		__bam_ca_dup(dbp,
		    PGNO(h), first, indx - O_INDX, PGNO(cp), cpindx);

		/* Delete the data item. */
		if ((ret = __db_ditem(dbp, h, indx, hdr.size)) != 0)
			goto err;

		/* Delete all but the first reference to the key. */
		if (--cnt == 0)
			break;
		if ((ret = __bam_adjindx(dbp, h, indx, first, 0)) != 0)
			goto err;
	}

	/* Put in a new data item that points to the duplicates page. */
	bo.deleted = 0;
	bo.type = B_DUPLICATE;
	bo.pgno = cp->pgno;
	bo.tlen = 0;

	OVPUT(h, indx, bo);

	return (memp_fput(dbp->mpf, cp, DB_MPOOL_DIRTY));

err:	(void)__bam_free(dbp, cp);
	return (ret);
}

/*
 * __bam_fixed --
 *	Build the real record for a fixed length put.
 */
static int
__bam_fixed(t, dbt)
	BTREE *t;
	DBT *dbt;
{
	RECNO *rp;

	rp = t->bt_recno;

	/*
	 * If using fixed-length records, and the record is long, return
	 * EINVAL.  If it's short, pad it out.  Use the record data return
	 * memory, it's only short-term.
	 */
	if (dbt->size > rp->re_len)
		return (EINVAL);
	if (t->bt_rdata.ulen < rp->re_len) {
		t->bt_rdata.data = t->bt_rdata.data == NULL ?
		    (void *)malloc(rp->re_len) :
		    (void *)realloc(t->bt_rdata.data, rp->re_len);
		if (t->bt_rdata.data == NULL) {
			t->bt_rdata.ulen = 0;
			return (ENOMEM);
		}
		t->bt_rdata.ulen = rp->re_len;
	}
	memcpy(t->bt_rdata.data, dbt->data, dbt->size);
	memset((u_int8_t *)t->bt_rdata.data + dbt->size,
	    rp->re_pad, rp->re_len - dbt->size);

	/* Set the DBT to reference our new record. */
	t->bt_rdata.size = rp->re_len;
	t->bt_rdata.dlen = 0;
	t->bt_rdata.doff = 0;
	t->bt_rdata.flags = 0;
	*dbt = t->bt_rdata;
	return (0);
}

/*
 * __bam_partial --
 *	Build the real record for a partial put.
 */
static int
__bam_partial(dbp, dbt, h, indx)
	DB *dbp;
	DBT *dbt;
	PAGE *h;
	u_int32_t indx;
{
	BTREE *t;
	BKEYDATA *bk, tbk;
	BOVERFLOW *bo;
	DBT copy;
	u_int32_t len, nbytes, tlen;
	int ret;
	u_int8_t *p;

	bo = NULL;			/* XXX: Shut the compiler up. */
	t = dbp->internal;

	/*
	 * Figure out how much total space we'll need.  Worst case is where
	 * the record is 0 bytes long, in which case doff causes the record
	 * to extend, and the put data is appended to it.
	 */
	if (indx < NUM_ENT(h)) {
		bk = GET_BKEYDATA(h, indx + (TYPE(h) == P_LBTREE ? O_INDX : 0));
		if (bk->type == B_OVERFLOW) {
			bo = (BOVERFLOW *)bk;
			nbytes = bo->tlen;
		} else
			nbytes = bk->len;
	} else {
		bk = &tbk;
		bk->type = B_KEYDATA;
		nbytes = bk->len = 0;
	}
	nbytes += dbt->doff + dbt->size + dbt->dlen;

	/* Allocate the space. */
	if (t->bt_rdata.ulen < nbytes) {
		t->bt_rdata.data = t->bt_rdata.data == NULL ?
		    (void *)malloc(nbytes) :
		    (void *)realloc(t->bt_rdata.data, nbytes);
		if (t->bt_rdata.data == NULL) {
			t->bt_rdata.ulen = 0;
			return (ENOMEM);
		}
		t->bt_rdata.ulen = nbytes;
	}

	/* We use nul bytes for extending the record, get it over with. */
	memset(t->bt_rdata.data, 0, nbytes);

	tlen = 0;
	if (bk->type == B_OVERFLOW) {
		/* Take up to doff bytes from the record. */
		memset(&copy, 0, sizeof(copy));
		if ((ret = __db_goff(dbp, &copy, bo->tlen,
		    bo->pgno, &t->bt_rdata.data, &t->bt_rdata.ulen)) != 0)
			return (ret);
		tlen += dbt->doff;

		/*
		 * If the original record was larger than the offset:
		 *	If dlen > size, shift the remaining data down.
		 *	If dlen < size, shift the remaining data up.
		 * Use memmove(), the regions may overlap.
		 */
		p = t->bt_rdata.data;
		if (bo->tlen > dbt->doff)
			if (dbt->dlen > dbt->size) {
				tlen += len = bo->tlen -
				    dbt->doff - (dbt->dlen - dbt->size);
				memmove(p + dbt->doff + dbt->size,
				    p + dbt->doff + dbt->dlen, len);
			} else if (dbt->dlen < dbt->size) {
				tlen += len = bo->tlen -
				    dbt->doff - (dbt->size - dbt->dlen);
				memmove(p + dbt->doff + dbt->dlen,
				    p + dbt->doff + dbt->size, len);
			} else
				tlen += bo->tlen - dbt->doff;

		/* Copy in the user's data. */
		memcpy((u_int8_t *)t->bt_rdata.data + dbt->doff,
		    dbt->data, dbt->size);
		tlen += dbt->size;
	} else {
		/* Take up to doff bytes from the record. */
		memcpy(t->bt_rdata.data,
		    bk->data, dbt->doff > bk->len ? bk->len : dbt->doff);
		tlen += dbt->doff;

		/* Copy in the user's data. */
		memcpy((u_int8_t *)t->bt_rdata.data +
		    dbt->doff, dbt->data, dbt->size);
		tlen += dbt->size;

		/* Copy in any remaining data. */
		len = dbt->doff + dbt->dlen;
		if (bk->len > len) {
			memcpy((u_int8_t *)t->bt_rdata.data + dbt->doff +
			    dbt->size, bk->data + len, bk->len - len);
			tlen += bk->len - len;
		}
	}

	/* Set the DBT to reference our new record. */
	t->bt_rdata.size = tlen;
	t->bt_rdata.dlen = 0;
	t->bt_rdata.doff = 0;
	t->bt_rdata.flags = 0;
	*dbt = t->bt_rdata;
	return (0);
}
