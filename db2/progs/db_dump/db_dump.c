/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1996, 1997, 1998\n\
	Sleepycat Software Inc.  All rights reserved.\n";
static const char sccsid[] = "@(#)db_dump.c	10.24 (Sleepycat) 11/22/98";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#undef stat

#include "db_int.h"
#include "db_page.h"
#include "btree.h"
#include "hash.h"
#include "clib_ext.h"

void	configure __P((char *));
DB_ENV *db_init __P((char *));
int	main __P((int, char *[]));
void	pheader __P((DB *, int));
void	usage __P((void));

const char
	*progname = "db_dump";				/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB *dbp;
	DBC *dbcp;
	DBT key, data;
	DB_ENV *dbenv;
	int ch, checkprint, dflag;
	char *home;

	home = NULL;
	checkprint = dflag = 0;
	while ((ch = getopt(argc, argv, "df:h:Np")) != EOF)
		switch (ch) {
		case 'd':
			dflag = 1;
			break;
		case 'f':
			if (freopen(optarg, "w", stdout) == NULL)
				err(1, "%s", optarg);
			break;
		case 'h':
			home = optarg;
			break;
		case 'N':
			(void)db_value_set(0, DB_MUTEXLOCKS);
			break;
		case 'p':
			checkprint = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	if (dflag && checkprint)
		errx(1, "the -d and -p options may not both be specified");

	/* Initialize the environment. */
	dbenv = db_init(home);

	/* Open the DB file. */
	if ((errno =
	    db_open(argv[0], DB_UNKNOWN, DB_RDONLY, 0, dbenv, NULL, &dbp)) != 0)
		err(1, "%s", argv[0]);

	/* DB dump. */
	if (dflag) {
		(void)__db_dump(dbp, NULL, 1);
		if ((errno = dbp->close(dbp, 0)) != 0)
			err(1, "close");
		exit (0);
	}

	/* Get a cursor and step through the database. */
	if ((errno = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		(void)dbp->close(dbp, 0);
		err(1, "cursor");
	}

	/* Print out the header. */
	pheader(dbp, checkprint);

	/* Print out the key/data pairs. */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	while ((errno = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
		if (dbp->type != DB_RECNO &&
		    (errno = __db_prdbt(&key, checkprint, stdout)) != 0)
			break;
		if ((errno = __db_prdbt(&data, checkprint, stdout)) != 0)
			break;
	}

	if (errno != DB_NOTFOUND)
		err(1, "cursor get");

	if ((errno = dbp->close(dbp, 0)) != 0)
		err(1, "close");
	return (0);
}

/*
 * db_init --
 *	Initialize the environment.
 */
DB_ENV *
db_init(home)
	char *home;
{
	DB_ENV *dbenv;

	if ((dbenv = (DB_ENV *)calloc(1, sizeof(DB_ENV))) == NULL) {
		errno = ENOMEM;
		err(1, NULL);
	}

	/*
	 * Try and use the shared mpool region so that we get pages that
	 * haven't been flushed to disk (mostly useful for debugging).
	 * If that fails, try again, without the DB_INIT_MPOOL flag.
	 *
	 * If it works, set the error output options so that future errors
	 * are correctly reported.
	 */
	if ((errno = db_appinit(home,
	    NULL, dbenv, DB_USE_ENVIRON | DB_INIT_MPOOL)) == 0) {
		dbenv->db_errfile = stderr;
		dbenv->db_errpfx = progname;
		return (dbenv);
	}

	/* Set the error output options -- this time we want a message. */
	memset(dbenv, 0, sizeof(*dbenv));
	dbenv->db_errfile = stderr;
	dbenv->db_errpfx = progname;

	/* Try again, and it's fatal if we fail. */
	if ((errno = db_appinit(home, NULL, dbenv, DB_USE_ENVIRON)) != 0)
		err(1, "db_appinit");

	return (dbenv);
}

/*
 * pheader --
 *	Write out the header information.
 */
void
pheader(dbp, pflag)
	DB *dbp;
	int pflag;
{
	DBC *dbc;
	DB_BTREE_STAT *btsp;
	HASH_CURSOR *hcp;
	int ret;

	printf("format=%s\n", pflag ? "print" : "bytevalue");
	switch (dbp->type) {
	case DB_BTREE:
		printf("type=btree\n");
		if ((errno = dbp->stat(dbp, &btsp, NULL, 0)) != 0)
			err(1, "dbp->stat");
		if (F_ISSET(dbp, DB_BT_RECNUM))
			printf("recnum=1\n");
		if (btsp->bt_maxkey != 0)
			printf("bt_maxkey=%lu\n", (u_long)btsp->bt_maxkey);
		if (btsp->bt_minkey != 0)
			printf("bt_minkey=%lu\n", (u_long)btsp->bt_minkey);
		break;
	case DB_HASH:
		printf("type=hash\n");
		if ((ret = dbp->cursor(dbp, NULL, &dbc, 0)) != 0)
			break;
		hcp = (HASH_CURSOR *)dbc->internal;
		GET_META(dbp, hcp, ret);
		if (ret == 0) {
			if (hcp->hdr->ffactor != 0)
				printf("h_ffactor=%lu\n",
				    (u_long)hcp->hdr->ffactor);
			if (hcp->hdr->nelem != 0)
				printf("h_nelem=%lu\n",
				    (u_long)hcp->hdr->nelem);
			RELEASE_META(dbp, hcp);
		}
		(void)dbc->c_close(dbc);
		break;
	case DB_RECNO:
		printf("type=recno\n");
		if ((errno = dbp->stat(dbp, &btsp, NULL, 0)) != 0)
			err(1, "dbp->stat");
		if (F_ISSET(dbp, DB_RE_RENUMBER))
			printf("renumber=1\n");
		if (F_ISSET(dbp, DB_RE_FIXEDLEN))
			printf("re_len=%lu\n", (u_long)btsp->bt_re_len);
		if (F_ISSET(dbp, DB_RE_PAD))
			printf("re_pad=%#x\n", btsp->bt_re_pad);
		break;
	case DB_UNKNOWN:
		abort();
		/* NOTREACHED */
	}

	if (F_ISSET(dbp, DB_AM_DUP))
		printf("duplicates=1\n");

	if (dbp->dbenv->db_lorder != 0)
		printf("db_lorder=%lu\n", (u_long)dbp->dbenv->db_lorder);

	if (!F_ISSET(dbp, DB_AM_PGDEF))
		printf("db_pagesize=%lu\n", (u_long)dbp->pgsize);

	printf("HEADER=END\n");
}

/*
 * usage --
 *	Display the usage message.
 */
void
usage()
{
	(void)fprintf(stderr,
	    "usage: db_dump [-dNp] [-f file] [-h home] db_file\n");
	exit(1);
}
