/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994
 *	Margo Seltzer.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
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
static const char sccsid[] = "@(#)hash_page.c	10.55 (Sleepycat) 1/3/99";
#endif /* not lint */

/*
 * PACKAGE:  hashing
 *
 * DESCRIPTION:
 *      Page manipulation for hashing package.
 *
 * ROUTINES:
 *
 * External
 *      __get_page
 *      __add_ovflpage
 *      __overflow_page
 * Internal
 *      open_temp
 */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "hash.h"

static int __ham_lock_bucket __P((DBC *, db_lockmode_t));

#ifdef DEBUG_SLOW
static void  __account_page(DB *, db_pgno_t, int);
#endif

/*
 * PUBLIC: int __ham_item __P((DBC *, db_lockmode_t));
 */
int
__ham_item(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	db_pgno_t next_pgno;
	int ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;

	if (F_ISSET(hcp, H_DELETED))
		return (EINVAL);
	F_CLR(hcp, H_OK | H_NOMORE);

	/* Check if we need to get a page for this cursor. */
	if ((ret = __ham_get_cpage(dbc, mode)) != 0)
		return (ret);

	/* Check if we are looking for space in which to insert an item. */
	if (hcp->seek_size && hcp->seek_found_page == PGNO_INVALID
	    && hcp->seek_size < P_FREESPACE(hcp->pagep))
		hcp->seek_found_page = hcp->pgno;

	/* Check if we need to go on to the next page. */
	if (F_ISSET(hcp, H_ISDUP) && hcp->dpgno == PGNO_INVALID)
		/*
		 * ISDUP is set, and offset is at the beginning of the datum.
		 * We need to grab the length of the datum, then set the datum
		 * pointer to be the beginning of the datum.
		 */
		memcpy(&hcp->dup_len,
		    HKEYDATA_DATA(H_PAIRDATA(hcp->pagep, hcp->bndx)) +
		    hcp->dup_off, sizeof(db_indx_t));
	else if (F_ISSET(hcp, H_ISDUP)) {
		/* Make sure we're not about to run off the page. */
		if (hcp->dpagep == NULL && (ret = __ham_get_page(dbp,
		    hcp->dpgno, &hcp->dpagep)) != 0)
			return (ret);

		if (hcp->dndx >= NUM_ENT(hcp->dpagep)) {
			if (NEXT_PGNO(hcp->dpagep) == PGNO_INVALID) {
				if (F_ISSET(hcp, H_DUPONLY)) {
					F_CLR(hcp, H_OK);
					F_SET(hcp, H_NOMORE);
					return (0);
				}
				if ((ret = __ham_put_page(dbp,
				    hcp->dpagep, 0)) != 0)
					return (ret);
				F_CLR(hcp, H_ISDUP);
				hcp->dpagep = NULL;
				hcp->dpgno = PGNO_INVALID;
				hcp->dndx = NDX_INVALID;
				hcp->bndx++;
			} else if ((ret = __ham_next_cpage(dbc,
			    NEXT_PGNO(hcp->dpagep), 0, H_ISDUP)) != 0)
				return (ret);
		}
	}

	if (hcp->bndx >= (db_indx_t)H_NUMPAIRS(hcp->pagep)) {
		/* Fetch next page. */
		if (NEXT_PGNO(hcp->pagep) == PGNO_INVALID) {
			F_SET(hcp, H_NOMORE);
			if (hcp->dpagep != NULL &&
			    (ret = __ham_put_page(dbp, hcp->dpagep, 0)) != 0)
				return (ret);
			hcp->dpgno = PGNO_INVALID;
			return (DB_NOTFOUND);
		}
		next_pgno = NEXT_PGNO(hcp->pagep);
		hcp->bndx = 0;
		if ((ret = __ham_next_cpage(dbc, next_pgno, 0, 0)) != 0)
			return (ret);
	}

	F_SET(hcp, H_OK);
	return (0);
}

/*
 * PUBLIC: int __ham_item_reset __P((DBC *));
 */
int
__ham_item_reset(dbc)
	DBC *dbc;
{
	HASH_CURSOR *hcp;
	DB *dbp;
	int ret;

	ret = 0;
	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	if (hcp->pagep != NULL)
		ret = __ham_put_page(dbp, hcp->pagep, 0);
	if (ret == 0 && hcp->dpagep != NULL)
		ret = __ham_put_page(dbp, hcp->dpagep, 0);

	__ham_item_init(hcp);
	return (ret);
}

/*
 * PUBLIC: void __ham_item_init __P((HASH_CURSOR *));
 */
void
__ham_item_init(hcp)
	HASH_CURSOR *hcp;
{
	/*
	 * If this cursor still holds any locks, we must
	 * release them if we are not running with transactions.
	 */
	if (hcp->lock && hcp->dbc->txn == NULL)
	    (void)lock_put(hcp->dbc->dbp->dbenv->lk_info, hcp->lock);

	/*
	 * The following fields must *not* be initialized here
	 * because they may have meaning across inits.
	 * 	hlock, hdr, split_buf, stats
	 */
	hcp->bucket = BUCKET_INVALID;
	hcp->lbucket = BUCKET_INVALID;
	hcp->lock = 0;
	hcp->pagep = NULL;
	hcp->pgno = PGNO_INVALID;
	hcp->bndx = NDX_INVALID;
	hcp->dpagep = NULL;
	hcp->dpgno = PGNO_INVALID;
	hcp->dndx = NDX_INVALID;
	hcp->dup_off = 0;
	hcp->dup_len = 0;
	hcp->dup_tlen = 0;
	hcp->seek_size = 0;
	hcp->seek_found_page = PGNO_INVALID;
	hcp->flags = 0;
}

/*
 * PUBLIC: int __ham_item_done __P((DBC *, int));
 */
int
__ham_item_done(dbc, dirty)
	DBC *dbc;
	int dirty;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	int ret, t_ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	t_ret = ret = 0;

	if (hcp->pagep)
		ret = __ham_put_page(dbp, hcp->pagep,
		    dirty && hcp->dpagep == NULL);
	hcp->pagep = NULL;

	if (hcp->dpagep)
		t_ret = __ham_put_page(dbp, hcp->dpagep, dirty);
	hcp->dpagep = NULL;

	if (ret == 0 && t_ret != 0)
		ret = t_ret;

	/*
	 * We don't throw out the page number since we might want to
	 * continue getting on this page.
	 */
	return (ret != 0 ? ret : t_ret);
}

/*
 * Returns the last item in a bucket.
 *
 * PUBLIC: int __ham_item_last __P((DBC *, db_lockmode_t));
 */
int
__ham_item_last(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	HASH_CURSOR *hcp;
	int ret;

	hcp = (HASH_CURSOR *)dbc->internal;
	if ((ret = __ham_item_reset(dbc)) != 0)
		return (ret);

	hcp->bucket = hcp->hdr->max_bucket;
	F_SET(hcp, H_OK);
	return (__ham_item_prev(dbc, mode));
}

/*
 * PUBLIC: int __ham_item_first __P((DBC *, db_lockmode_t));
 */
int
__ham_item_first(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	HASH_CURSOR *hcp;
	int ret;

	hcp = (HASH_CURSOR *)dbc->internal;
	if ((ret = __ham_item_reset(dbc)) != 0)
		return (ret);
	F_SET(hcp, H_OK);
	hcp->bucket = 0;
	return (__ham_item_next(dbc, mode));
}

/*
 * __ham_item_prev --
 *	Returns a pointer to key/data pair on a page.  In the case of
 *	bigkeys, just returns the page number and index of the bigkey
 *	pointer pair.
 *
 * PUBLIC: int __ham_item_prev __P((DBC *, db_lockmode_t));
 */
int
__ham_item_prev(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	db_pgno_t next_pgno;
	int ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	/*
	 * There are N cases for backing up in a hash file.
	 * Case 1: In the middle of a page, no duplicates, just dec the index.
	 * Case 2: In the middle of a duplicate set, back up one.
	 * Case 3: At the beginning of a duplicate set, get out of set and
	 *	back up to next key.
	 * Case 4: At the beginning of a page; go to previous page.
	 * Case 5: At the beginning of a bucket; go to prev bucket.
	 */
	F_CLR(hcp, H_OK | H_NOMORE | H_DELETED);

	/*
	 * First handle the duplicates.  Either you'll get the key here
	 * or you'll exit the duplicate set and drop into the code below
	 * to handle backing up through keys.
	 */
	if (F_ISSET(hcp, H_ISDUP)) {
		if (hcp->dpgno == PGNO_INVALID) {
			/* Duplicates are on-page. */
			if (hcp->dup_off != 0) {
				if ((ret = __ham_get_cpage(dbc, mode)) != 0)
					return (ret);
				else {
					HASH_CURSOR *h;
					h = hcp;
					memcpy(&h->dup_len, HKEYDATA_DATA(
					    H_PAIRDATA(h->pagep, h->bndx))
					    + h->dup_off - sizeof(db_indx_t),
					    sizeof(db_indx_t));
					hcp->dup_off -=
					    DUP_SIZE(hcp->dup_len);
					hcp->dndx--;
					return (__ham_item(dbc, mode));
				}
			}
		} else if (hcp->dndx > 0) {	/* Duplicates are off-page. */
			hcp->dndx--;
			return (__ham_item(dbc, mode));
		} else if ((ret = __ham_get_cpage(dbc, mode)) != 0)
			return (ret);
		else if (PREV_PGNO(hcp->dpagep) == PGNO_INVALID) {
			if (F_ISSET(hcp, H_DUPONLY)) {
				F_CLR(hcp, H_OK);
				F_SET(hcp, H_NOMORE);
				return (0);
			} else {
				F_CLR(hcp, H_ISDUP); /* End of dups */
				hcp->dpgno = PGNO_INVALID;
				if (hcp->dpagep != NULL)
					(void)__ham_put_page(dbp,
					    hcp->dpagep, 0);
				hcp->dpagep = NULL;
			}
		} else if ((ret = __ham_next_cpage(dbc,
		    PREV_PGNO(hcp->dpagep), 0, H_ISDUP)) != 0)
			return (ret);
		else {
			hcp->dndx = NUM_ENT(hcp->pagep) - 1;
			return (__ham_item(dbc, mode));
		}
	}

	/*
	 * If we get here, we are not in a duplicate set, and just need
	 * to back up the cursor.  There are still three cases:
	 * midpage, beginning of page, beginning of bucket.
	 */

	if (F_ISSET(hcp, H_DUPONLY)) {
		F_CLR(hcp, H_OK);
		F_SET(hcp, H_NOMORE);
		return (0);
	}

	if (hcp->bndx == 0) { 		/* Beginning of page. */
		if ((ret = __ham_get_cpage(dbc, mode)) != 0)
			return (ret);
		hcp->pgno = PREV_PGNO(hcp->pagep);
		if (hcp->pgno == PGNO_INVALID) {
			/* Beginning of bucket. */
			F_SET(hcp, H_NOMORE);
			return (DB_NOTFOUND);
		} else if ((ret =
		    __ham_next_cpage(dbc, hcp->pgno, 0, 0)) != 0)
			return (ret);
		else
			hcp->bndx = H_NUMPAIRS(hcp->pagep);
	}

	/*
	 * Either we've got the cursor set up to be decremented, or we
	 * have to find the end of a bucket.
	 */
	if (hcp->bndx == NDX_INVALID) {
		if (hcp->pagep == NULL)
			next_pgno = BUCKET_TO_PAGE(hcp, hcp->bucket);
		else
			goto got_page;

		do {
			if ((ret = __ham_next_cpage(dbc, next_pgno, 0, 0)) != 0)
				return (ret);
got_page:		next_pgno = NEXT_PGNO(hcp->pagep);
			hcp->bndx = H_NUMPAIRS(hcp->pagep);
		} while (next_pgno != PGNO_INVALID);

		if (hcp->bndx == 0) {
			/* Bucket was empty. */
			F_SET(hcp, H_NOMORE);
			return (DB_NOTFOUND);
		}
	}

	hcp->bndx--;

	return (__ham_item(dbc, mode));
}

/*
 * Sets the cursor to the next key/data pair on a page.
 *
 * PUBLIC: int __ham_item_next __P((DBC *, db_lockmode_t));
 */
int
__ham_item_next(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	HASH_CURSOR *hcp;

	hcp = (HASH_CURSOR *)dbc->internal;
	/*
	 * Deleted on-page duplicates are a weird case. If we delete the last
	 * one, then our cursor is at the very end of a duplicate set and
	 * we actually need to go on to the next key.
	 */
	if (F_ISSET(hcp, H_DELETED)) {
		if (hcp->bndx != NDX_INVALID &&
		    F_ISSET(hcp, H_ISDUP) &&
		    hcp->dpgno == PGNO_INVALID &&
		    hcp->dup_tlen == hcp->dup_off) {
			if (F_ISSET(hcp, H_DUPONLY)) {
				F_CLR(hcp, H_OK);
				F_SET(hcp, H_NOMORE);
				return (0);
			} else {
				F_CLR(hcp, H_ISDUP);
				hcp->dpgno = PGNO_INVALID;
				hcp->bndx++;
			}
		} else if (!F_ISSET(hcp, H_ISDUP) &&
		    F_ISSET(hcp, H_DUPONLY)) {
			F_CLR(hcp, H_OK);
			F_SET(hcp, H_NOMORE);
			return (0);
		}
		F_CLR(hcp, H_DELETED);
	} else if (hcp->bndx == NDX_INVALID) {
		hcp->bndx = 0;
		hcp->dpgno = PGNO_INVALID;
		F_CLR(hcp, H_ISDUP);
	} else if (F_ISSET(hcp, H_ISDUP) && hcp->dpgno != PGNO_INVALID)
		hcp->dndx++;
	else if (F_ISSET(hcp, H_ISDUP)) {
		if (hcp->dup_off + DUP_SIZE(hcp->dup_len) >=
		    hcp->dup_tlen && F_ISSET(hcp, H_DUPONLY)) {
			F_CLR(hcp, H_OK);
			F_SET(hcp, H_NOMORE);
			return (0);
		}
		hcp->dndx++;
		hcp->dup_off += DUP_SIZE(hcp->dup_len);
		if (hcp->dup_off >= hcp->dup_tlen) {
			F_CLR(hcp, H_ISDUP);
			hcp->dpgno = PGNO_INVALID;
			hcp->bndx++;
		}
	} else if (F_ISSET(hcp, H_DUPONLY)) {
		F_CLR(hcp, H_OK);
		F_SET(hcp, H_NOMORE);
		return (0);
	} else
		hcp->bndx++;

	return (__ham_item(dbc, mode));
}

/*
 * PUBLIC: void __ham_putitem __P((PAGE *p, const DBT *, int));
 *
 * This is a little bit sleazy in that we're overloading the meaning
 * of the H_OFFPAGE type here.  When we recover deletes, we have the
 * entire entry instead of having only the DBT, so we'll pass type
 * H_OFFPAGE to mean, "copy the whole entry" as opposed to constructing
 * an H_KEYDATA around it.
 */
void
__ham_putitem(p, dbt, type)
	PAGE *p;
	const DBT *dbt;
	int type;
{
	u_int16_t n, off;

	n = NUM_ENT(p);

	/* Put the item element on the page. */
	if (type == H_OFFPAGE) {
		off = HOFFSET(p) - dbt->size;
		HOFFSET(p) = p->inp[n] = off;
		memcpy(P_ENTRY(p, n), dbt->data, dbt->size);
	} else {
		off = HOFFSET(p) - HKEYDATA_SIZE(dbt->size);
		HOFFSET(p) = p->inp[n] = off;
		PUT_HKEYDATA(P_ENTRY(p, n), dbt->data, dbt->size, type);
	}

	/* Adjust page info. */
	NUM_ENT(p) += 1;
}

/*
 * PUBLIC: void __ham_reputpair
 * PUBLIC:    __P((PAGE *p, u_int32_t, u_int32_t, const DBT *, const DBT *));
 *
 * This is a special case to restore a key/data pair to its original
 * location during recovery.  We are guaranteed that the pair fits
 * on the page and is not the last pair on the page (because if it's
 * the last pair, the normal insert works).
 */
void
__ham_reputpair(p, psize, ndx, key, data)
	PAGE *p;
	u_int32_t psize, ndx;
	const DBT *key, *data;
{
	db_indx_t i, movebytes, newbytes;
	u_int8_t *from;

	/* First shuffle the existing items up on the page.  */
	movebytes =
	    (ndx == 0 ? psize : p->inp[H_DATAINDEX(ndx - 1)]) - HOFFSET(p);
	newbytes = key->size + data->size;
	from = (u_int8_t *)p + HOFFSET(p);
	memmove(from - newbytes, from, movebytes);

	/*
	 * Adjust the indices and move them up 2 spaces. Note that we
	 * have to check the exit condition inside the loop just in case
	 * we are dealing with index 0 (db_indx_t's are unsigned).
	 */
	for (i = NUM_ENT(p) - 1; ; i-- ) {
		p->inp[i + 2] = p->inp[i] - newbytes;
		if (i == H_KEYINDEX(ndx))
			break;
	}

	/* Put the key and data on the page. */
	p->inp[H_KEYINDEX(ndx)] =
	    (ndx == 0 ? psize : p->inp[H_DATAINDEX(ndx - 1)]) - key->size;
	p->inp[H_DATAINDEX(ndx)] = p->inp[H_KEYINDEX(ndx)] - data->size;
	memcpy(P_ENTRY(p, H_KEYINDEX(ndx)), key->data, key->size);
	memcpy(P_ENTRY(p, H_DATAINDEX(ndx)), data->data, data->size);

	/* Adjust page info. */
	HOFFSET(p) -= newbytes;
	NUM_ENT(p) += 2;
}


/*
 * PUBLIC: int __ham_del_pair __P((DBC *, int));
 */
int
__ham_del_pair(dbc, reclaim_page)
	DBC *dbc;
	int reclaim_page;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DBT data_dbt, key_dbt;
	DB_ENV *dbenv;
	DB_LSN new_lsn, *n_lsn, tmp_lsn;
	PAGE *p;
	db_indx_t ndx;
	db_pgno_t chg_pgno, pgno;
	int ret, tret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;

	dbenv = dbp->dbenv;
	ndx = hcp->bndx;
	if (hcp->pagep == NULL &&
	    (ret = __ham_get_page(dbp, hcp->pgno, &hcp->pagep)) != 0)
		return (ret);

	p = hcp->pagep;

	/*
	 * We optimize for the normal case which is when neither the key nor
	 * the data are large.  In this case, we write a single log record
	 * and do the delete.  If either is large, we'll call __big_delete
	 * to remove the big item and then update the page to remove the
	 * entry referring to the big item.
	 */
	ret = 0;
	if (HPAGE_PTYPE(H_PAIRKEY(p, ndx)) == H_OFFPAGE) {
		memcpy(&pgno, HOFFPAGE_PGNO(P_ENTRY(p, H_KEYINDEX(ndx))),
		    sizeof(db_pgno_t));
		ret = __db_doff(dbc, pgno, __ham_del_page);
	}

	if (ret == 0)
		switch (HPAGE_PTYPE(H_PAIRDATA(p, ndx))) {
		case H_OFFPAGE:
			memcpy(&pgno,
			    HOFFPAGE_PGNO(P_ENTRY(p, H_DATAINDEX(ndx))),
			    sizeof(db_pgno_t));
			ret = __db_doff(dbc, pgno, __ham_del_page);
			break;
		case H_OFFDUP:
			memcpy(&pgno,
			    HOFFDUP_PGNO(P_ENTRY(p, H_DATAINDEX(ndx))),
			    sizeof(db_pgno_t));
			ret = __db_ddup(dbc, pgno, __ham_del_page);
			F_CLR(hcp, H_ISDUP);
			break;
		case H_DUPLICATE:
			/*
			 * If we delete a pair that is/was a duplicate, then
			 * we had better clear the flag so that we update the
			 * cursor appropriately.
			 */
			F_CLR(hcp, H_ISDUP);
			break;
		}

	if (ret)
		return (ret);

	/* Now log the delete off this page. */
	if (DB_LOGGING(dbc)) {
		key_dbt.data = P_ENTRY(p, H_KEYINDEX(ndx));
		key_dbt.size =
		    LEN_HITEM(p, hcp->hdr->pagesize, H_KEYINDEX(ndx));
		data_dbt.data = P_ENTRY(p, H_DATAINDEX(ndx));
		data_dbt.size =
		    LEN_HITEM(p, hcp->hdr->pagesize, H_DATAINDEX(ndx));

		if ((ret = __ham_insdel_log(dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, DELPAIR,
		    dbp->log_fileid, PGNO(p), (u_int32_t)ndx,
		    &LSN(p), &key_dbt, &data_dbt)) != 0)
			return (ret);

		/* Move lsn onto page. */
		LSN(p) = new_lsn;
	}

	__ham_dpair(dbp, p, ndx);

	/*
	 * If we are locking, we will not maintain this, because it is
	 * a hot spot.
	 * XXX perhaps we can retain incremental numbers and apply them
	 * later.
	 */
	if (!F_ISSET(dbp, DB_AM_LOCKING))
		--hcp->hdr->nelem;

	/*
	 * If we need to reclaim the page, then check if the page is empty.
	 * There are two cases.  If it's empty and it's not the first page
	 * in the bucket (i.e., the bucket page) then we can simply remove
	 * it. If it is the first chain in the bucket, then we need to copy
	 * the second page into it and remove the second page.
	 */
	if (reclaim_page && NUM_ENT(p) == 0 && PREV_PGNO(p) == PGNO_INVALID &&
	    NEXT_PGNO(p) != PGNO_INVALID) {
		PAGE *n_pagep, *nn_pagep;
		db_pgno_t tmp_pgno;

		/*
		 * First page in chain is empty and we know that there
		 * are more pages in the chain.
		 */
		if ((ret =
		    __ham_get_page(dbp, NEXT_PGNO(p), &n_pagep)) != 0)
			return (ret);

		if (NEXT_PGNO(n_pagep) != PGNO_INVALID) {
			if ((ret =
			    __ham_get_page(dbp, NEXT_PGNO(n_pagep),
			    &nn_pagep)) != 0) {
				(void) __ham_put_page(dbp, n_pagep, 0);
				return (ret);
			}
		}

		if (DB_LOGGING(dbc)) {
			key_dbt.data = n_pagep;
			key_dbt.size = hcp->hdr->pagesize;
			if ((ret = __ham_copypage_log(dbenv->lg_info,
			    dbc->txn, &new_lsn, 0, dbp->log_fileid, PGNO(p),
			    &LSN(p), PGNO(n_pagep), &LSN(n_pagep),
			    NEXT_PGNO(n_pagep),
			    NEXT_PGNO(n_pagep) == PGNO_INVALID ? NULL :
			    &LSN(nn_pagep), &key_dbt)) != 0)
				return (ret);

			/* Move lsn onto page. */
			LSN(p) = new_lsn;	/* Structure assignment. */
			LSN(n_pagep) = new_lsn;
			if (NEXT_PGNO(n_pagep) != PGNO_INVALID)
				LSN(nn_pagep) = new_lsn;
		}
		if (NEXT_PGNO(n_pagep) != PGNO_INVALID) {
			PREV_PGNO(nn_pagep) = PGNO(p);
			(void)__ham_put_page(dbp, nn_pagep, 1);
		}

		tmp_pgno = PGNO(p);
		tmp_lsn = LSN(p);
		memcpy(p, n_pagep, hcp->hdr->pagesize);
		PGNO(p) = tmp_pgno;
		LSN(p) = tmp_lsn;
		PREV_PGNO(p) = PGNO_INVALID;

		/*
		 * Cursor is advanced to the beginning of the next page.
		 */
		hcp->bndx = 0;
		hcp->pgno = PGNO(p);
		F_SET(hcp, H_DELETED);
		chg_pgno = PGNO(p);
		if ((ret = __ham_dirty_page(dbp, p)) != 0 ||
		    (ret = __ham_del_page(dbc, n_pagep)) != 0)
			return (ret);
	} else if (reclaim_page &&
	    NUM_ENT(p) == 0 && PREV_PGNO(p) != PGNO_INVALID) {
		PAGE *n_pagep, *p_pagep;

		if ((ret =
		    __ham_get_page(dbp, PREV_PGNO(p), &p_pagep)) != 0)
			return (ret);

		if (NEXT_PGNO(p) != PGNO_INVALID) {
			if ((ret = __ham_get_page(dbp,
			    NEXT_PGNO(p), &n_pagep)) != 0) {
				(void)__ham_put_page(dbp, p_pagep, 0);
				return (ret);
			}
			n_lsn = &LSN(n_pagep);
		} else {
			n_pagep = NULL;
			n_lsn = NULL;
		}

		NEXT_PGNO(p_pagep) = NEXT_PGNO(p);
		if (n_pagep != NULL)
			PREV_PGNO(n_pagep) = PGNO(p_pagep);

		if (DB_LOGGING(dbc)) {
			if ((ret = __ham_newpage_log(dbenv->lg_info,
			    dbc->txn, &new_lsn, 0, DELOVFL,
			    dbp->log_fileid, PREV_PGNO(p), &LSN(p_pagep),
			    PGNO(p), &LSN(p), NEXT_PGNO(p), n_lsn)) != 0)
				return (ret);

			/* Move lsn onto page. */
			LSN(p_pagep) = new_lsn;	/* Structure assignment. */
			if (n_pagep)
				LSN(n_pagep) = new_lsn;
			LSN(p) = new_lsn;
		}
		hcp->pgno = NEXT_PGNO(p);
		hcp->bndx = 0;
		/*
		 * Since we are about to delete the cursor page and we have
		 * just moved the cursor, we need to make sure that the
		 * old page pointer isn't left hanging around in the cursor.
		 */
		hcp->pagep = NULL;
		chg_pgno = PGNO(p);
		ret = __ham_del_page(dbc, p);
		if ((tret = __ham_put_page(dbp, p_pagep, 1)) != 0 &&
		    ret == 0)
			ret = tret;
		if (n_pagep != NULL &&
		    (tret = __ham_put_page(dbp, n_pagep, 1)) != 0 &&
		    ret == 0)
			ret = tret;
		if (ret != 0)
			return (ret);
	} else {
		/*
		 * Mark item deleted so that we don't try to return it, and
		 * so that we update the cursor correctly on the next call
		 * to next.
		 */
		F_SET(hcp, H_DELETED);
		chg_pgno = hcp->pgno;
		ret = __ham_dirty_page(dbp, p);
	}
	__ham_c_update(hcp, chg_pgno, 0, 0, 0);

	/*
	 * Since we just deleted a pair from the master page, anything
	 * in hcp->dpgno should be cleared.
	 */
	hcp->dpgno = PGNO_INVALID;

	F_CLR(hcp, H_OK);
	return (ret);
}

/*
 * __ham_replpair --
 *	Given the key data indicated by the cursor, replace part/all of it
 *	according to the fields in the dbt.
 *
 * PUBLIC: int __ham_replpair __P((DBC *, DBT *, u_int32_t));
 */
int
__ham_replpair(dbc, dbt, make_dup)
	DBC *dbc;
	DBT *dbt;
	u_int32_t make_dup;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DBT old_dbt, tdata, tmp;
	DB_LSN	new_lsn;
	int32_t change;			/* XXX: Possible overflow. */
	u_int32_t len;
	int is_big, ret, type;
	u_int8_t *beg, *dest, *end, *hk, *src;

	/*
	 * Big item replacements are handled in generic code.
	 * Items that fit on the current page fall into 4 classes.
	 * 1. On-page element, same size
	 * 2. On-page element, new is bigger (fits)
	 * 3. On-page element, new is bigger (does not fit)
	 * 4. On-page element, old is bigger
	 * Numbers 1, 2, and 4 are essentially the same (and should
	 * be the common case).  We handle case 3 as a delete and
	 * add.
	 */
	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;

	/*
	 * We need to compute the number of bytes that we are adding or
	 * removing from the entry.  Normally, we can simply substract
	 * the number of bytes we are replacing (dbt->dlen) from the
	 * number of bytes we are inserting (dbt->size).  However, if
	 * we are doing a partial put off the end of a record, then this
	 * formula doesn't work, because we are essentially adding
	 * new bytes.
	 */
	change = dbt->size - dbt->dlen;

	hk = H_PAIRDATA(hcp->pagep, hcp->bndx);
	is_big = HPAGE_PTYPE(hk) == H_OFFPAGE;

	if (is_big)
		memcpy(&len, HOFFPAGE_TLEN(hk), sizeof(u_int32_t));
	else
		len = LEN_HKEYDATA(hcp->pagep,
		    dbp->pgsize, H_DATAINDEX(hcp->bndx));

	if (dbt->doff + dbt->dlen > len)
		change += dbt->doff + dbt->dlen - len;


	if (change > (int32_t)P_FREESPACE(hcp->pagep) || is_big) {
		/*
		 * Case 3 -- two subcases.
		 * A. This is not really a partial operation, but an overwrite.
		 *    Simple del and add works.
		 * B. This is a partial and we need to construct the data that
		 *    we are really inserting (yuck).
		 * In both cases, we need to grab the key off the page (in
		 * some cases we could do this outside of this routine; for
		 * cleanliness we do it here.  If you happen to be on a big
		 * key, this could be a performance hit).
		 */
		tmp.flags = 0;
		F_SET(&tmp, DB_DBT_MALLOC | DB_DBT_INTERNAL);
		if ((ret =
		    __db_ret(dbp, hcp->pagep, H_KEYINDEX(hcp->bndx),
		    &tmp, &dbc->rkey.data, &dbc->rkey.size)) != 0)
			return (ret);

		if (dbt->doff == 0 && dbt->dlen == len) {
			ret = __ham_del_pair(dbc, 0);
			if (ret == 0)
			    ret = __ham_add_el(dbc, &tmp, dbt, H_KEYDATA);
		} else {					/* Case B */
			type = HPAGE_PTYPE(hk) != H_OFFPAGE ?
			    HPAGE_PTYPE(hk) : H_KEYDATA;
			tdata.flags = 0;
			F_SET(&tdata, DB_DBT_MALLOC | DB_DBT_INTERNAL);

			if ((ret = __db_ret(dbp, hcp->pagep,
			    H_DATAINDEX(hcp->bndx), &tdata, &dbc->rdata.data,
			    &dbc->rdata.size)) != 0)
				goto err;

			/* Now we can delete the item. */
			if ((ret = __ham_del_pair(dbc, 0)) != 0) {
				__os_free(tdata.data, tdata.size);
				goto err;
			}

			/* Now shift old data around to make room for new. */
			if (change > 0) {
				 if ((ret = __os_realloc(&tdata.data,
				     tdata.size + change)) != 0)
					return (ret);
				memset((u_int8_t *)tdata.data + tdata.size,
				    0, change);
			}
			end = (u_int8_t *)tdata.data + tdata.size;

			src = (u_int8_t *)tdata.data + dbt->doff + dbt->dlen;
			if (src < end && tdata.size > dbt->doff + dbt->dlen) {
				len = tdata.size - dbt->doff - dbt->dlen;
				dest = src + change;
				memmove(dest, src, len);
			}
			memcpy((u_int8_t *)tdata.data + dbt->doff,
			    dbt->data, dbt->size);
			tdata.size += change;

			/* Now add the pair. */
			ret = __ham_add_el(dbc, &tmp, &tdata, type);
			__os_free(tdata.data, tdata.size);
		}
err:		__os_free(tmp.data, tmp.size);
		return (ret);
	}

	/*
	 * Set up pointer into existing data. Do it before the log
	 * message so we can use it inside of the log setup.
	 */
	beg = HKEYDATA_DATA(H_PAIRDATA(hcp->pagep, hcp->bndx));
	beg += dbt->doff;

	/*
	 * If we are going to have to move bytes at all, figure out
	 * all the parameters here.  Then log the call before moving
	 * anything around.
	 */
	if (DB_LOGGING(dbc)) {
		old_dbt.data = beg;
		old_dbt.size = dbt->dlen;
		if ((ret = __ham_replace_log(dbp->dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, dbp->log_fileid, PGNO(hcp->pagep),
		    (u_int32_t)H_DATAINDEX(hcp->bndx), &LSN(hcp->pagep),
		    (u_int32_t)dbt->doff, &old_dbt, dbt, make_dup)) != 0)
			return (ret);

		LSN(hcp->pagep) = new_lsn;	/* Structure assignment. */
	}

	__ham_onpage_replace(hcp->pagep, dbp->pgsize,
	    (u_int32_t)H_DATAINDEX(hcp->bndx), (int32_t)dbt->doff, change, dbt);

	return (0);
}

/*
 * Replace data on a page with new data, possibly growing or shrinking what's
 * there.  This is called on two different occasions. On one (from replpair)
 * we are interested in changing only the data.  On the other (from recovery)
 * we are replacing the entire data (header and all) with a new element.  In
 * the latter case, the off argument is negative.
 * pagep: the page that we're changing
 * ndx: page index of the element that is growing/shrinking.
 * off: Offset at which we are beginning the replacement.
 * change: the number of bytes (+ or -) that the element is growing/shrinking.
 * dbt: the new data that gets written at beg.
 * PUBLIC: void __ham_onpage_replace __P((PAGE *, size_t, u_int32_t, int32_t,
 * PUBLIC:     int32_t,  DBT *));
 */
void
__ham_onpage_replace(pagep, pgsize, ndx, off, change, dbt)
	PAGE *pagep;
	size_t pgsize;
	u_int32_t ndx;
	int32_t off;
	int32_t change;
	DBT *dbt;
{
	db_indx_t i;
	int32_t len;
	u_int8_t *src, *dest;
	int zero_me;

	if (change != 0) {
		zero_me = 0;
		src = (u_int8_t *)(pagep) + HOFFSET(pagep);
		if (off < 0)
			len = pagep->inp[ndx] - HOFFSET(pagep);
		else if ((u_int32_t)off >= LEN_HKEYDATA(pagep, pgsize, ndx)) {
			len = HKEYDATA_DATA(P_ENTRY(pagep, ndx)) +
			    LEN_HKEYDATA(pagep, pgsize, ndx) - src;
			zero_me = 1;
		} else
			len = (HKEYDATA_DATA(P_ENTRY(pagep, ndx)) + off) - src;
		dest = src - change;
		memmove(dest, src, len);
		if (zero_me)
			memset(dest + len, 0, change);

		/* Now update the indices. */
		for (i = ndx; i < NUM_ENT(pagep); i++)
			pagep->inp[i] -= change;
		HOFFSET(pagep) -= change;
	}
	if (off >= 0)
		memcpy(HKEYDATA_DATA(P_ENTRY(pagep, ndx)) + off,
		    dbt->data, dbt->size);
	else
		memcpy(P_ENTRY(pagep, ndx), dbt->data, dbt->size);
}

/*
 * PUBLIC: int __ham_split_page __P((DBC *, u_int32_t, u_int32_t));
 */
int
__ham_split_page(dbc, obucket, nbucket)
	DBC *dbc;
	u_int32_t obucket, nbucket;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DBT key, page_dbt;
	DB_ENV *dbenv;
	DB_LSN new_lsn;
	PAGE **pp, *old_pagep, *temp_pagep, *new_pagep;
	db_indx_t n;
	db_pgno_t bucket_pgno, next_pgno;
	u_int32_t big_len, len;
	int ret, tret;
	void *big_buf;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	dbenv = dbp->dbenv;
	temp_pagep = old_pagep = new_pagep = NULL;

	bucket_pgno = BUCKET_TO_PAGE(hcp, obucket);
	if ((ret = __ham_get_page(dbp, bucket_pgno, &old_pagep)) != 0)
		return (ret);
	if ((ret = __ham_new_page(dbp, BUCKET_TO_PAGE(hcp, nbucket), P_HASH,
	    &new_pagep)) != 0)
		goto err;

	temp_pagep = hcp->split_buf;
	memcpy(temp_pagep, old_pagep, hcp->hdr->pagesize);

	if (DB_LOGGING(dbc)) {
		page_dbt.size = hcp->hdr->pagesize;
		page_dbt.data = old_pagep;
		if ((ret = __ham_splitdata_log(dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, dbp->log_fileid, SPLITOLD,
		    PGNO(old_pagep), &page_dbt, &LSN(old_pagep))) != 0)
			goto err;
	}

	P_INIT(old_pagep, hcp->hdr->pagesize, PGNO(old_pagep), PGNO_INVALID,
	    PGNO_INVALID, 0, P_HASH);

	if (DB_LOGGING(dbc))
		LSN(old_pagep) = new_lsn;	/* Structure assignment. */

	big_len = 0;
	big_buf = NULL;
	key.flags = 0;
	while (temp_pagep != NULL) {
		for (n = 0; n < (db_indx_t)H_NUMPAIRS(temp_pagep); n++) {
			if ((ret =
			    __db_ret(dbp, temp_pagep, H_KEYINDEX(n),
			    &key, &big_buf, &big_len)) != 0)
				goto err;

			if (__ham_call_hash(hcp, key.data, key.size)
			    == obucket)
				pp = &old_pagep;
			else
				pp = &new_pagep;

			/*
			 * Figure out how many bytes we need on the new
			 * page to store the key/data pair.
			 */

			len = LEN_HITEM(temp_pagep, hcp->hdr->pagesize,
			    H_DATAINDEX(n)) +
			    LEN_HITEM(temp_pagep, hcp->hdr->pagesize,
			    H_KEYINDEX(n)) +
			    2 * sizeof(db_indx_t);

			if (P_FREESPACE(*pp) < len) {
				if (DB_LOGGING(dbc)) {
					page_dbt.size = hcp->hdr->pagesize;
					page_dbt.data = *pp;
					if ((ret = __ham_splitdata_log(
					    dbenv->lg_info, dbc->txn,
					    &new_lsn, 0, dbp->log_fileid,
					    SPLITNEW, PGNO(*pp), &page_dbt,
					    &LSN(*pp))) != 0)
						goto err;
					LSN(*pp) = new_lsn;
				}
				if ((ret =
				    __ham_add_ovflpage(dbc, *pp, 1, pp)) != 0)
					goto err;
			}
			__ham_copy_item(dbp->pgsize,
			    temp_pagep, H_KEYINDEX(n), *pp);
			__ham_copy_item(dbp->pgsize,
			    temp_pagep, H_DATAINDEX(n), *pp);
		}
		next_pgno = NEXT_PGNO(temp_pagep);

		/* Clear temp_page; if it's a link overflow page, free it. */
		if (PGNO(temp_pagep) != bucket_pgno && (ret =
		    __ham_del_page(dbc, temp_pagep)) != 0)
			goto err;

		if (next_pgno == PGNO_INVALID)
			temp_pagep = NULL;
		else if ((ret =
		    __ham_get_page(dbp, next_pgno, &temp_pagep)) != 0)
			goto err;

		if (temp_pagep != NULL && DB_LOGGING(dbc)) {
			page_dbt.size = hcp->hdr->pagesize;
			page_dbt.data = temp_pagep;
			if ((ret = __ham_splitdata_log(dbenv->lg_info,
			    dbc->txn, &new_lsn, 0, dbp->log_fileid,
			    SPLITOLD, PGNO(temp_pagep),
			    &page_dbt, &LSN(temp_pagep))) != 0)
				goto err;
			LSN(temp_pagep) = new_lsn;
		}
	}
	if (big_buf != NULL)
		__os_free(big_buf, big_len);

	/*
	 * If the original bucket spanned multiple pages, then we've got
	 * a pointer to a page that used to be on the bucket chain.  It
	 * should be deleted.
	 */
	if (temp_pagep != NULL && PGNO(temp_pagep) != bucket_pgno &&
	    (ret = __ham_del_page(dbc, temp_pagep)) != 0)
		goto err;

	/*
	 * Write new buckets out.
	 */
	if (DB_LOGGING(dbc)) {
		page_dbt.size = hcp->hdr->pagesize;
		page_dbt.data = old_pagep;
		if ((ret = __ham_splitdata_log(dbenv->lg_info,
		   dbc->txn, &new_lsn, 0, dbp->log_fileid,
		   SPLITNEW, PGNO(old_pagep),
		    &page_dbt, &LSN(old_pagep))) != 0)
			goto err;
		LSN(old_pagep) = new_lsn;

		page_dbt.data = new_pagep;
		if ((ret = __ham_splitdata_log(dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, dbp->log_fileid,
		    SPLITNEW, PGNO(new_pagep), &page_dbt, &LSN(new_pagep))) != 0)
			goto err;
		LSN(new_pagep) = new_lsn;
	}
	ret = __ham_put_page(dbp, old_pagep, 1);
	if ((tret = __ham_put_page(dbp, new_pagep, 1)) != 0 &&
	    ret == 0)
		ret = tret;

	if (0) {
err:		if (old_pagep != NULL)
			(void)__ham_put_page(dbp, old_pagep, 1);
		if (new_pagep != NULL)
			(void)__ham_put_page(dbp, new_pagep, 1);
		if (temp_pagep != NULL && PGNO(temp_pagep) != bucket_pgno)
			(void)__ham_put_page(dbp, temp_pagep, 1);
	}
	return (ret);
}

/*
 * Add the given pair to the page.  The page in question may already be
 * held (i.e. it was already gotten).  If it is, then the page is passed
 * in via the pagep parameter.  On return, pagep will contain the page
 * to which we just added something.  This allows us to link overflow
 * pages and return the new page having correctly put the last page.
 *
 * PUBLIC: int __ham_add_el __P((DBC *, const DBT *, const DBT *, int));
 */
int
__ham_add_el(dbc, key, val, type)
	DBC *dbc;
	const DBT *key, *val;
	int type;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	const DBT *pkey, *pdata;
	DBT key_dbt, data_dbt;
	DB_LSN new_lsn;
	HOFFPAGE doff, koff;
	db_pgno_t next_pgno;
	u_int32_t data_size, key_size, pairsize, rectype;
	int do_expand, is_keybig, is_databig, ret;
	int key_type, data_type;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	do_expand = 0;

	if (hcp->pagep == NULL && (ret = __ham_get_page(dbp,
	    hcp->seek_found_page != PGNO_INVALID ?  hcp->seek_found_page :
	    hcp->pgno, &hcp->pagep)) != 0)
		return (ret);

	key_size = HKEYDATA_PSIZE(key->size);
	data_size = HKEYDATA_PSIZE(val->size);
	is_keybig = ISBIG(hcp, key->size);
	is_databig = ISBIG(hcp, val->size);
	if (is_keybig)
		key_size = HOFFPAGE_PSIZE;
	if (is_databig)
		data_size = HOFFPAGE_PSIZE;

	pairsize = key_size + data_size;

	/* Advance to first page in chain with room for item. */
	while (H_NUMPAIRS(hcp->pagep) && NEXT_PGNO(hcp->pagep) !=
	    PGNO_INVALID) {
		/*
		 * This may not be the end of the chain, but the pair may fit
		 * anyway.  Check if it's a bigpair that fits or a regular
		 * pair that fits.
		 */
		if (P_FREESPACE(hcp->pagep) >= pairsize)
			break;
		next_pgno = NEXT_PGNO(hcp->pagep);
		if ((ret =
		    __ham_next_cpage(dbc, next_pgno, 0, 0)) != 0)
			return (ret);
	}

	/*
	 * Check if we need to allocate a new page.
	 */
	if (P_FREESPACE(hcp->pagep) < pairsize) {
		do_expand = 1;
		if ((ret = __ham_add_ovflpage(dbc,
		    hcp->pagep, 1, &hcp->pagep)) !=  0)
			return (ret);
		hcp->pgno = PGNO(hcp->pagep);
	}

	/*
	 * Update cursor.
	 */
	hcp->bndx = H_NUMPAIRS(hcp->pagep);
	F_CLR(hcp, H_DELETED);
	if (is_keybig) {
		koff.type = H_OFFPAGE;
		UMRW(koff.unused[0]);
		UMRW(koff.unused[1]);
		UMRW(koff.unused[2]);
		if ((ret = __db_poff(dbc,
		    key, &koff.pgno, __ham_overflow_page)) != 0)
			return (ret);
		koff.tlen = key->size;
		key_dbt.data = &koff;
		key_dbt.size = sizeof(koff);
		pkey = &key_dbt;
		key_type = H_OFFPAGE;
	} else {
		pkey = key;
		key_type = H_KEYDATA;
	}

	if (is_databig) {
		doff.type = H_OFFPAGE;
		UMRW(doff.unused[0]);
		UMRW(doff.unused[1]);
		UMRW(doff.unused[2]);
		if ((ret = __db_poff(dbc,
		    val, &doff.pgno, __ham_overflow_page)) != 0)
			return (ret);
		doff.tlen = val->size;
		data_dbt.data = &doff;
		data_dbt.size = sizeof(doff);
		pdata = &data_dbt;
		data_type = H_OFFPAGE;
	} else {
		pdata = val;
		data_type = type;
	}

	if (DB_LOGGING(dbc)) {
		rectype = PUTPAIR;
		if (is_databig)
			rectype |= PAIR_DATAMASK;
		if (is_keybig)
			rectype |= PAIR_KEYMASK;

		if ((ret = __ham_insdel_log(dbp->dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, rectype,
		    dbp->log_fileid, PGNO(hcp->pagep),
		    (u_int32_t)H_NUMPAIRS(hcp->pagep),
		    &LSN(hcp->pagep), pkey, pdata)) != 0)
			return (ret);

		/* Move lsn onto page. */
		LSN(hcp->pagep) = new_lsn;	/* Structure assignment. */
	}

	__ham_putitem(hcp->pagep, pkey, key_type);
	__ham_putitem(hcp->pagep, pdata, data_type);

	/*
	 * For splits, we are going to update item_info's page number
	 * field, so that we can easily return to the same page the
	 * next time we come in here.  For other operations, this shouldn't
	 * matter, since odds are this is the last thing that happens before
	 * we return to the user program.
	 */
	hcp->pgno = PGNO(hcp->pagep);

	/*
	 * XXX Maybe keep incremental numbers here
	 */
	if (!F_ISSET(dbp, DB_AM_LOCKING))
		hcp->hdr->nelem++;

	if (do_expand || (hcp->hdr->ffactor != 0 &&
	    (u_int32_t)H_NUMPAIRS(hcp->pagep) > hcp->hdr->ffactor))
		F_SET(hcp, H_EXPAND);
	return (0);
}


/*
 * Special __putitem call used in splitting -- copies one entry to
 * another.  Works for all types of hash entries (H_OFFPAGE, H_KEYDATA,
 * H_DUPLICATE, H_OFFDUP).  Since we log splits at a high level, we
 * do not need to do any logging here.
 *
 * PUBLIC: void __ham_copy_item __P((size_t, PAGE *, u_int32_t, PAGE *));
 */
void
__ham_copy_item(pgsize, src_page, src_ndx, dest_page)
	size_t pgsize;
	PAGE *src_page;
	u_int32_t src_ndx;
	PAGE *dest_page;
{
	u_int32_t len;
	void *src, *dest;

	/*
	 * Copy the key and data entries onto this new page.
	 */
	src = P_ENTRY(src_page, src_ndx);

	/* Set up space on dest. */
	len = LEN_HITEM(src_page, pgsize, src_ndx);
	HOFFSET(dest_page) -= len;
	dest_page->inp[NUM_ENT(dest_page)] = HOFFSET(dest_page);
	dest = P_ENTRY(dest_page, NUM_ENT(dest_page));
	NUM_ENT(dest_page)++;

	memcpy(dest, src, len);
}

/*
 *
 * Returns:
 *      pointer on success
 *      NULL on error
 *
 * PUBLIC: int __ham_add_ovflpage __P((DBC *, PAGE *, int, PAGE **));
 */
int
__ham_add_ovflpage(dbc, pagep, release, pp)
	DBC *dbc;
	PAGE *pagep;
	int release;
	PAGE **pp;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DB_LSN new_lsn;
	PAGE *new_pagep;
	int ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;

	if ((ret = __ham_overflow_page(dbc, P_HASH, &new_pagep)) != 0)
		return (ret);

	if (DB_LOGGING(dbc)) {
		if ((ret = __ham_newpage_log(dbp->dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, PUTOVFL,
		    dbp->log_fileid, PGNO(pagep), &LSN(pagep),
		    PGNO(new_pagep), &LSN(new_pagep), PGNO_INVALID, NULL)) != 0)
			return (ret);

		/* Move lsn onto page. */
		LSN(pagep) = LSN(new_pagep) = new_lsn;
	}
	NEXT_PGNO(pagep) = PGNO(new_pagep);
	PREV_PGNO(new_pagep) = PGNO(pagep);

	if (release)
		ret = __ham_put_page(dbp, pagep, 1);

	hcp->stats.hash_overflows++;
	*pp = new_pagep;
	return (ret);
}


/*
 * PUBLIC: int __ham_new_page __P((DB *, u_int32_t, u_int32_t, PAGE **));
 */
int
__ham_new_page(dbp, addr, type, pp)
	DB *dbp;
	u_int32_t addr, type;
	PAGE **pp;
{
	PAGE *pagep;
	int ret;

	if ((ret = memp_fget(dbp->mpf,
	    &addr, DB_MPOOL_CREATE, &pagep)) != 0)
		return (ret);

	/* This should not be necessary because page-in should do it. */
	P_INIT(pagep, dbp->pgsize, addr, PGNO_INVALID, PGNO_INVALID, 0, type);

	*pp = pagep;
	return (0);
}

/*
 * PUBLIC: int __ham_del_page __P((DBC *, PAGE *));
 */
int
__ham_del_page(dbc, pagep)
	DBC *dbc;
	PAGE *pagep;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DB_LSN new_lsn;
	int ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	ret = 0;
	DIRTY_META(dbp, hcp, ret);
	if (ret != 0) {
		if (ret != EAGAIN)
			__db_err(dbp->dbenv,
			    "free_ovflpage: unable to lock meta data page %s\n",
			    strerror(ret));
		/*
		 * If we are going to return an error, then we should free
		 * the page, so it doesn't stay pinned forever.
		 */
		(void)__ham_put_page(dbp, pagep, 0);
		return (ret);
	}

	if (DB_LOGGING(dbc)) {
		if ((ret = __ham_newpgno_log(dbp->dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, DELPGNO,
		    dbp->log_fileid, PGNO(pagep), hcp->hdr->last_freed,
		    (u_int32_t)TYPE(pagep), NEXT_PGNO(pagep), P_INVALID,
		    &LSN(pagep), &hcp->hdr->lsn)) != 0)
			return (ret);

		hcp->hdr->lsn = new_lsn;
		LSN(pagep) = new_lsn;
	}

#ifdef DIAGNOSTIC
	{
		db_pgno_t __pgno;
		DB_LSN __lsn;
		__pgno = pagep->pgno;
		__lsn = pagep->lsn;
		memset(pagep, 0xdb, dbp->pgsize);
		pagep->pgno = __pgno;
		pagep->lsn = __lsn;
	}
#endif
	TYPE(pagep) = P_INVALID;
	NEXT_PGNO(pagep) = hcp->hdr->last_freed;
	hcp->hdr->last_freed = PGNO(pagep);

	return (__ham_put_page(dbp, pagep, 1));
}


/*
 * PUBLIC: int __ham_put_page __P((DB *, PAGE *, int32_t));
 */
int
__ham_put_page(dbp, pagep, is_dirty)
	DB *dbp;
	PAGE *pagep;
	int32_t is_dirty;
{
#ifdef DEBUG_SLOW
	__account_page(dbp, ((BKT *)((char *)pagep - sizeof(BKT)))->pgno, -1);
#endif
	return (memp_fput(dbp->mpf, pagep, (is_dirty ? DB_MPOOL_DIRTY : 0)));
}

/*
 * __ham_dirty_page --
 *	Mark a page dirty.
 *
 * PUBLIC: int __ham_dirty_page __P((DB *, PAGE *));
 */
int
__ham_dirty_page(dbp, pagep)
	DB *dbp;
	PAGE *pagep;
{
	return (memp_fset(dbp->mpf, pagep, DB_MPOOL_DIRTY));
}

/*
 * PUBLIC: int __ham_get_page __P((DB *, db_pgno_t, PAGE **));
 */
int
__ham_get_page(dbp, addr, pagep)
	DB *dbp;
	db_pgno_t addr;
	PAGE **pagep;
{
	int ret;

	ret = memp_fget(dbp->mpf, &addr, DB_MPOOL_CREATE, pagep);
#ifdef DEBUG_SLOW
	if (*pagep != NULL)
		__account_page(dbp, addr, 1);
#endif
	return (ret);
}

/*
 * PUBLIC: int __ham_overflow_page
 * PUBLIC:     __P((DBC *, u_int32_t, PAGE **));
 */
int
__ham_overflow_page(dbc, type, pp)
	DBC *dbc;
	u_int32_t type;
	PAGE **pp;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DB_LSN *lsnp, new_lsn;
	PAGE *p;
	db_pgno_t new_addr, next_free, newalloc_flag;
	u_int32_t offset, splitnum;
	int ret;

	ret = 0;
	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	DIRTY_META(dbp, hcp, ret);
	if (ret != 0)
		return (ret);

	/*
	 * This routine is split up into two parts.  First we have
	 * to figure out the address of the new page that we are
	 * allocating.  Then we have to log the allocation.  Only
	 * after the log do we get to complete allocation of the
	 * new page.
	 */
	new_addr = hcp->hdr->last_freed;
	if (new_addr != PGNO_INVALID) {
		if ((ret = __ham_get_page(dbp, new_addr, &p)) != 0)
			return (ret);
		next_free = NEXT_PGNO(p);
		lsnp = &LSN(p);
		newalloc_flag = 0;
	} else {
		splitnum = hcp->hdr->ovfl_point;
		hcp->hdr->spares[splitnum]++;
		offset = hcp->hdr->spares[splitnum] -
		    (splitnum ? hcp->hdr->spares[splitnum - 1] : 0);
		new_addr = PGNO_OF(hcp, hcp->hdr->ovfl_point, offset);
		if (new_addr > MAX_PAGES(hcp)) {
			__db_err(dbp->dbenv, "hash: out of file pages");
			hcp->hdr->spares[splitnum]--;
			return (ENOMEM);
		}
		next_free = PGNO_INVALID;
		p = NULL;
		lsnp = NULL;
		newalloc_flag = 1;
	}

	if (DB_LOGGING(dbc)) {
		if ((ret = __ham_newpgno_log(dbp->dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, ALLOCPGNO,
		    dbp->log_fileid, new_addr, next_free,
		    0, newalloc_flag, type, lsnp, &hcp->hdr->lsn)) != 0)
			return (ret);

		hcp->hdr->lsn = new_lsn;
		if (lsnp != NULL)
			*lsnp = new_lsn;
	}

	if (p != NULL) {
		/* We just took something off the free list, initialize it. */
		hcp->hdr->last_freed = next_free;
		P_INIT(p, hcp->hdr->pagesize, PGNO(p), PGNO_INVALID,
		    PGNO_INVALID, 0, (u_int8_t)type);
	} else {
		/* Get the new page. */
		if ((ret = __ham_new_page(dbp, new_addr, type, &p)) != 0)
			return (ret);
	}
	if (DB_LOGGING(dbc))
		LSN(p) = new_lsn;

	*pp = p;
	return (0);
}

#ifdef DEBUG
/*
 * PUBLIC: #ifdef DEBUG
 * PUBLIC: db_pgno_t __bucket_to_page __P((HASH_CURSOR *, db_pgno_t));
 * PUBLIC: #endif
 */
db_pgno_t
__bucket_to_page(hcp, n)
	HASH_CURSOR *hcp;
	db_pgno_t n;
{
	int ret_val;

	ret_val = n + 1;
	if (n != 0)
		ret_val += hcp->hdr->spares[__db_log2(n + 1) - 1];
	return (ret_val);
}
#endif

/*
 * Create a bunch of overflow pages at the current split point.
 * PUBLIC: void __ham_init_ovflpages __P((DBC *));
 */
void
__ham_init_ovflpages(dbc)
	DBC *dbc;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	DB_LSN new_lsn;
	PAGE *p;
	db_pgno_t last_pgno, new_pgno;
	u_int32_t i, curpages, numpages;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;

	curpages = hcp->hdr->spares[hcp->hdr->ovfl_point] -
	    hcp->hdr->spares[hcp->hdr->ovfl_point - 1];
	numpages = hcp->hdr->ovfl_point + 1 - curpages;

	last_pgno = hcp->hdr->last_freed;
	new_pgno = PGNO_OF(hcp, hcp->hdr->ovfl_point, curpages + 1);
	if (DB_LOGGING(dbc)) {
		(void)__ham_ovfl_log(dbp->dbenv->lg_info,
		    dbc->txn, &new_lsn, 0, dbp->log_fileid, new_pgno,
		    numpages, last_pgno, hcp->hdr->ovfl_point, &hcp->hdr->lsn);
		hcp->hdr->lsn = new_lsn;
	} else
		ZERO_LSN(new_lsn);

	hcp->hdr->spares[hcp->hdr->ovfl_point] += numpages;
	for (i = numpages; i > 0; i--) {
		if (__ham_new_page(dbp,
		    PGNO_OF(hcp, hcp->hdr->ovfl_point, curpages + i),
		    P_INVALID, &p) != 0)
			break;
		LSN(p) = new_lsn;
		NEXT_PGNO(p) = last_pgno;
		last_pgno = PGNO(p);
		(void)__ham_put_page(dbp, p, 1);
	}
	hcp->hdr->last_freed = last_pgno;
}

/*
 * PUBLIC: int __ham_get_cpage __P((DBC *, db_lockmode_t));
 */
int
__ham_get_cpage(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	int ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;

	/*
	 * There are three cases with respect to buckets and locks.  If there
	 * is no lock held, then if we are locking, we should get the lock.
	 * If there is a lock held and it's for the current bucket, we don't
	 * need to do anything.  If there is a lock, but it's for a different
	 * bucket, then we need to release and get.
	 */
	if (F_ISSET(dbp, DB_AM_LOCKING)) {
		if (hcp->lock != 0 && hcp->lbucket != hcp->bucket) {
			/*
			 * If this is the original lock, don't release it,
			 * because we may need to restore it upon exit.
			 */
			if (dbc->txn == NULL &&
			    !F_ISSET(hcp, H_ORIGINAL) && (ret =
			    lock_put(dbp->dbenv->lk_info, hcp->lock)) != 0)
				return (ret);
			F_CLR(hcp, H_ORIGINAL);
			hcp->lock = 0;
		}
		if (hcp->lock == 0 && (ret = __ham_lock_bucket(dbc, mode)) != 0)
			return (ret);
		hcp->lbucket = hcp->bucket;
	}

	if (hcp->pagep == NULL) {
		if (hcp->pgno == PGNO_INVALID) {
			hcp->pgno = BUCKET_TO_PAGE(hcp, hcp->bucket);
			hcp->bndx = 0;
		}

		if ((ret =
		    __ham_get_page(dbp, hcp->pgno, &hcp->pagep)) != 0)
			return (ret);
	}

	if (hcp->dpgno != PGNO_INVALID && hcp->dpagep == NULL)
		if ((ret =
		    __ham_get_page(dbp, hcp->dpgno, &hcp->dpagep)) != 0)
			return (ret);
	return (0);
}

/*
 * Get a new page at the cursor, putting the last page if necessary.
 * If the flag is set to H_ISDUP, then we are talking about the
 * duplicate page, not the main page.
 *
 * PUBLIC: int __ham_next_cpage __P((DBC *, db_pgno_t, int, u_int32_t));
 */
int
__ham_next_cpage(dbc, pgno, dirty, flags)
	DBC *dbc;
	db_pgno_t pgno;
	int dirty;
	u_int32_t flags;
{
	DB *dbp;
	HASH_CURSOR *hcp;
	PAGE *p;
	int ret;

	dbp = dbc->dbp;
	hcp = (HASH_CURSOR *)dbc->internal;
	if (LF_ISSET(H_ISDUP) && hcp->dpagep != NULL &&
	    (ret = __ham_put_page(dbp, hcp->dpagep, dirty)) != 0)
		return (ret);
	else if (!LF_ISSET(H_ISDUP) && hcp->pagep != NULL &&
	    (ret = __ham_put_page(dbp, hcp->pagep, dirty)) != 0)
		return (ret);

	if ((ret = __ham_get_page(dbp, pgno, &p)) != 0)
		return (ret);

	if (LF_ISSET(H_ISDUP)) {
		hcp->dpagep = p;
		hcp->dpgno = pgno;
		hcp->dndx = 0;
	} else {
		hcp->pagep = p;
		hcp->pgno = pgno;
		hcp->bndx = 0;
	}

	return (0);
}

/*
 * __ham_lock_bucket --
 *	Get the lock on a particular bucket.
 */
static int
__ham_lock_bucket(dbc, mode)
	DBC *dbc;
	db_lockmode_t mode;
{
	HASH_CURSOR *hcp;
	int ret;

	hcp = (HASH_CURSOR *)dbc->internal;
	dbc->lock.pgno = (db_pgno_t)(hcp->bucket);
	if (dbc->txn == NULL)
		ret = lock_get(dbc->dbp->dbenv->lk_info, dbc->locker, 0,
		    &dbc->lock_dbt, mode, &hcp->lock);
	else
		ret = lock_tget(dbc->dbp->dbenv->lk_info, dbc->txn, 0,
		    &dbc->lock_dbt, mode, &hcp->lock);

	return (ret < 0 ? EAGAIN : ret);
}

/*
 * __ham_dpair --
 *	Delete a pair on a page, paying no attention to what the pair
 *	represents.  The caller is responsible for freeing up duplicates
 *	or offpage entries that might be referenced by this pair.
 *
 * PUBLIC: void __ham_dpair __P((DB *, PAGE *, u_int32_t));
 */
void
__ham_dpair(dbp, p, pndx)
	DB *dbp;
	PAGE *p;
	u_int32_t pndx;
{
	db_indx_t delta, n;
	u_int8_t *dest, *src;

	/*
	 * Compute "delta", the amount we have to shift all of the
	 * offsets.  To find the delta, we just need to calculate
	 * the size of the pair of elements we are removing.
	 */
	delta = H_PAIRSIZE(p, dbp->pgsize, pndx);

	/*
	 * The hard case: we want to remove something other than
	 * the last item on the page.  We need to shift data and
	 * offsets down.
	 */
	if ((db_indx_t)pndx != H_NUMPAIRS(p) - 1) {
		/*
		 * Move the data: src is the first occupied byte on
		 * the page. (Length is delta.)
		 */
		src = (u_int8_t *)p + HOFFSET(p);

		/*
		 * Destination is delta bytes beyond src.  This might
		 * be an overlapping copy, so we have to use memmove.
		 */
		dest = src + delta;
		memmove(dest, src, p->inp[H_DATAINDEX(pndx)] - HOFFSET(p));
	}

	/* Adjust the offsets. */
	for (n = (db_indx_t)pndx; n < (db_indx_t)(H_NUMPAIRS(p) - 1); n++) {
		p->inp[H_KEYINDEX(n)] = p->inp[H_KEYINDEX(n+1)] + delta;
		p->inp[H_DATAINDEX(n)] = p->inp[H_DATAINDEX(n+1)] + delta;
	}

	/* Adjust page metadata. */
	HOFFSET(p) = HOFFSET(p) + delta;
	NUM_ENT(p) = NUM_ENT(p) - 2;
}
