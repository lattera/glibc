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
static const char sccsid[] = "@(#)db_recover.c	10.12 (Sleepycat) 7/27/97";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#endif

#include "db_int.h"
#include "txn.h"
#include "common_ext.h"
#include "clib_ext.h"

DB_ENV	*db_init __P((char *, int, int));
void	 usage __P((void));
int	 main __P((int, char *[]));

const char *progname = "db_recover";			/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB_ENV *dbenv;
	time_t now;
	int ch, flags, verbose;
	char *home;

	home = NULL;
	flags = verbose = 0;
	while ((ch = getopt(argc, argv, "ch:v")) != EOF)
		switch (ch) {
		case 'c':
			LF_SET(DB_RECOVER_FATAL);
			break;
		case 'h':
			home = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	dbenv = db_init(home, flags, verbose);
	if (verbose) {
		__db_err(dbenv, "Recovery complete at %s", ctime(&now));
		__db_err(dbenv, "%s %lu %s [%lu][%lu]",
		    "Maximum transaction id",
		    (u_long)dbenv->tx_info->region->last_txnid,
		    "Recovery checkpoint",
		    (u_long)dbenv->tx_info->region->last_ckp.file,
		    (u_long)dbenv->tx_info->region->last_ckp.offset);
	}

	exit (db_appexit(dbenv));
}

DB_ENV *
db_init(home, flags, verbose)
	char *home;
	int flags, verbose;
{
	DB_ENV *dbenv;
	int local_flags;

	if ((dbenv = (DB_ENV *)calloc(sizeof(DB_ENV), 1)) == NULL) {
		errno = ENOMEM;
		err(1, NULL);
	}
	dbenv->db_errfile = stderr;
	dbenv->db_errpfx = "db_recover";
	dbenv->db_verbose = verbose;

	/* Initialize environment for pathnames only. */
	local_flags = DB_CREATE | DB_INIT_LOG |
	    DB_INIT_MPOOL | DB_INIT_LOCK | DB_INIT_TXN | DB_USE_ENVIRON;

	if (LF_ISSET(DB_RECOVER_FATAL))
		local_flags |= DB_RECOVER_FATAL;
	else
		local_flags |= DB_RECOVER;

	if ((errno = db_appinit(home, NULL, dbenv, local_flags)) != 0)
		err(1, "appinit failed");

	return (dbenv);
}

void
usage()
{
	(void)fprintf(stderr, "usage: db_recover [-cv] [-h home]\n");
	exit(1);
}
