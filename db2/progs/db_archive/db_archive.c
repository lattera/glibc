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
static const char sccsid[] = "@(#)db_archive.c	10.20 (Sleepycat) 10/3/98";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "log.h"
#include "db_dispatch.h"
#include "clib_ext.h"
#include "common_ext.h"

DB_ENV	*db_init __P((char *, int));
int	 main __P((int, char *[]));
void	 nosig __P((void));
void	 usage __P((void));

const char
	*progname = "db_archive";			/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB_ENV *dbenv;
	u_int32_t flags;
	int ch, verbose;
	char *home, **list;

	flags = verbose = 0;
	home = NULL;
	while ((ch = getopt(argc, argv, "ah:lsv")) != EOF)
		switch (ch) {
		case 'a':
			flags |= DB_ARCH_ABS;
			break;
		case 'h':
			home = optarg;
			break;
		case 'l':
			flags |= DB_ARCH_LOG;
			break;
		case 's':
			flags |= DB_ARCH_DATA;
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

	/*
	 * Ignore signals -- we don't want to be interrupted because we're
	 * spending all of our time in the DB library.
	 */
	nosig();
	dbenv = db_init(home, verbose);

	/* Get the list of names. */
	if ((errno = log_archive(dbenv->lg_info, &list, flags, NULL)) != 0) {
		warn(NULL);
		(void)db_appexit(dbenv);
		return (1);
	}

	/* Print the names. */
	if (list != NULL)
		for (; *list != NULL; ++list)
			printf("%s\n", *list);

	if ((errno = db_appexit(dbenv)) != 0) {
		warn(NULL);
		return (1);
	}

	return (0);
}

/*
 * db_init --
 *	Initialize the environment.
 */
DB_ENV *
db_init(home, verbose)
	char *home;
	int verbose;
{
	DB_ENV *dbenv;

	if ((dbenv = (DB_ENV *)calloc(sizeof(DB_ENV), 1)) == NULL) {
		errno = ENOMEM;
		err(1, NULL);
	}
	dbenv->db_errfile = stderr;
	dbenv->db_errpfx = progname;
	dbenv->db_verbose = verbose;

	if ((errno = db_appinit(home, NULL, dbenv,
	    DB_CREATE | DB_INIT_LOG | DB_INIT_TXN | DB_USE_ENVIRON)) != 0)
		err(1, "db_appinit");

	return (dbenv);
}

/*
 * nosig --
 *	We don't want to be interrupted.
 */
void
nosig()
{
#ifdef SIGHUP
	(void)signal(SIGHUP, SIG_IGN);
#endif
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGTERM, SIG_IGN);
}

void
usage()
{
	(void)fprintf(stderr, "usage: db_archive [-alsv] [-h home]\n");
	exit(1);
}
