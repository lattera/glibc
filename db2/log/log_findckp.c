/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)log_findckp.c	10.11 (Sleepycat) 8/27/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "log.h"
#include "txn.h"
#include "common_ext.h"

/*
 * __log_findckp --
 *
 * Looks for the most recent checkpoint that occurs before the most recent
 * checkpoint LSN.  This is the point from which recovery can start and the
 * point up to which archival/truncation can take place.  Checkpoints in
 * the log look like:
 *
 * -------------------------------------------------------------------
 *  | ckp A, ckplsn 100 |  .... record .... | ckp B, ckplsn 600 | ...
 * -------------------------------------------------------------------
 *         LSN 500                                 LSN 1000
 *
 * If we read what log returns from using the DB_CKP parameter to logput,
 * we'll get the record at LSN 1000.  The checkpoint LSN there is 600.
 * Now we have to scan backwards looking for a checkpoint before LSN 600.
 * We find one at 500.  This means that we can truncate the log before
 * 500 or run recovery beginning at 500.
 *
 * Returns 0 if we find a checkpoint.
 * Returns errno on error.
 * Returns DB_NOTFOUND if we could not find a suitable start point and
 * we should start from the beginning.
 *
 * PUBLIC: int __log_findckp __P((DB_LOG *, DB_LSN *));
 */
int
__log_findckp(lp, lsnp)
	DB_LOG *lp;
	DB_LSN *lsnp;
{
	DBT data;
	DB_LSN ckp_lsn, last_ckp, next_lsn;
	__txn_ckp_args *ckp_args;
	int ret, verbose;

	verbose = lp->dbenv != NULL && lp->dbenv->db_verbose != 0;

	/*
	 * Need to find the appropriate point from which to begin
	 * recovery.
	 */
	memset(&data, 0, sizeof(data));
	if (F_ISSET(lp, DB_AM_THREAD))
		F_SET(&data, DB_DBT_MALLOC);
	if ((ret = log_get(lp, &last_ckp, &data, DB_CHECKPOINT)) != 0)
		return (ret == ENOENT ? DB_NOTFOUND : ret);
	ZERO_LSN(ckp_lsn);

	next_lsn = last_ckp;
	do {
		if (F_ISSET(lp, DB_AM_THREAD))
			free(data.data);

		if ((ret = log_get(lp, &next_lsn, &data, DB_SET)) != 0)
			return (ret);
		if ((ret = __txn_ckp_read(data.data, &ckp_args)) != 0) {
			if (F_ISSET(lp, DB_AM_THREAD))
				free(data.data);
			return (ret);
		}
		if (IS_ZERO_LSN(ckp_lsn))
			ckp_lsn = ckp_args->ckp_lsn;
		if (verbose) {
			__db_err(lp->dbenv, "Checkpoint at: [%lu][%lu]",
			    (u_long)last_ckp.file, (u_long)last_ckp.offset);
			__db_err(lp->dbenv, "Checkpoint LSN: [%lu][%lu]",
			    (u_long)ckp_args->ckp_lsn.file,
			    (u_long)ckp_args->ckp_lsn.offset);
			__db_err(lp->dbenv, "Previous checkpoint: [%lu][%lu]",
			    (u_long)ckp_args->last_ckp.file,
			    (u_long)ckp_args->last_ckp.offset);
		}
		last_ckp = next_lsn;
		next_lsn = ckp_args->last_ckp;
		free(ckp_args);
	} while (!IS_ZERO_LSN(next_lsn) &&
	    log_compare(&last_ckp, &ckp_lsn) > 0);

	if (F_ISSET(lp, DB_AM_THREAD))
		free(data.data);

	/*
	 * At this point, either, next_lsn is ZERO or ckp_lsn is the
	 * checkpoint lsn and last_ckp is the LSN of the last checkpoint
	 * before ckp_lsn.  If the compare in the loop is still true, then
	 * next_lsn must be 0 and we need to roll forward from the
	 * beginning of the log.
	 */
	if (log_compare(&last_ckp, &ckp_lsn) > 0) {
		if ((ret = log_get(lp, &last_ckp, &data, DB_FIRST)) != 0)
			return (ret);
		if (F_ISSET(lp, DB_AM_THREAD))
			free(data.data);
	}
	*lsnp = last_ckp;

	if (verbose)
		__db_err(lp->dbenv, "Rolling forward from [%lu][%lu]",
			(u_long)last_ckp.file, (u_long)last_ckp.offset);

	return (IS_ZERO_LSN(last_ckp) ? DB_NOTFOUND : 0);
}
