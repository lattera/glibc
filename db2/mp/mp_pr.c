/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_pr.c	10.12 (Sleepycat) 7/29/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"

void __memp_debug __P((DB_MPOOL *, FILE *, int));

static void __memp_pbh __P((FILE *, DB_MPOOL *, BH *, int));
static void __memp_pdbmf __P((FILE *, DB_MPOOLFILE *, int));
static void __memp_pmf __P((FILE *, MPOOLFILE *, int));
static void __memp_pmp __P((FILE *, DB_MPOOL *, MPOOL *, int));

/*
 * memp_stat --
 *	Display MPOOL statistics.
 */
int
memp_stat(dbmp, gspp, fspp, db_malloc)
	DB_MPOOL *dbmp;
	DB_MPOOL_STAT **gspp;
	DB_MPOOL_FSTAT ***fspp;
	void *(*db_malloc) __P((size_t));
{
	DB_MPOOL_FSTAT **tfsp;
	MPOOLFILE *mfp;
	size_t len, nlen;
	char *name;

	/* Allocate space for the global statistics. */
	if (gspp != NULL) {
		*gspp = NULL;

		if ((*gspp = db_malloc == NULL ?
		    (DB_MPOOL_STAT *)malloc(sizeof(**gspp)) :
		    (DB_MPOOL_STAT *)db_malloc(sizeof(**gspp))) == NULL)
			return (ENOMEM);

		LOCKREGION(dbmp);

		/* Copy out the global statistics. */
		**gspp = dbmp->mp->stat;
		(*gspp)->st_hash_buckets = dbmp->mp->htab_buckets;

		UNLOCKREGION(dbmp);
	}

	if (fspp != NULL) {
		*fspp = NULL;

		LOCKREGION(dbmp);

		/* Count the MPOOLFILE structures. */
		for (len = 0,
		    mfp = SH_TAILQ_FIRST(&dbmp->mp->mpfq, __mpoolfile);
		    mfp != NULL;
		    ++len, mfp = SH_TAILQ_NEXT(mfp, q, __mpoolfile));

		UNLOCKREGION(dbmp);

		if (len == 0)
			return (0);

		/* Allocate space for the pointers. */
		len = (len + 1) * sizeof(DB_MPOOL_FSTAT *);
		if ((*fspp = db_malloc == NULL ?
		    (DB_MPOOL_FSTAT **)malloc(len) :
		    (DB_MPOOL_FSTAT **)db_malloc(len)) == NULL)
			return (ENOMEM);

		LOCKREGION(dbmp);

		/* Build each individual entry. */
		for (tfsp = *fspp,
		    mfp = SH_TAILQ_FIRST(&dbmp->mp->mpfq, __mpoolfile);
		    mfp != NULL;
		    ++tfsp, mfp = SH_TAILQ_NEXT(mfp, q, __mpoolfile)) {
			name = ADDR(dbmp, mfp->path_off);
			nlen = strlen(name);
			len = sizeof(DB_MPOOL_FSTAT) + nlen + 1;
			if ((*tfsp = db_malloc == NULL ?
			    (DB_MPOOL_FSTAT *)malloc(len) :
			    (DB_MPOOL_FSTAT *)db_malloc(len)) == NULL)
				return (ENOMEM);
			**tfsp = mfp->stat;
			(*tfsp)->file_name = (char *)
			    (u_int8_t *)*tfsp + sizeof(DB_MPOOL_FSTAT);
			memcpy((*tfsp)->file_name, name, nlen + 1);
		}
		*tfsp = NULL;

		UNLOCKREGION(dbmp);
	}
	return (0);
}

/*
 * __memp_debug --
 *	Display MPOOL structures.
 *
 * PUBLIC: void __memp_debug __P((DB_MPOOL *, FILE *, int));
 */
void
__memp_debug(dbmp, fp, data)
	DB_MPOOL *dbmp;
	FILE *fp;
	int data;
{
	DB_MPOOLFILE *dbmfp;
	u_long cnt;

	/* Make it easy to call from the debugger. */
	if (fp == NULL)
		fp = stderr;

	/* Welcome message. */
	(void)fprintf(fp, "%s\nMpool per-process (%lu) statistics\n",
	    DB_LINE, (u_long)getpid());

	if (data)
		(void)fprintf(fp, "    fd: %d; addr %lx; maddr %lx\n",
		    dbmp->fd, (u_long)dbmp->addr, (u_long)dbmp->maddr);

	/* Display the DB_MPOOLFILE structures. */
	for (cnt = 0, dbmfp = TAILQ_FIRST(&dbmp->dbmfq);
	    dbmfp != NULL; ++cnt, dbmfp = TAILQ_NEXT(dbmfp, q));
	(void)fprintf(fp, "%lu process-local files\n", cnt);
	for (dbmfp = TAILQ_FIRST(&dbmp->dbmfq);
	    dbmfp != NULL; dbmfp = TAILQ_NEXT(dbmfp, q)) {
		(void)fprintf(fp, "%s\n", dbmfp->path);
		__memp_pdbmf(fp, dbmfp, data);
	}

	/* Switch to global statistics. */
	(void)fprintf(fp, "\n%s\nMpool statistics\n", DB_LINE);

	/* Display the MPOOL structure. */
	__memp_pmp(fp, dbmp, dbmp->mp, data);

	/* Flush in case we're debugging. */
	(void)fflush(fp);
}

/*
 * __memp_pdbmf --
 *	Display a DB_MPOOLFILE structure.
 */
static void
__memp_pdbmf(fp, dbmfp, data)
	FILE *fp;
	DB_MPOOLFILE *dbmfp;
	int data;
{
	if (!data)
		return;

	(void)fprintf(fp, "    fd: %d; %s\n",
	    dbmfp->fd, F_ISSET(dbmfp, MP_READONLY) ? "readonly" : "read/write");
}

/*
 * __memp_pmp --
 *	Display the MPOOL structure.
 */
static void
__memp_pmp(fp, dbmp, mp, data)
	FILE *fp;
	DB_MPOOL *dbmp;
	MPOOL *mp;
	int data;
{
	BH *bhp;
	MPOOLFILE *mfp;
	DB_HASHTAB *htabp;
	size_t bucket;
	int cnt;
	const char *sep;

	(void)fprintf(fp, "references: %lu; cachesize: %lu\n",
	    (u_long)mp->rlayout.refcnt, (u_long)mp->stat.st_cachesize);
	(void)fprintf(fp,
	    "    %lu pages created\n", mp->stat.st_page_create);
	(void)fprintf(fp,
	    "    %lu mmap pages returned\n", mp->stat.st_map);
	(void)fprintf(fp, "    %lu I/O's (%lu read, %lu written)\n",
	    mp->stat.st_page_in + mp->stat.st_page_out,
	    mp->stat.st_page_in, mp->stat.st_page_out);
	if (mp->stat.st_cache_hit + mp->stat.st_cache_miss != 0)
		(void)fprintf(fp,
		    "    %.0f%% cache hit rate (%lu hit, %lu miss)\n",
		    ((double)mp->stat.st_cache_hit /
	    (mp->stat.st_cache_hit + mp->stat.st_cache_miss)) * 100,
		    mp->stat.st_cache_hit, mp->stat.st_cache_miss);

	/* Display the MPOOLFILE structures. */
	for (cnt = 0, mfp = SH_TAILQ_FIRST(&dbmp->mp->mpfq, __mpoolfile);
	    mfp != NULL; ++cnt, mfp = SH_TAILQ_NEXT(mfp, q, __mpoolfile));
	(void)fprintf(fp, "%d total files\n", cnt);
	for (cnt = 1, mfp = SH_TAILQ_FIRST(&dbmp->mp->mpfq, __mpoolfile);
	    mfp != NULL; ++cnt, mfp = SH_TAILQ_NEXT(mfp, q, __mpoolfile)) {
		(void)fprintf(fp, "file %d\n", cnt);
		__memp_pmf(fp, mfp, data);
	}

	if (!data)
		return;

	/* Display the hash table list of BH's. */
	(void)fprintf(fp, "%s\nHASH table of BH's (%lu buckets):\n",
	    DB_LINE, (u_long)mp->htab_buckets);
	(void)fprintf(fp,
	    "longest chain searched %lu\n", mp->stat.st_hash_longest);
	(void)fprintf(fp, "average chain searched %lu (total/calls: %lu/%lu)\n",
	    mp->stat.st_hash_examined /
	    (mp->stat.st_hash_searches ? mp->stat.st_hash_searches : 1),
	    mp->stat.st_hash_examined, mp->stat.st_hash_searches);
	for (htabp = dbmp->htab,
	    bucket = 0; bucket < mp->htab_buckets; ++htabp, ++bucket) {
		if (SH_TAILQ_FIRST(&dbmp->htab[bucket], __bh) != NULL)
			(void)fprintf(fp, "%lu:\n", (u_long)bucket);
		for (bhp = SH_TAILQ_FIRST(&dbmp->htab[bucket], __bh);
		    bhp != NULL; bhp = SH_TAILQ_NEXT(bhp, mq, __bh))
			__memp_pbh(fp, dbmp, bhp, data);
	}

	/* Display the LRU list of BH's. */
	(void)fprintf(fp, "LRU list of BH's (pgno/offset):");
	for (sep = "\n    ", bhp = SH_TAILQ_FIRST(&dbmp->mp->bhq, __bh);
	    bhp != NULL; sep = ", ", bhp = SH_TAILQ_NEXT(bhp, q, __bh))
		(void)fprintf(fp, "%s%lu/%lu", sep,
		    (u_long)bhp->pgno, (u_long)OFFSET(dbmp, bhp));
	(void)fprintf(fp, "\n");
}

/*
 * __memp_pmf --
 *	Display an MPOOLFILE structure.
 */
static void
__memp_pmf(fp, mfp, data)
	FILE *fp;
	MPOOLFILE *mfp;
	int data;
{
	(void)fprintf(fp, "    %lu pages created\n", mfp->stat.st_page_create);
	(void)fprintf(fp, "    %lu I/O's (%lu read, %lu written)\n",
	    mfp->stat.st_page_in + mfp->stat.st_page_out,
	    mfp->stat.st_page_in, mfp->stat.st_page_out);
	if (mfp->stat.st_cache_hit + mfp->stat.st_cache_miss != 0)
		(void)fprintf(fp,
		    "    %.0f%% cache hit rate (%lu hit, %lu miss)\n",
		    ((double)mfp->stat.st_cache_hit /
		    (mfp->stat.st_cache_hit + mfp->stat.st_cache_miss)) * 100,
		    mfp->stat.st_cache_hit, mfp->stat.st_cache_miss);
	if (!data)
		return;

	(void)fprintf(fp, "    %d references; %s; pagesize: %lu\n", mfp->ref,
	    mfp->can_mmap ? "mmap" : "read/write",
	    (u_long)mfp->stat.st_pagesize);
}

/*
 * __memp_pbh --
 *	Display a BH structure.
 */
static void
__memp_pbh(fp, dbmp, bhp, data)
	FILE *fp;
	DB_MPOOL *dbmp;
	BH *bhp;
	int data;
{
	const char *sep;

	if (!data)
		return;

	(void)fprintf(fp, "    BH @ %lu (mf: %lu): page %lu; ref %lu",
	    (u_long)OFFSET(dbmp, bhp),
	    (u_long)bhp->mf_offset, (u_long)bhp->pgno, (u_long)bhp->ref);
	sep = "; ";
	if (F_ISSET(bhp, BH_DIRTY)) {
		(void)fprintf(fp, "%sdirty", sep);
		sep = ", ";
	}
	if (F_ISSET(bhp, BH_WRITE)) {
		(void)fprintf(fp, "%schk_write", sep);
		sep = ", ";
	}
	(void)fprintf(fp, "\n");
}
