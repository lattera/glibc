/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)db_am.h	10.6 (Sleepycat) 8/27/97
 */
#ifndef _DB_AM_H
#define _DB_AM_H

#define DB_ISBIG	0x01
#define	DB_ADD_DUP	0x10
#define	DB_REM_DUP	0x20
#define	DB_ADD_BIG	0x30
#define	DB_REM_BIG	0x40
#define	DB_SPLITOLD	0x50
#define	DB_SPLITNEW	0x60

/*
 * Standard initialization and shutdown macros for all recovery functions.
 *
 * Requires the following local variables:
 *
 *	DB *file_dbp, *mdbp;
 *	DB_MPOOLFILE *mpf;
 *	int ret;
 */
#define	REC_INTRO(func) {						\
	file_dbp = mdbp = NULL;						\
	if ((ret = func(dbtp->data, &argp)) != 0)			\
		goto out;						\
	if ((ret = __db_fileid_to_db(logp, &mdbp, argp->fileid)) != 0) {\
		if (ret	== DB_DELETED)					\
			ret = 0;					\
		goto out;						\
	}								\
	if (mdbp == NULL)						\
		goto out;						\
	if (F_ISSET(mdbp, DB_AM_THREAD)) {				\
		if ((ret = __db_gethandle(mdbp,				\
		    mdbp->type == DB_HASH ? __ham_hdup : __bam_bdup,	\
		    &file_dbp)) != 0)					\
			goto out;					\
	} else								\
		file_dbp = mdbp;					\
	F_SET(file_dbp, DB_AM_RECOVER);					\
	mpf = file_dbp->mpf;						\
}
#define	REC_CLOSE {							\
	if (argp != NULL)						\
		free (argp);						\
	if (file_dbp != NULL) {						\
		F_CLR(file_dbp, DB_AM_RECOVER);				\
		if (F_ISSET(file_dbp, DB_AM_THREAD))			\
			__db_puthandle(file_dbp);			\
	}								\
	return (ret);							\
}

/*
 * No-op versions of the same macros.
 */
#define	REC_NOOP_INTRO(func) {						\
	if ((ret = func(dbtp->data, &argp)) != 0)			\
		return (ret);						\
}
#define	REC_NOOP_CLOSE {						\
	if (argp != NULL)						\
		free (argp);						\
	return (ret);							\
}

/*
 * Standard debugging macro for all recovery functions.
 */
#ifdef DEBUG_RECOVER
#define	REC_PRINT(func)							\
	(void)func(logp, dbtp, lsnp, redo, info);
#else
#define	REC_PRINT(func)							\
	info = info;			/* XXX: Shut the compiler up. */
#endif

#include "db_auto.h"
#include "db_ext.h"
#endif
