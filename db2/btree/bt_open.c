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
static const char sccsid[] = "@(#)bt_open.c	10.39 (Sleepycat) 11/21/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"

/*
 * __bam_open --
 *	Open a btree.
 *
 * PUBLIC: int __bam_open __P((DB *, DB_INFO *));
 */
int
__bam_open(dbp, dbinfo)
	DB *dbp;
	DB_INFO *dbinfo;
{
	BTREE *t;
	int ret;

	/* Allocate and initialize the private btree structure. */
	if ((ret = __os_calloc(1, sizeof(BTREE), &t)) != 0)
		return (ret);
	dbp->internal = t;

	/*
	 * Intention is to make sure all of the user's selections are okay
	 * here and then use them without checking.
	 */
	if (dbinfo == NULL) {
		t->bt_minkey = DEFMINKEYPAGE;
		t->bt_compare = __bam_defcmp;
		t->bt_prefix = __bam_defpfx;
	} else {
		/* Minimum number of keys per page. */
		if (dbinfo->bt_minkey == 0)
			t->bt_minkey = DEFMINKEYPAGE;
		else {
			if (dbinfo->bt_minkey < 2)
				goto einval;
			t->bt_minkey = dbinfo->bt_minkey;
		}

		/* Maximum number of keys per page. */
		if (dbinfo->bt_maxkey == 0)
			t->bt_maxkey = 0;
		else {
			if (dbinfo->bt_maxkey < 1)
				goto einval;
			t->bt_maxkey = dbinfo->bt_maxkey;
		}

		/*
		 * If no comparison, use default comparison.  If no comparison
		 * and no prefix, use default prefix.  (We can't default the
		 * prefix if the user supplies a comparison routine; shortening
		 * the keys may break their comparison algorithm.  We don't
		 * permit the user to specify a prefix routine if they didn't
		 * also specify a comparison routine, they can't know enough
		 * about our comparison routine to get it right.)
		 */
		if ((t->bt_compare = dbinfo->bt_compare) == NULL) {
			if (dbinfo->bt_prefix != NULL)
				goto einval;
			t->bt_compare = __bam_defcmp;
			t->bt_prefix = __bam_defpfx;
		} else
			t->bt_prefix = dbinfo->bt_prefix;
	}

	/* Initialize the remaining fields/methods of the DB. */
	dbp->am_close = __bam_close;
	dbp->del = __bam_delete;
	dbp->stat = __bam_stat;

	/* Start up the tree. */
	if ((ret = __bam_read_root(dbp)) != 0)
		goto err;

	/* Set the overflow page size. */
	__bam_setovflsize(dbp);

	return (0);

einval:	ret = EINVAL;

err:	__os_free(t, sizeof(BTREE));
	return (ret);
}

/*
 * __bam_close --
 *	Close a btree.
 *
 * PUBLIC: int __bam_close __P((DB *));
 */
int
__bam_close(dbp)
	DB *dbp;
{
	__os_free(dbp->internal, sizeof(BTREE));
	dbp->internal = NULL;

	return (0);
}

/*
 * __bam_setovflsize --
 *
 * PUBLIC: void __bam_setovflsize __P((DB *));
 */
void
__bam_setovflsize(dbp)
	DB *dbp;
{
	BTREE *t;

	t = dbp->internal;

	/*
	 * !!!
	 * Correction for recno, which doesn't know anything about minimum
	 * keys per page.
	 */
	if (t->bt_minkey == 0)
		t->bt_minkey = DEFMINKEYPAGE;

	/*
	 * The btree data structure requires that at least two key/data pairs
	 * can fit on a page, but other than that there's no fixed requirement.
	 * Translate the minimum number of items into the bytes a key/data pair
	 * can use before being placed on an overflow page.  We calculate for
	 * the worst possible alignment by assuming every item requires the
	 * maximum alignment for padding.
	 *
	 * Recno uses the btree bt_ovflsize value -- it's close enough.
	 */
	t->bt_ovflsize = (dbp->pgsize - P_OVERHEAD) / (t->bt_minkey * P_INDX)
	    - (BKEYDATA_PSIZE(0) + ALIGN(1, 4));
}

/*
 * __bam_read_root --
 *	Check (and optionally create) a tree.
 *
 * PUBLIC: int __bam_read_root __P((DB *));
 */
int
__bam_read_root(dbp)
	DB *dbp;
{
	BTMETA *meta;
	BTREE *t;
	DBC *dbc;
	DB_LOCK metalock, rootlock;
	PAGE *root;
	db_pgno_t pgno;
	int ret, t_ret;

	ret = 0;
	t = dbp->internal;

	/* Get a cursor. */
	if ((ret = dbp->cursor(dbp, NULL, &dbc, 0)) != 0)
		return (ret);

	/* Get, and optionally create the metadata page. */
	pgno = PGNO_METADATA;
	if ((ret =
	    __bam_lget(dbc, 0, PGNO_METADATA, DB_LOCK_WRITE, &metalock)) != 0)
		goto err;
	if ((ret =
	    memp_fget(dbp->mpf, &pgno, DB_MPOOL_CREATE, (PAGE **)&meta)) != 0) {
		(void)__BT_LPUT(dbc, metalock);
		goto err;
	}

	/*
	 * If the magic number is correct, we're not creating the tree.
	 * Correct any fields that may not be right.  Note, all of the
	 * local flags were set by db_open(3).
	 */
	if (meta->magic != 0) {
		t->bt_maxkey = meta->maxkey;
		t->bt_minkey = meta->minkey;

		(void)memp_fput(dbp->mpf, (PAGE *)meta, 0);
		(void)__BT_LPUT(dbc, metalock);
		goto done;
	}

	/* Initialize the tree structure metadata information. */
	memset(meta, 0, sizeof(BTMETA));
	ZERO_LSN(meta->lsn);
	meta->pgno = PGNO_METADATA;
	meta->magic = DB_BTREEMAGIC;
	meta->version = DB_BTREEVERSION;
	meta->pagesize = dbp->pgsize;
	meta->maxkey = t->bt_maxkey;
	meta->minkey = t->bt_minkey;
	meta->free = PGNO_INVALID;
	if (dbp->type == DB_RECNO)
		F_SET(meta, BTM_RECNO);
	if (F_ISSET(dbp, DB_AM_DUP))
		F_SET(meta, BTM_DUP);
	if (F_ISSET(dbp, DB_RE_FIXEDLEN))
		F_SET(meta, BTM_FIXEDLEN);
	if (F_ISSET(dbp, DB_BT_RECNUM))
		F_SET(meta, BTM_RECNUM);
	if (F_ISSET(dbp, DB_RE_RENUMBER))
		F_SET(meta, BTM_RENUMBER);
	memcpy(meta->uid, dbp->fileid, DB_FILE_ID_LEN);

	/* Create and initialize a root page. */
	pgno = PGNO_ROOT;
	if ((ret =
	    __bam_lget(dbc, 0, PGNO_ROOT, DB_LOCK_WRITE, &rootlock)) != 0)
		goto err;
	if ((ret = memp_fget(dbp->mpf, &pgno, DB_MPOOL_CREATE, &root)) != 0) {
		(void)__BT_LPUT(dbc, rootlock);
		goto err;
	}
	P_INIT(root, dbp->pgsize, PGNO_ROOT, PGNO_INVALID,
	    PGNO_INVALID, 1, dbp->type == DB_RECNO ? P_LRECNO : P_LBTREE);
	ZERO_LSN(root->lsn);

	/* Release the metadata and root pages. */
	if ((ret = memp_fput(dbp->mpf, (PAGE *)meta, DB_MPOOL_DIRTY)) != 0)
		goto err;
	if ((ret = memp_fput(dbp->mpf, root, DB_MPOOL_DIRTY)) != 0)
		goto err;

	/*
	 * Flush the metadata and root pages to disk -- since the user can't
	 * transaction protect open, the pages have to exist during recovery.
	 *
	 * XXX
	 * It's not useful to return not-yet-flushed here -- convert it to
	 * an error.
	 */
	if ((ret = memp_fsync(dbp->mpf)) == DB_INCOMPLETE)
		ret = EINVAL;

	/* Release the locks. */
	(void)__BT_LPUT(dbc, metalock);
	(void)__BT_LPUT(dbc, rootlock);

err:
done:	if ((t_ret = dbc->c_close(dbc)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}
