/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1997\n\
	Sleepycat Software Inc.  All rights reserved.\n";
static const char sccsid[] = "@(#)db_stat.c	8.20 (Sleepycat) 8/27/97";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "clib_ext.h"

#define	DIVIDER	"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

typedef enum { T_NOTSET, T_DB, T_MPOOL, T_TXN } test_t;

void	bstat __P((DB *));
DB_ENV *db_init __P((char *, test_t));
void	hstat __P((DB *));
int	main __P((int, char *[]));
void	mstat __P((DB_ENV *));
void	onint __P((int));
void	prflags __P((u_int32_t, const FN *));
void	tstat __P((DB_ENV *));
int	txn_compare __P((const void *, const void *));
void	usage __P((void));

int	 interrupted;
const char
	*progname = "db_stat";				/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB *dbp;
	DB_ENV *dbenv;
	test_t ttype;
	int ch;
	char *db, *home;

	ttype = T_NOTSET;
	db = home = NULL;
	while ((ch = getopt(argc, argv, "d:h:mt")) != EOF)
		switch (ch) {
		case 'd':
			db = optarg;
			ttype = T_DB;
			break;
		case 'h':
			home = optarg;
			break;
		case 'm':
			ttype = T_MPOOL;
			break;
		case 't':
			ttype = T_TXN;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 0 || ttype == T_NOTSET)
		usage();

	dbenv = db_init(home, ttype);

	(void)signal(SIGINT, onint);

	switch (ttype) {
	case T_DB:
		if ((errno = db_open(db, DB_UNKNOWN,
		    DB_RDONLY, 0, dbenv, NULL, &dbp)) != 0)
			return (1);
		switch (dbp->type) {
		case DB_BTREE:
		case DB_RECNO:
			bstat(dbp);
			break;
		case DB_HASH:
			hstat(dbp);
			break;
		case DB_UNKNOWN:
			abort();		/* Impossible. */
			/* NOTREACHED */
		}
		(void)dbp->close(dbp, 0);
		break;
	case T_MPOOL:
		mstat(dbenv);
		break;
	case T_TXN:
		tstat(dbenv);
		break;
	case T_NOTSET:
		abort();			/* Impossible. */
		/* NOTREACHED */
	}

	(void)db_appexit(dbenv);

	if (interrupted) {
		(void)signal(SIGINT, SIG_DFL);
		(void)raise(SIGINT);
		/* NOTREACHED */
	}
	return (0);
}

/*
 * bstat --
 *	Display btree/recno statistics.
 */
void
bstat(dbp)
	DB *dbp;
{
	static const FN fn[] = {
		{ DB_DUP,	"DB_DUP" },
		{ DB_FIXEDLEN,	"DB_FIXEDLEN" },
		{ DB_RECNUM,	"DB_RECNUM" },
		{ DB_RENUMBER,	"DB_RENUMBER" },
		{ 0 }
	};
	DB_BTREE_STAT *sp;

	if (dbp->stat(dbp, &sp, NULL, 0))
		err(1, "dbp->stat");

#define	PCT(f, t)							\
    (t == 0 ? 0 :							\
    (((double)((t * sp->bt_pagesize) - f) / (t * sp->bt_pagesize)) * 100))

	prflags(sp->bt_flags, fn);
	if (dbp->type == DB_BTREE) {
#ifdef NOT_IMPLEMENTED
		printf("%lu\tMaximum keys per-page.\n", (u_long)sp->bt_maxkey);
#endif
		printf("%lu\tMinimum keys per-page.\n", (u_long)sp->bt_minkey);
	}
	if (dbp->type == DB_RECNO) {
		printf("%lu\tFixed-length record size.\n",
		    (u_long)sp->bt_re_len);
		if (isprint(sp->bt_re_pad))
			printf("%c\tFixed-length record pad.\n",
			    (int)sp->bt_re_pad);
		else
			printf("0x%x\tFixed-length record pad.\n",
			    (int)sp->bt_re_pad);
	}
	printf("%lu\tUnderlying tree page size.\n", (u_long)sp->bt_pagesize);
	printf("%lu\tNumber of levels in the tree.\n", (u_long)sp->bt_levels);
	printf("%lu\tNumber of keys in the tree.\n", (u_long)sp->bt_nrecs);
	printf("%lu\tNumber of tree internal pages.\n", (u_long)sp->bt_int_pg);
	printf("%lu\tNumber of tree leaf pages.\n", (u_long)sp->bt_leaf_pg);
	printf("%lu\tNumber of tree duplicate pages.\n",
	    (u_long)sp->bt_dup_pg);
	printf("%lu\tNumber of tree overflow pages.\n",
	    (u_long)sp->bt_over_pg);
	printf("%lu\tNumber of pages on the free list.\n",
	    (u_long)sp->bt_free);
	printf("%lu\tNumber of pages freed for reuse.\n",
	    (u_long)sp->bt_freed);
	printf("%lu\tNumber of bytes free in tree internal pages (%.0f%% ff)\n",
	    (u_long)sp->bt_int_pgfree,
	    PCT(sp->bt_int_pgfree, sp->bt_int_pg));
	printf("%lu\tNumber of bytes free in tree leaf pages (%.0f%% ff).\n",
	    (u_long)sp->bt_leaf_pgfree,
	    PCT(sp->bt_leaf_pgfree, sp->bt_leaf_pg));
printf("%lu\tNumber of bytes free in tree duplicate pages (%.0f%% ff).\n",
	    (u_long)sp->bt_dup_pgfree,
	    PCT(sp->bt_dup_pgfree, sp->bt_dup_pg));
printf("%lu\tNumber of bytes free in tree overflow pages (%.0f%% ff).\n",
	    (u_long)sp->bt_over_pgfree,
	    PCT(sp->bt_over_pgfree, sp->bt_over_pg));
	printf("%lu\tNumber of bytes saved by prefix compression.\n",
	    (u_long)sp->bt_pfxsaved);
	printf("%lu\tTotal number of tree page splits.\n",
	    (u_long)sp->bt_split);
	printf("%lu\tNumber of root page splits.\n", (u_long)sp->bt_rootsplit);
	printf("%lu\tNumber of fast splits.\n", (u_long)sp->bt_fastsplit);
	printf("%lu\tNumber of hits in tree fast-insert code.\n",
	    (u_long)sp->bt_cache_hit);
	printf("%lu\tNumber of misses in tree fast-insert code.\n",
	    (u_long)sp->bt_cache_miss);
	printf("%lu\tNumber of keys added.\n", (u_long)sp->bt_added);
	printf("%lu\tNumber of keys deleted.\n", (u_long)sp->bt_deleted);
}

/*
 * hstat --
 *	Display hash statistics.
 */
void
hstat(dbp)
	DB *dbp;
{
	return;
}

/*
 * mstat --
 *	Display mpool statistics.
 */
void
mstat(dbenv)
	DB_ENV *dbenv;
{
	DB_MPOOL_FSTAT **fsp;
	DB_MPOOL_STAT *gsp;

	if (memp_stat(dbenv->mp_info, &gsp, &fsp, NULL))
		err(1, NULL);

	printf("%lu\tCache size (%luK).\n",
	    (u_long)gsp->st_cachesize, (u_long)gsp->st_cachesize / 1024);
	printf("%lu\tRequested pages found in the cache", gsp->st_cache_hit);
	if (gsp->st_cache_hit + gsp->st_cache_miss != 0)
		printf(" (%.0f%%)", ((double)gsp->st_cache_hit /
		    (gsp->st_cache_hit + gsp->st_cache_miss)) * 100);
	printf(".\n");
	printf("%lu\tRequested pages mapped into the process' address space.\n",
	    gsp->st_map);
	printf("%lu\tRequested pages not found in the cache.\n",
	    gsp->st_cache_miss);
	printf("%lu\tPages created in the cache.\n", gsp->st_page_create);
	printf("%lu\tPages read into the cache.\n", gsp->st_page_in);
	printf("%lu\tPages written from the cache to the backing file.\n",
	    gsp->st_page_out);
	printf("%lu\tRead-only pages forced from the cache.\n",
	    gsp->st_ro_evict);
	printf("%lu\tRead-write pages forced from the cache.\n",
	    gsp->st_rw_evict);
	printf("%lu\tNumber of hash buckets used for page location.\n",
	    gsp->st_hash_buckets);
	printf("%lu\tTotal number of times hash chains searched for a page.\n",
	    gsp->st_hash_searches);
	printf("%lu\tThe longest hash chain searched for a page.\n",
	    gsp->st_hash_longest);
	printf(
	    "%lu\tTotal number of hash buckets examined for page location.\n",
	    gsp->st_hash_examined);

	for (; fsp != NULL && *fsp != NULL; ++fsp) {
		printf("%s\n", DIVIDER);
		printf("%s\n", (*fsp)->file_name);
		printf("%lu\tPage size.\n", (u_long)(*fsp)->st_pagesize);
		printf("%lu\tRequested pages found in the cache",
		    (*fsp)->st_cache_hit);
		if ((*fsp)->st_cache_hit + (*fsp)->st_cache_miss != 0)
			printf(" (%.0f%%)", ((double)(*fsp)->st_cache_hit /
			    ((*fsp)->st_cache_hit + (*fsp)->st_cache_miss)) *
			    100);
		printf(".\n");
	printf("%lu\tRequested pages mapped into the process' address space.\n",
		    (*fsp)->st_map);
		printf("%lu\tRequested pages not found in the cache.\n",
		    (*fsp)->st_cache_miss);
		printf("%lu\tPages created in the cache.\n",
		    (*fsp)->st_page_create);
		printf("%lu\tPages read into the cache.\n", (*fsp)->st_page_in);
	printf("%lu\tPages written from the cache to the backing file.\n",
		    (*fsp)->st_page_out);
	}
}

/*
 * tstat --
 *	Display transaction statistics.
 */
void
tstat(dbenv)
	DB_ENV *dbenv;
{
	DB_TXN_STAT *tstat;
	u_int32_t i;
	const char *p;

	if (txn_stat(dbenv->tx_info, &tstat, NULL))
		err(1, NULL);

	p = tstat->st_last_ckp.file == 0 ?
	    "No checkpoint LSN." : "File/offset for last checkpoint LSN.";
	printf("%lu/%lu\t%s\n", (u_long)tstat->st_last_ckp.file,
	    (u_long)tstat->st_last_ckp.offset, p);
	p = tstat->st_pending_ckp.file == 0 ?
	    "No pending checkpoint LSN." :
	    "File/offset for last pending checkpoint LSN.";
	printf("%lu/%lu\t%s.\n",
	    (u_long)tstat->st_pending_ckp.file,
	    (u_long)tstat->st_pending_ckp.offset, p);
	if (tstat->st_time_ckp == 0)
		printf("0\tNo checkpoint timestamp.\n");
	else
		printf("%.24s\tCheckpoint timestamp.\n",
		    ctime(&tstat->st_time_ckp));
	printf("%lx\tLast transaction ID allocated.\n",
	    (u_long)tstat->st_last_txnid);
	printf("%lu\tMaximum number of active transactions.\n",
	    (u_long)tstat->st_maxtxns);
	printf("%lu\tNumber of transactions begun.\n",
	    (u_long)tstat->st_nbegins);
	printf("%lu\tNumber of transactions aborted.\n",
	    (u_long)tstat->st_naborts);
	printf("%lu\tNumber of transactions committed.\n",
	    (u_long)tstat->st_ncommits);
	printf("%lu\tActive transactions.\n", (u_long)tstat->st_nactive);
	qsort(tstat->st_txnarray,
	    tstat->st_nactive, sizeof(tstat->st_txnarray[0]), txn_compare);
	for (i = 0; i < tstat->st_nactive; ++i)
		printf("\tid: %lx; initial LSN file/offest %lu/%lu\n",
		    (u_long)tstat->st_txnarray[i].txnid,
		    (u_long)tstat->st_txnarray[i].lsn.file,
		    (u_long)tstat->st_txnarray[i].lsn.offset);
}

int
txn_compare(a1, b1)
	const void *a1, *b1;
{
	const DB_TXN_ACTIVE *a, *b;

	a = a1;
	b = b1;

	if (a->txnid > b->txnid)
		return (1);
	if (a->txnid < b->txnid)
		return (-1);
	return (0);
}

/*
 * prflags --
 *	Print out flag values.
 */
void
prflags(flags, fn)
	u_int32_t flags;
	FN const *fn;
{
	const FN *fnp;
	int found;
	const char *sep;

	sep = " ";
	printf("Flags:");
	for (found = 0, fnp = fn; fnp->mask != 0; ++fnp)
		if (fnp->mask & flags) {
			printf("%s%s", sep, fnp->name);
			sep = ", ";
			found = 1;
		}
	printf("\n");
}

/*
 * db_init --
 *	Initialize the environment.
 */
DB_ENV *
db_init(home, ttype)
	char *home;
	test_t ttype;
{
	DB_ENV *dbenv;
	int flags;

	flags = DB_USE_ENVIRON;
	switch (ttype) {
	case T_MPOOL:
		flags |= DB_INIT_MPOOL;
		break;
	case T_TXN:
		flags |= DB_INIT_TXN;
		break;
	default:
		break;
	}

	if ((dbenv = (DB_ENV *)calloc(sizeof(DB_ENV), 1)) == NULL) {
		errno = ENOMEM;
		err(1, NULL);
	}
	dbenv->db_errfile = stderr;
	dbenv->db_errpfx = progname;

	if ((errno = db_appinit(home, NULL, dbenv, flags)) != 0)
		err(1, "db_appinit");
	return (dbenv);
}

/*
 * oninit --
 *	Interrupt signal handler.
 */
void
onint(signo)
	int signo;
{
	signo = 1;			/* XXX: Shut the compiler up. */
	interrupted = 1;
}

void
usage()
{
	fprintf(stderr, "usage: db_stat [-mt] [-d file] [-h home]\n");
	exit (1);
}
