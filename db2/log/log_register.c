/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)log_register.c	10.11 (Sleepycat) 9/15/97";
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
#include "common_ext.h"

/*
 * log_register --
 *	Register a file name.
 */
int
log_register(dblp, dbp, name, type, idp)
	DB_LOG *dblp;
	DB *dbp;
	const char *name;
	DBTYPE type;
	u_int32_t *idp;
{
	DBT r_name;
	DBT fid_dbt;
	DB_LSN r_unused;
	FNAME *fnp;
	size_t len;
	u_int32_t fid;
	int inserted, ret;
	char *fullname;
	void *fidp, *namep;

	fid = 0;
	inserted = 0;
	fullname = NULL;
	fnp = fidp = namep = NULL;

	/* Check the arguments. */
	if (type != DB_BTREE && type != DB_HASH && type != DB_RECNO) {
		__db_err(dblp->dbenv, "log_register: unknown DB file type");
		return (EINVAL);
	}

	/* Get the log file id. */
	if ((ret = __db_appname(dblp->dbenv,
	    DB_APP_DATA, NULL, name, NULL, &fullname)) != 0)
		return (ret);

	LOCK_LOGREGION(dblp);

	/*
	 * See if we've already got this file in the log, finding the
	 * next-to-lowest file id currently in use as we do it.
	 */
	for (fid = 1, fnp = SH_TAILQ_FIRST(&dblp->lp->fq, __fname);
	    fnp != NULL; fnp = SH_TAILQ_NEXT(fnp, q, __fname)) {
		if (fid <= fnp->id)
			fid = fnp->id + 1;
		if (!memcmp(dbp->lock.fileid,
		    ADDR(dblp, fnp->fileid_off), DB_FILE_ID_LEN)) {
			++fnp->ref;
			fid = fnp->id;
			if (!F_ISSET(dblp, DB_AM_RECOVER) &&
			    (ret = __log_add_logid(dblp, dbp, fid) != 0))
				goto err;
			goto ret1;
		}
	}

	/* Allocate a new file name structure. */
	if ((ret = __db_shalloc(dblp->addr, sizeof(FNAME), 0, &fnp)) != 0)
		goto err;
	fnp->ref = 1;
	fnp->id = fid;
	fnp->s_type = type;

	if ((ret = __db_shalloc(dblp->addr, DB_FILE_ID_LEN, 0, &fidp)) != 0)
		goto err;
	/*
	 * XXX Now that uids are fixed size, we can put them in the fnp
	 * structure.
	 */
	fnp->fileid_off = OFFSET(dblp, fidp);
	memcpy(fidp, dbp->lock.fileid, DB_FILE_ID_LEN);

	len = strlen(name) + 1;
	if ((ret = __db_shalloc(dblp->addr, len, 0, &namep)) != 0)
		goto err;
	fnp->name_off = OFFSET(dblp, namep);
	memcpy(namep, name, len);

	SH_TAILQ_INSERT_HEAD(&dblp->lp->fq, fnp, q, __fname);
	inserted = 1;

	/* Log the registry. */
	if (!F_ISSET(dblp, DB_AM_RECOVER)) {
		r_name.data = (void *)name;		/* XXX: Yuck! */
		r_name.size = strlen(name) + 1;
		memset(&fid_dbt, 0, sizeof(fid_dbt));
		fid_dbt.data = dbp->lock.fileid;
		fid_dbt.size = DB_FILE_ID_LEN;
		if ((ret = __log_register_log(dblp, NULL, &r_unused,
		    0, &r_name, &fid_dbt, fid, type)) != 0)
			goto err;
		if ((ret = __log_add_logid(dblp, dbp, fid)) != 0)
			goto err;
	}

	if (0) {
err:		/*
		 * XXX
		 * We should grow the region.
		 */
		if (inserted)
			SH_TAILQ_REMOVE(&dblp->lp->fq, fnp, q, __fname);
		if (namep != NULL)
			__db_shalloc_free(dblp->addr, namep);
		if (fidp != NULL)
			__db_shalloc_free(dblp->addr, fidp);
		if (fnp != NULL)
			__db_shalloc_free(dblp->addr, fnp);
	}

ret1:	UNLOCK_LOGREGION(dblp);

	if (fullname != NULL)
		FREES(fullname);

	if (idp != NULL)
		*idp = fid;
	return (ret);
}

/*
 * log_unregister --
 *	Discard a registered file name.
 */
int
log_unregister(dblp, fid)
	DB_LOG *dblp;
	u_int32_t fid;
{
	DB_LSN r_unused;
	FNAME *fnp;
	int ret;

	ret = 0;
	LOCK_LOGREGION(dblp);

	/* Unlog the registry. */
	if (!F_ISSET(dblp, DB_AM_RECOVER) &&
	    (ret = __log_unregister_log(dblp, NULL, &r_unused, 0, fid)) != 0)
		return (ret);

	/* Find the entry in the log. */
	for (fnp = SH_TAILQ_FIRST(&dblp->lp->fq, __fname);
	    fnp != NULL; fnp = SH_TAILQ_NEXT(fnp, q, __fname))
		if (fid == fnp->id)
			break;
	if (fnp == NULL) {
		__db_err(dblp->dbenv, "log_unregister: non-existent file id");
		ret = EINVAL;
		goto ret1;
	}

	/* If more than 1 reference, decrement the reference and return. */
	if (fnp->ref > 1) {
		--fnp->ref;
		goto ret1;
	}

	/* Free the unique file information, name and structure. */
	__db_shalloc_free(dblp->addr, ADDR(dblp, fnp->fileid_off));
	__db_shalloc_free(dblp->addr, ADDR(dblp, fnp->name_off));
	SH_TAILQ_REMOVE(&dblp->lp->fq, fnp, q, __fname);
	__db_shalloc_free(dblp->addr, fnp);

	/*
	 * Remove from the process local table.  If this operation is taking
	 * place during recovery, then the logid was never added to the table,
	 * so do not remove it.
	 */
	if (!F_ISSET(dblp, DB_AM_RECOVER))
		__log_rem_logid(dblp, fid);

ret1:	UNLOCK_LOGREGION(dblp);

	return (ret);
}
