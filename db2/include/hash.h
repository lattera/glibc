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
 *
 *	@(#)hash.h	10.14 (Sleepycat) 10/4/98
 */

/* Cursor structure definitions. */
typedef struct cursor_t {
	DBC		*dbc;

	/* Per-thread information */
	DB_LOCK hlock;			/* Metadata page lock. */
	HASHHDR *hdr;			/* Pointer to meta-data page. */
	PAGE *split_buf;		/* Temporary buffer for splits. */
	struct __db_h_stat stats;	/* Hash statistics. */

	/* Hash cursor information */
	db_pgno_t	bucket;		/* Bucket we are traversing. */
	db_pgno_t	lbucket;	/* Bucket for which we are locked. */
	DB_LOCK		lock;		/* Lock held on the current bucket. */
	PAGE		*pagep;		/* The current page. */
	db_pgno_t	pgno;		/* Current page number. */
	db_indx_t	bndx;		/* Index within the current page. */
	PAGE		*dpagep;	/* Duplicate page pointer. */
	db_pgno_t	dpgno;		/* Duplicate page number. */
	db_indx_t	dndx;		/* Index within a duplicate set. */
	db_indx_t	dup_off;	/* Offset within a duplicate set. */
	db_indx_t	dup_len;	/* Length of current duplicate. */
	db_indx_t	dup_tlen;	/* Total length of duplicate entry. */
	u_int32_t	seek_size;	/* Number of bytes we need for add. */
	db_pgno_t	seek_found_page;/* Page on which we can insert. */

#define	H_DELETED	0x0001		/* Cursor item is deleted. */
#define	H_DUPONLY	0x0002		/* Dups only; do not change key. */
#define	H_EXPAND	0x0004		/* Table expanded. */
#define	H_ISDUP		0x0008		/* Cursor is within duplicate set. */
#define	H_NOMORE	0x0010		/* No more entries in bucket. */
#define	H_OK		0x0020		/* Request succeeded. */
#define H_DIRTY		0x0040		/* Meta-data page needs to be written */
#define	H_ORIGINAL	0x0080		/* Bucket lock existed on entry. */
	u_int32_t	flags;
} HASH_CURSOR;

#define	IS_VALID(C) ((C)->bucket != BUCKET_INVALID)

#define	SAVE_CURSOR(ORIG, COPY) {					\
	F_SET((ORIG), H_ORIGINAL);					\
	*(COPY) = *(ORIG);						\
}

#define	RESTORE_CURSOR(D, ORIG, COPY, RET) {				\
	if ((RET) == 0) {						\
		if ((ORIG)->dbc->txn == NULL &&				\
		    (COPY)->lock != 0 && (ORIG)->lock != (COPY)->lock)	\
			(void)lock_put((D)->dbenv->lk_info, (COPY)->lock); \
	} else {							\
		if ((ORIG)->dbc->txn == NULL &&				\
		    (ORIG)->lock != 0 && (ORIG)->lock != (COPY)->lock)	\
			(void)lock_put((D)->dbenv->lk_info, (ORIG)->lock); \
		*ORIG = *COPY;						\
	}								\
}

/*
 * More interface macros used to get/release the meta data page.
 */
#define	GET_META(D, I, R) {						\
	if (F_ISSET(D, DB_AM_LOCKING) &&				\
	    !F_ISSET((I)->dbc, DBC_RECOVER)) {				\
		(I)->dbc->lock.pgno = BUCKET_INVALID;			\
		(R) = lock_get((D)->dbenv->lk_info, (I)->dbc->locker, 	\
		    0, &(I)->dbc->lock_dbt, DB_LOCK_READ, &(I)->hlock);	\
		(R) = (R) < 0 ? EAGAIN : (R);				\
	}								\
	if ((R) == 0 && 						\
	    ((R) = __ham_get_page(D, 0, (PAGE **)&((I)->hdr))) != 0 &&  \
	    (I)->hlock != LOCK_INVALID) {				\
		(void)lock_put((D)->dbenv->lk_info, (I)->hlock);	\
		(I)->hlock = LOCK_INVALID;				\
	}								\
}

#define	RELEASE_META(D, I) {						\
	if ((I)->hdr)							\
		(void)__ham_put_page(D, (PAGE *)(I)->hdr,		\
		    F_ISSET(I, H_DIRTY) ? 1 : 0);			\
	(I)->hdr = NULL;						\
	if (!F_ISSET((I)->dbc, DBC_RECOVER) &&				\
	    (I)->dbc->txn == NULL && (I)->hlock)			\
		(void)lock_put((D)->dbenv->lk_info, (I)->hlock);	\
	(I)->hlock = LOCK_INVALID;					\
	F_CLR(I, H_DIRTY);						\
}

#define	DIRTY_META(D, I, R) {						\
	if (F_ISSET(D, DB_AM_LOCKING) &&				\
	    !F_ISSET((I)->dbc, DBC_RECOVER)) {				\
		DB_LOCK _tmp;						\
		(I)->dbc->lock.pgno = BUCKET_INVALID;			\
	    	if (((R) = lock_get((D)->dbenv->lk_info,		\
	    	    (I)->dbc->locker, 0, &(I)->dbc->lock_dbt,		\
	    	    DB_LOCK_WRITE, &_tmp)) == 0)			\
			(R) = lock_put((D)->dbenv->lk_info, (I)->hlock);\
		else if ((R) < 0)					\
			(R) = EAGAIN;					\
		(I)->hlock = _tmp;					\
	}								\
	F_SET((I), H_DIRTY);						\
}

/* Test string. */
#define	CHARKEY			"%$sniglet^&"

/* Overflow management */
/*
 * Overflow page numbers are allocated per split point.  At each doubling of
 * the table, we can allocate extra pages.  We keep track of how many pages
 * we've allocated at each point to calculate bucket to page number mapping.
 */
#define	BUCKET_TO_PAGE(I, B) \
	((B) + 1 + ((B) ? (I)->hdr->spares[__db_log2((B)+1)-1] : 0))

#define	PGNO_OF(I, S, O) (BUCKET_TO_PAGE((I), (1 << (S)) - 1) + (O))

/* Constraints about number of pages and how much data goes on a page. */

#define	MAX_PAGES(H)	UINT32_T_MAX
#define	MINFILL		4
#define	ISBIG(I, N)	(((N) > ((I)->hdr->pagesize / MINFILL)) ? 1 : 0)

/* Shorthands for accessing structure */
#define	NDX_INVALID	0xFFFF
#define	BUCKET_INVALID	0xFFFFFFFF

/* On page duplicates are stored as a string of size-data-size triples. */
#define	DUP_SIZE(len)	((len) + 2 * sizeof(db_indx_t))

/* Log messages types (these are subtypes within a record type) */
#define	PAIR_KEYMASK		0x1
#define	PAIR_DATAMASK		0x2
#define	PAIR_ISKEYBIG(N)	(N & PAIR_KEYMASK)
#define	PAIR_ISDATABIG(N)	(N & PAIR_DATAMASK)
#define	OPCODE_OF(N)    	(N & ~(PAIR_KEYMASK | PAIR_DATAMASK))

#define	PUTPAIR		0x20
#define	DELPAIR		0x30
#define	PUTOVFL		0x40
#define	DELOVFL		0x50
#define	ALLOCPGNO	0x60
#define	DELPGNO		0x70
#define	SPLITOLD	0x80
#define	SPLITNEW	0x90

#include "hash_auto.h"
#include "hash_ext.h"
#include "db_am.h"
#include "common_ext.h"
