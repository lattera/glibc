/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_err.c	10.42 (Sleepycat) 11/24/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "lock.h"
#include "lock_ext.h"
#include "log.h"
#include "log_ext.h"
#include "mp.h"
#include "mp_ext.h"
#include "txn.h"
#include "txn_ext.h"
#include "common_ext.h"
#include "clib_ext.h"

/*
 * __db_fchk --
 *	General flags checking routine.
 *
 * PUBLIC: int __db_fchk __P((DB_ENV *, const char *, u_int32_t, u_int32_t));
 */
int
__db_fchk(dbenv, name, flags, ok_flags)
	DB_ENV *dbenv;
	const char *name;
	u_int32_t flags, ok_flags;
{
	return (flags & ~ok_flags ?  __db_ferr(dbenv, name, 0) : 0);
}

/*
 * __db_fcchk --
 *	General combination flags checking routine.
 *
 * PUBLIC: int __db_fcchk
 * PUBLIC:    __P((DB_ENV *, const char *, u_int32_t, u_int32_t, u_int32_t));
 */
int
__db_fcchk(dbenv, name, flags, flag1, flag2)
	DB_ENV *dbenv;
	const char *name;
	u_int32_t flags, flag1, flag2;
{
	return ((flags & flag1) &&
	    (flags & flag2) ?  __db_ferr(dbenv, name, 1) : 0);
}

/*
 * __db_ferr --
 *	Common flag errors.
 *
 * PUBLIC: int __db_ferr __P((const DB_ENV *, const char *, int));
 */
int
__db_ferr(dbenv, name, iscombo)
	const DB_ENV *dbenv;
	const char *name;
	int iscombo;
{
	__db_err(dbenv, "illegal flag %sspecified to %s",
	    iscombo ? "combination " : "", name);
	return (EINVAL);
}

/*
 * __db_err --
 *	Standard DB error routine.
 *
 * PUBLIC: #ifdef __STDC__
 * PUBLIC: void __db_err __P((const DB_ENV *dbenv, const char *fmt, ...));
 * PUBLIC: #else
 * PUBLIC: void __db_err();
 * PUBLIC: #endif
 */
void
#ifdef __STDC__
__db_err(const DB_ENV *dbenv, const char *fmt, ...)
#else
__db_err(dbenv, fmt, va_alist)
	const DB_ENV *dbenv;
	const char *fmt;
	va_dcl
#endif
{
	va_list ap;
	char errbuf[2048];	/* XXX: END OF THE STACK DON'T TRUST SPRINTF. */

	if (dbenv == NULL)
		return;

	if (dbenv->db_errcall != NULL) {
#ifdef __STDC__
         	va_start(ap, fmt);
#else
	        va_start(ap);
#endif
		(void)vsnprintf(errbuf, sizeof(errbuf), fmt, ap);
		dbenv->db_errcall(dbenv->db_errpfx, errbuf);
		va_end(ap);
	}
	if (dbenv->db_errfile != NULL) {
		if (dbenv->db_errpfx != NULL)
			(void)fprintf(dbenv->db_errfile, "%s: ",
			    dbenv->db_errpfx);
#ifdef __STDC__
         	va_start(ap, fmt);
#else
	        va_start(ap);
#endif
		(void)vfprintf(dbenv->db_errfile, fmt, ap);
		(void)fprintf(dbenv->db_errfile, "\n");
		(void)fflush(dbenv->db_errfile);
		va_end(ap);
	}
}

/*
 * __db_pgerr --
 *	Error when unable to retrieve a specified page.
 *
 * PUBLIC: int __db_pgerr __P((DB *, db_pgno_t));
 */
int
__db_pgerr(dbp, pgno)
	DB *dbp;
	db_pgno_t pgno;
{
	/*
	 * Three things are certain:
	 * Death, taxes, and lost data.
	 * Guess which has occurred.
	 */
	__db_err(dbp->dbenv,
	    "unable to create/retrieve page %lu", (u_long)pgno);
	return (__db_panic(dbp->dbenv, EIO));
}

/*
 * __db_pgfmt --
 *	Error when a page has the wrong format.
 *
 * PUBLIC: int __db_pgfmt __P((DB *, db_pgno_t));
 */
int
__db_pgfmt(dbp, pgno)
	DB *dbp;
	db_pgno_t pgno;
{
	__db_err(dbp->dbenv,
	    "page %lu: illegal page type or format", (u_long)pgno);
	return (__db_panic(dbp->dbenv, EINVAL));
}

/*
 * __db_panic --
 *	Lock out the tree due to unrecoverable error.
 *
 * PUBLIC: int __db_panic __P((DB_ENV *, int));
 */
int
__db_panic(dbenv, errval)
	DB_ENV *dbenv;
	int errval;
{
	if (dbenv != NULL) {
		dbenv->db_panic = errval;

		(void)__log_panic(dbenv);
		(void)__memp_panic(dbenv);
		(void)__lock_panic(dbenv);
		(void)__txn_panic(dbenv);

		__db_err(dbenv, "PANIC: %s", strerror(errval));

		if (dbenv->db_paniccall != NULL)
			dbenv->db_paniccall(dbenv, errval);
	}

	/*
	 * Chaos reigns within.
	 * Reflect, repent, and reboot.
	 * Order shall return.
	 */
	return (DB_RUNRECOVERY);
}
