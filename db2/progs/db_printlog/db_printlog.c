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
static const char sccsid[] = "@(#)db_printlog.c	10.17 (Sleepycat) 11/1/98";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_page.h"
#include "btree.h"
#include "hash.h"
#include "log.h"
#include "txn.h"
#include "db_am.h"
#include "clib_ext.h"

DB_ENV *db_init __P((char *));
int	main __P((int, char *[]));
void	onint __P((int));
void	siginit __P((void));
void	usage __P((void));

int	 interrupted;
const char
	*progname = "db_printlog";			/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB_ENV *dbenv;
	DBT data;
	DB_LSN key;
	int ch, ret;
	char *home;

	ret = 0;
	home = NULL;
	while ((ch = getopt(argc, argv, "h:N")) != EOF)
		switch (ch) {
		case 'h':
			home = optarg;
			break;
		case 'N':
			(void)db_value_set(0, DB_MUTEXLOCKS);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	/* Initialize the environment. */
	siginit();
	dbenv = db_init(home);

	if ((errno = __bam_init_print(dbenv)) != 0 ||
	    (errno = __db_init_print(dbenv)) != 0 ||
	    (errno = __ham_init_print(dbenv)) != 0 ||
	    (errno = __log_init_print(dbenv)) != 0 ||
	    (errno = __txn_init_print(dbenv)) != 0) {
		warn("initialization");
		(void)db_appexit(dbenv);
		return (1);
	}

	memset(&data, 0, sizeof(data));
	while (!interrupted) {
		if ((errno =
		    log_get(dbenv->lg_info, &key, &data, DB_NEXT)) != 0) {
			if (errno == DB_NOTFOUND)
				break;
			warn("log_get");
			goto err;
		}
		if (dbenv->tx_recover != NULL)
			errno = dbenv->tx_recover(dbenv->lg_info,
			    &data, &key, 0, NULL);
		else
			errno = __db_dispatch(dbenv->lg_info,
			    &data, &key, 0, NULL);

		fflush(stdout);
		if (errno != 0) {
			warn("dispatch");
			goto err;
		}
	}

	if (0) {
err:		ret = 1;
	}

	if (dbenv != NULL && (errno = db_appexit(dbenv)) != 0) {
		ret = 1;
		warn(NULL);
	}

	if (interrupted) {
		(void)signal(interrupted, SIG_DFL);
		(void)raise(interrupted);
		/* NOTREACHED */
	}

	return (ret);
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

	if ((dbenv = (DB_ENV *)calloc(sizeof(DB_ENV), 1)) == NULL) {
		errno = ENOMEM;
		err(1, NULL);
	}
	dbenv->db_errfile = stderr;
	dbenv->db_errpfx = progname;

	if ((errno =
	    db_appinit(home, NULL, dbenv, DB_CREATE | DB_INIT_LOG)) != 0)
		err(1, "db_appinit");
	return (dbenv);
}

/*
 * siginit --
 *	Initialize the set of signals for which we want to clean up.
 *	Generally, we try not to leave the shared regions locked if
 *	we can.
 */
void
siginit()
{
#ifdef SIGHUP
	(void)signal(SIGHUP, onint);
#endif
	(void)signal(SIGINT, onint);
	(void)signal(SIGTERM, onint);
}

/*
 * onint --
 *	Interrupt signal handler.
 */
void
onint(signo)
	int signo;
{
	if ((interrupted = signo) == 0)
		interrupted = SIGINT;
}

void
usage()
{
	fprintf(stderr, "usage: db_printlog [-N] [-h home]\n");
	exit (1);
}
