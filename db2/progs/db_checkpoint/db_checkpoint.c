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
static const char sccsid[] = "@(#)db_checkpoint.c	10.11 (Sleepycat) 8/27/97";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_page.h"
#include "log.h"
#include "btree.h"
#include "hash.h"
#include "clib_ext.h"
#include "common_ext.h"

char	*check __P((DB_ENV *, long, long));
int	 checkpoint __P((DB_ENV *, char *, int));
DB_ENV	*db_init __P((char *));
int	 logpid __P((char *, int));
int	 main __P((int, char *[]));
void	 onint __P((int));
void	 siginit __P((void));
void	 usage __P((void));

int	 interrupted;
time_t	 now;					/* Checkpoint time. */
const char
	*progname = "db_checkpoint";		/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB_ENV *dbenv;
	time_t now;
	long kbytes, minutes, seconds;
	int ch, rval, verbose;
	char *home, *logfile;

	home = logfile = NULL;
	kbytes = minutes = 0;
	verbose = 0;
	while ((ch = getopt(argc, argv, "h:k:L:p:v")) != EOF)
		switch (ch) {
		case 'h':
			home = optarg;
			break;
		case 'k':
			get_long(optarg, 1, LONG_MAX, &kbytes);
			break;
		case 'L':
			logfile = optarg;
			break;
		case 'p':
			get_long(optarg, 1, LONG_MAX, &minutes);
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

	if (kbytes == 0 && minutes == 0) {
		warnx("at least one of -k and -p must be specified");
		usage();
	}

	/* Initialize the environment. */
	dbenv = db_init(home);

	if (logfile != NULL && logpid(logfile, 1)) {
		(void)db_appexit(dbenv);
		return (1);
	}

	/*
	 * If we have only a time delay, then we'll sleep the right amount
	 * to wake up when a checkpoint is necessary.  If we have a "kbytes"
	 * field set, then we'll check every 30 seconds.
	 */
	rval = 0;
	seconds = kbytes != 0 ? 30 : minutes * 60;
	while (!interrupted) {
		(void)__db_sleep(seconds, 0);

		if (verbose) {
			(void)time(&now);
			printf("checkpoint: %s", ctime(&now));
		}
		rval = txn_checkpoint(dbenv->tx_info, kbytes, minutes);
		if (rval < 0)
			break;

		while (rval > 0) {
			if (verbose)
				__db_err(dbenv,
				    "checkpoint did not finish, retrying");
			(void)__db_sleep(2, 0);
			rval = txn_checkpoint(dbenv->tx_info, 0, 0);
		}
		if (rval < 0)
			break;
	}

	if (logfile != NULL && logpid(logfile, 0))
		rval = 1;

	if (interrupted) {
		(void)signal(interrupted, SIG_DFL);
		(void)raise(interrupted);
		/* NOTREACHED */
	}

	return (db_appexit(dbenv) || rval ? 1 : 0);
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

	if ((errno = db_appinit(home, NULL, dbenv,
	   DB_INIT_LOG | DB_INIT_TXN | DB_INIT_MPOOL | DB_USE_ENVIRON)) != 0)
		err(1, "db_appinit");

	if (memp_register(dbenv->mp_info,
	    DB_FTYPE_BTREE, __bam_pgin, __bam_pgout) ||
	    memp_register(dbenv->mp_info,
	    DB_FTYPE_HASH, __ham_pgin, __ham_pgout)) {
		(void)db_appexit(dbenv);
		errx(1,
		    "db_appinit: failed to register access method functions");
	}

	siginit();

	return (dbenv);
}

/*
 * logpid --
 *	Log that we're running.
 */
int
logpid(fname, is_open)
	char *fname;
	int is_open;
{
	FILE *fp;
	time_t now;

	if (is_open) {
		if ((fp = fopen(fname, "w")) == NULL) {
			warn("%s", fname);
			return (1);
		}
		(void)time(&now);
		fprintf(fp,
		    "%s: %lu %s", progname, (u_long)getpid(), ctime(&now));
		fclose(fp);
	} else
		(void)remove(fname);
	return (0);
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
#ifdef SIGKILL
	(void)signal(SIGKILL, onint);
#endif
	(void)signal(SIGTERM, onint);
}

/*
 * oninit --
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
	(void)fprintf(stderr,
    "usage: db_checkpoint [-v] [-h home] [-k kbytes] [-L file] [-p min]\n");
	exit(1);
}
