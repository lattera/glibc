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
 *
 *	@(#)btree.h	10.26 (Sleepycat) 12/16/98
 */

/* Forward structure declarations. */
struct __btree;		typedef struct __btree BTREE;
struct __cursor;	typedef struct __cursor CURSOR;
struct __epg;		typedef struct __epg EPG;
struct __recno;		typedef struct __recno RECNO;

#define	DEFMINKEYPAGE	 (2)

#define	ISINTERNAL(p)	(TYPE(p) == P_IBTREE || TYPE(p) == P_IRECNO)
#define	ISLEAF(p)	(TYPE(p) == P_LBTREE || TYPE(p) == P_LRECNO)

/*
 * If doing transactions we have to hold the locks associated with a data item
 * from a page for the entire transaction.  However, we don't have to hold the
 * locks associated with walking the tree.  Distinguish between the two so that
 * we don't tie up the internal pages of the tree longer than necessary.
 */
#define	__BT_LPUT(dbc, lock)						\
	(F_ISSET((dbc)->dbp, DB_AM_LOCKING) ?				\
	    lock_put((dbc)->dbp->dbenv->lk_info, lock) : 0)
#define	__BT_TLPUT(dbc, lock)						\
	(F_ISSET((dbc)->dbp, DB_AM_LOCKING) && (dbc)->txn == NULL ?	\
	    lock_put((dbc)->dbp->dbenv->lk_info, lock) : 0)

/*
 * Flags to __bam_search() and __bam_rsearch().
 *
 * Note, internal page searches must find the largest record less than key in
 * the tree so that descents work.  Leaf page searches must find the smallest
 * record greater than key so that the returned index is the record's correct
 * position for insertion.
 *
 * The flags parameter to the search routines describes three aspects of the
 * search: the type of locking required (including if we're locking a pair of
 * pages), the item to return in the presence of duplicates and whether or not
 * to return deleted entries.  To simplify both the mnemonic representation
 * and the code that checks for various cases, we construct a set of bitmasks.
 */
#define	S_READ		0x00001		/* Read locks. */
#define	S_WRITE		0x00002		/* Write locks. */

#define	S_APPEND	0x00040		/* Append to the tree. */
#define	S_DELNO		0x00080		/* Don't return deleted items. */
#define	S_DUPFIRST	0x00100		/* Return first duplicate. */
#define	S_DUPLAST	0x00200		/* Return last duplicate. */
#define	S_EXACT		0x00400		/* Exact items only. */
#define	S_PARENT	0x00800		/* Lock page pair. */
#define	S_STACK		0x01000		/* Need a complete stack. */
#define	S_PAST_EOF	0x02000		/* If doing insert search (or keyfirst
					 * or keylast operations), or a split
					 * on behalf of an insert, it's okay to
					 * return an entry one past end-of-page.
					 */

#define	S_DELETE	(S_WRITE | S_DUPFIRST | S_DELNO | S_EXACT | S_STACK)
#define	S_FIND		(S_READ | S_DUPFIRST | S_DELNO)
#define	S_FIND_WR	(S_WRITE | S_DUPFIRST | S_DELNO)
#define	S_INSERT	(S_WRITE | S_DUPLAST | S_PAST_EOF | S_STACK)
#define	S_KEYFIRST	(S_WRITE | S_DUPFIRST | S_PAST_EOF | S_STACK)
#define	S_KEYLAST	(S_WRITE | S_DUPLAST | S_PAST_EOF | S_STACK)
#define	S_WRPAIR	(S_WRITE | S_DUPLAST | S_PAST_EOF | S_PARENT)

/*
 * Flags to __bam_iitem().
 */
#define	BI_DELETED	0x01		/* Key/data pair only placeholder. */
#define	BI_DOINCR	0x02		/* Increment the record count. */
#define	BI_NEWKEY	0x04		/* New key. */

/*
 * Various routines pass around page references.  A page reference can be a
 * pointer to the page or a page number; for either, an indx can designate
 * an item on the page.
 */
struct __epg {
	PAGE	 *page;			/* The page. */
	db_indx_t indx;			/* The index on the page. */
	DB_LOCK	  lock;			/* The page's lock. */
};

/*
 * We maintain a stack of the pages that we're locking in the tree.  Btree's
 * (currently) only save two levels of the tree at a time, so the default
 * stack is always large enough.  Recno trees have to lock the entire tree to
 * do inserts/deletes, however.  Grow the stack as necessary.
 */
#define	BT_STK_CLR(c)							\
	((c)->csp = (c)->sp)

#define	BT_STK_ENTER(c, pagep, page_indx, lock, ret) do {		\
	if ((ret =							\
	    (c)->csp == (c)->esp ? __bam_stkgrow(c) : 0) == 0) {	\
		(c)->csp->page = pagep;					\
		(c)->csp->indx = page_indx;				\
		(c)->csp->lock = lock;					\
	}								\
} while (0)

#define	BT_STK_PUSH(c, pagep, page_indx, lock, ret) do {		\
	BT_STK_ENTER(c, pagep, page_indx, lock, ret);			\
	++(c)->csp;							\
} while (0)

#define	BT_STK_POP(c)							\
	((c)->csp == (c)->stack ? NULL : --(c)->csp)

/*
 * Arguments passed to __bam_ca_replace().
 */
typedef enum {
	REPLACE_SETUP,
	REPLACE_SUCCESS,
	REPLACE_FAILED
} ca_replace_arg;

/* Arguments passed to __ram_ca(). */
typedef enum {
	CA_DELETE,
	CA_IAFTER,
	CA_IBEFORE
} ca_recno_arg;

#define	RECNO_OOB	0		/* Illegal record number. */

/* Btree/Recno cursor. */
struct __cursor {
	DBC		*dbc;		/* Enclosing DBC. */

	/* Per-thread information: shared by btree/recno. */
	EPG		*sp;		/* Stack pointer. */
	EPG	 	*csp;		/* Current stack entry. */
	EPG		*esp;		/* End stack pointer. */
	EPG		 stack[5];

	/* Per-thread information: btree private. */
	PAGE		*page;		/* Cursor page. */

	db_pgno_t	 pgno;		/* Page. */
	db_indx_t	 indx;		/* Page item ref'd by the cursor. */

	db_pgno_t	 dpgno;		/* Duplicate page. */
	db_indx_t	 dindx;		/* Page item ref'd by the cursor. */

	DB_LOCK		 lock;		/* Cursor read lock. */
	db_lockmode_t	 mode;		/* Lock mode. */

	/* Per-thread information: recno private. */
	db_recno_t	 recno;		/* Current record number. */

	/*
	 * Btree:
	 * We set a flag in the cursor structure if the underlying object has
	 * been deleted.  It's not strictly necessary, we could get the same
	 * information by looking at the page itself.
	 *
	 * Recno:
	 * When renumbering recno databases during deletes, cursors referencing
	 * "deleted" records end up positioned between two records, and so must
	 * be specially adjusted on the next operation.
	 */
#define	C_DELETED	0x0001		/* Record was deleted. */
	u_int32_t	 flags;
};

/*
 * The in-memory recno data structure.
 *
 * !!!
 * These fields are ignored as far as multi-threading is concerned.  There
 * are no transaction semantics associated with backing files, nor is there
 * any thread protection.
 */
struct __recno {
	int		 re_delim;	/* Variable-length delimiting byte. */
	int		 re_pad;	/* Fixed-length padding byte. */
	u_int32_t	 re_len;	/* Length for fixed-length records. */

	char		*re_source;	/* Source file name. */
	int		 re_fd;		/* Source file descriptor */
	db_recno_t	 re_last;	/* Last record number read. */
	void		*re_cmap;	/* Current point in mapped space. */
	void		*re_smap;	/* Start of mapped space. */
	void		*re_emap;	/* End of mapped space. */
	size_t		 re_msize;	/* Size of mapped region. */
					/* Recno input function. */
	int (*re_irec) __P((DBC *, db_recno_t));

#define	RECNO_EOF	0x0001		/* EOF on backing source file. */
#define	RECNO_MODIFIED	0x0002		/* Tree was modified. */
	u_int32_t	 flags;
};

/*
 * The in-memory, per-tree btree data structure.
 */
struct __btree {
	db_pgno_t	 bt_lpgno;	/* Last insert location. */

	db_indx_t 	 bt_maxkey;	/* Maximum keys per page. */
	db_indx_t 	 bt_minkey;	/* Minimum keys per page. */

	int (*bt_compare)		/* Comparison function. */
	    __P((const DBT *, const DBT *));
	size_t(*bt_prefix)		/* Prefix function. */
	    __P((const DBT *, const DBT *));

	db_indx_t	 bt_ovflsize;	/* Maximum key/data on-page size. */

	RECNO		*recno;		/* Private recno structure. */
};

#include "btree_auto.h"
#include "btree_ext.h"
#include "db_am.h"
#include "common_ext.h"
