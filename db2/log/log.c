/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)log.c	10.33 (Sleepycat) 11/2/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "log.h"
#include "db_dispatch.h"
#include "txn_auto.h"
#include "common_ext.h"

static int __log_recover __P((DB_LOG *));

/*
 * log_open --
 *	Initialize and/or join a log.
 */
int
log_open(path, flags, mode, dbenv, lpp)
	const char *path;
	int flags;
	int mode;
	DB_ENV *dbenv;
	DB_LOG **lpp;
{
	DB_LOG *dblp;
	LOG *lp;
	size_t len;
	int fd, newregion, ret, retry_cnt;

	/* Validate arguments. */
#ifdef HAVE_SPINLOCKS
#define	OKFLAGS	(DB_CREATE | DB_THREAD)
#else
#define	OKFLAGS	(DB_CREATE)
#endif
	if ((ret = __db_fchk(dbenv, "log_open", flags, OKFLAGS)) != 0)
		return (ret);

	/*
	 * We store 4-byte offsets into the file, so the maximum file
	 * size can't be larger than that.
	 */
	if (dbenv != NULL && dbenv->lg_max > UINT32_T_MAX) {
		__db_err(dbenv, "log_open: maximum file size too large");
		return (EINVAL);
	}

	/* Create and initialize the DB_LOG structure. */
	if ((dblp = (DB_LOG *)__db_calloc(1, sizeof(DB_LOG))) == NULL)
		return (ENOMEM);

	if (path != NULL && (dblp->dir = __db_strdup(path)) == NULL) {
		__db_free(dblp);
		return (ENOMEM);
	}

	dblp->dbenv = dbenv;
	dblp->lfd = -1;
	ZERO_LSN(dblp->c_lsn);
	dblp->c_fd = -1;

	/*
	 * The log region isn't fixed size because we store the registered
	 * file names there.  Make it fairly large so that we don't have to
	 * grow it.
	 */
	len = 30 * 1024;

	/* Map in the region. */
	retry_cnt = newregion = 0;
retry:	if (LF_ISSET(DB_CREATE)) {
		ret = __db_rcreate(dbenv, DB_APP_LOG, path,
		    DB_DEFAULT_LOG_FILE, mode, len, &fd, &dblp->maddr);
		if (ret == 0) {
			/* Put the LOG structure first in the region. */
			lp = dblp->maddr;

			/* Initialize the rest of the region as free space. */
			dblp->addr = (u_int8_t *)dblp->maddr + sizeof(LOG);
			__db_shalloc_init(dblp->addr, len - sizeof(LOG));

			/* Initialize the LOG structure. */
			lp->persist.lg_max = dbenv == NULL ? 0 : dbenv->lg_max;
			if (lp->persist.lg_max == 0)
				lp->persist.lg_max = DEFAULT_MAX;
			lp->persist.magic = DB_LOGMAGIC;
			lp->persist.version = DB_LOGVERSION;
			lp->persist.mode = mode;
			SH_TAILQ_INIT(&lp->fq);

			/* Initialize LOG LSNs. */
			lp->lsn.file = 1;
			lp->lsn.offset = 0;

			newregion = 1;
		} else if (ret != EEXIST)
			goto err;
	}

	/* If we didn't or couldn't create the region, try and join it. */
	if (!newregion &&
	    (ret = __db_ropen(dbenv, DB_APP_LOG,
	    path, DB_DEFAULT_LOG_FILE, 0, &fd, &dblp->maddr)) != 0) {
		/*
		 * If we fail because the file isn't available, wait a
		 * second and try again.
		 */
		if (ret == EAGAIN && ++retry_cnt < 3) {
			(void)__db_sleep(1, 0);
			goto retry;
		}
		goto err;
	}

	/* Set up the common information. */
	dblp->lp = dblp->maddr;
	dblp->addr = (u_int8_t *)dblp->maddr + sizeof(LOG);
	dblp->fd = fd;

	/* Initialize thread information. */
	if (LF_ISSET(DB_THREAD)) {
		F_SET(dblp, DB_AM_THREAD);

		if (!newregion)
			LOCK_LOGREGION(dblp);
		if ((ret = __db_shalloc(dblp->addr,
		    sizeof(db_mutex_t), MUTEX_ALIGNMENT, &dblp->mutexp)) == 0)
			(void)__db_mutex_init(dblp->mutexp, -1);
		if (!newregion)
			UNLOCK_LOGREGION(dblp);
		if (ret != 0) {
			(void)log_close(dblp);
			if (newregion)
				(void)log_unlink(path, 1, dbenv);
			return (ret);
		}
	}

	/*
	 * If doing recovery, try and recover any previous log files
	 * before releasing the lock.
	 */
	if (newregion) {
		ret = __log_recover(dblp);
		UNLOCK_LOGREGION(dblp);

		if (ret != 0) {
			(void)log_close(dblp);
			(void)log_unlink(path, 1, dbenv);
			return (ret);
		}
	}
	*lpp = dblp;
	return (0);

err:	/*
	 * We never get here with an allocated thread-mutex, so we do
	 * not have to worry about freeing it.
	 */
	FREE(dblp, sizeof(DB_LOG));
	return (ret);

}

/*
 * __log_recover --
 *	Recover a log.
 */
static int
__log_recover(dblp)
	DB_LOG *dblp;
{
	DBT dbt;
	DB_LSN lsn;
	LOG *lp;
	u_int32_t chk;
	int cnt, found_checkpoint, ret;

	lp = dblp->lp;

	/*
	 * Find a log file.  If none exist, we simply return, leaving
	 * everything initialized to a new log.
	 */
	if ((ret = __log_find(dblp, &cnt)) != 0)
		return (ret);
	if (cnt == 0)
		return (0);

	/* We have a log file name, find the last one. */
	while (cnt < MAXLFNAME)
		if (__log_valid(dblp, lp, ++cnt) != 0) {
			--cnt;
			break;
		}

	/*
	 * We have the last useful log file and we've loaded any persistent
	 * information.  Pretend that the log is larger than it can possibly
	 * be, and read this file, looking for a checkpoint and its end.
	 */
	dblp->c_lsn.file = cnt;
	dblp->c_lsn.offset = 0;
	lsn = dblp->c_lsn;
	lp->lsn.file = cnt + 1;
	lp->lsn.offset = 0;

	/* Set the cursor.  Shouldn't fail, leave error messages on. */
	memset(&dbt, 0, sizeof(dbt));
	if ((ret = __log_get(dblp, &lsn, &dbt, DB_SET, 0)) != 0)
		return (ret);

	/*
	 * Read to the end of the file, saving checkpoints.  This will fail
	 * at some point, so turn off error messages.
	 */
	found_checkpoint = 0;
	while (__log_get(dblp, &lsn, &dbt, DB_NEXT, 1) == 0) {
		if (dbt.size < sizeof(u_int32_t))
			continue;
		memcpy(&chk, dbt.data, sizeof(u_int32_t));
		if (chk == DB_txn_ckp) {
			lp->c_lsn = lsn;
			found_checkpoint = 1;
		}
	}

	/*
	 * We know where the end of the log is.  Since that record is on disk,
	 * it's also the last-synced LSN.
	 */
	lp->lsn = lsn;
	lp->lsn.offset += dblp->c_len;
	lp->s_lsn = lp->lsn;

	/* Set up the current buffer information, too. */
	lp->len = dblp->c_len;
	lp->b_off = 0;
	lp->w_off = lp->lsn.offset;

	/*
	 * It's possible that we didn't find a checkpoint because there wasn't
	 * one in the last log file.  Start searching.
	 */
	while (!found_checkpoint && cnt > 1) {
		dblp->c_lsn.file = --cnt;
		dblp->c_lsn.offset = 0;
		lsn = dblp->c_lsn;

		/* Set the cursor.  Shouldn't fail, leave error messages on. */
		if ((ret = __log_get(dblp, &lsn, &dbt, DB_SET, 0)) != 0)
			return (ret);

		/*
		 * Read to the end of the file, saving checkpoints.  Shouldn't
		 * fail, leave error messages on.
		 */
		while (__log_get(dblp, &lsn, &dbt, DB_NEXT, 0) == 0) {
			if (dbt.size < sizeof(u_int32_t))
				continue;
			memcpy(&chk, dbt.data, sizeof(u_int32_t));
			if (chk == DB_txn_ckp) {
				lp->c_lsn = lsn;
				found_checkpoint = 1;
			}
		}
	}

	/* If we never find a checkpoint, that's okay, just 0 it out. */
	if (!found_checkpoint) {
		lp->c_lsn.file = 1;
		lp->c_lsn.offset = 0;
	}

	__db_err(dblp->dbenv,
	    "Recovering the log: last valid LSN: file: %lu offset %lu",
	    (u_long)lp->lsn.file, (u_long)lp->lsn.offset);

	/* Reset the cursor.  */
	ZERO_LSN(dblp->c_lsn);

	return (0);
}

/*
 * __log_find --
 *	Try to find a log file.
 *
 * PUBLIC: int __log_find __P((DB_LOG *, int *));
 */
int
__log_find(dblp, valp)
	DB_LOG *dblp;
	int *valp;
{
	int cnt, fcnt, logval, ret;
	const char *dir;
	char **names, *p, *q;

	/* Find the directory name. */
	if ((ret = __log_name(dblp, 1, &p)) != 0)
		return (ret);
	if ((q = __db_rpath(p)) == NULL)
		dir = PATH_DOT;
	else {
		*q = '\0';
		dir = p;
	}

	/* Get the list of file names. */
	ret = __db_dirlist(dir, &names, &fcnt);
	FREES(p);
	if (ret != 0) {
		__db_err(dblp->dbenv, "%s: %s", dir, strerror(ret));
		return (ret);
	}

	/*
	 * Search for a valid log file name, return a value of 0 on
	 * failure.
	 */
	*valp = 0;
	for (cnt = fcnt, logval = 0; --cnt >= 0;)
		if (strncmp(names[cnt], "log.", sizeof("log.") - 1) == 0) {
			logval = atoi(names[cnt] + 4);
			if (logval != 0 &&
			    __log_valid(dblp, dblp->lp, logval) == 0) {
				*valp = logval;
				break;
			}
		}

	/* Discard the list. */
	__db_dirfree(names, fcnt);

	return (ret);
}

/*
 * log_valid --
 *	Validate a log file.
 *
 * PUBLIC: int __log_valid __P((DB_LOG *, LOG *, int));
 */
int
__log_valid(dblp, lp, cnt)
	DB_LOG *dblp;
	LOG *lp;
	int cnt;
{
	LOGP persist;
	ssize_t nw;
	int fd, ret;
	char *p;

	if ((ret = __log_name(dblp, cnt, &p)) != 0)
		return (ret);

	fd = -1;
	if ((ret = __db_open(p,
	    DB_RDONLY | DB_SEQUENTIAL,
	    DB_RDONLY | DB_SEQUENTIAL, 0, &fd)) != 0 ||
	    (ret = __db_seek(fd, 0, 0, sizeof(HDR), SEEK_SET)) != 0 ||
	    (ret = __db_read(fd, &persist, sizeof(LOGP), &nw)) != 0 ||
	    nw != sizeof(LOGP)) {
		if (ret == 0)
			ret = EIO;
		if (fd != -1) {
			(void)__db_close(fd);
			__db_err(dblp->dbenv,
			    "Ignoring log file: %s: %s", p, strerror(ret));
		}
		goto err;
	}
	(void)__db_close(fd);

	if (persist.magic != DB_LOGMAGIC) {
		__db_err(dblp->dbenv,
		    "Ignoring log file: %s: magic number %lx, not %lx",
		    p, (u_long)persist.magic, (u_long)DB_LOGMAGIC);
		ret = EINVAL;
		goto err;
	}
	if (persist.version < DB_LOGOLDVER || persist.version > DB_LOGVERSION) {
		__db_err(dblp->dbenv,
		    "Ignoring log file: %s: unsupported log version %lu",
		    p, (u_long)persist.version);
		ret = EINVAL;
		goto err;
	}

	if (lp != NULL) {
		lp->persist.lg_max = persist.lg_max;
		lp->persist.mode = persist.mode;
	}
	ret = 0;

err:	FREES(p);
	return (ret);
}

/*
 * log_close --
 *	Close a log.
 */
int
log_close(dblp)
	DB_LOG *dblp;
{
	int ret, t_ret;

	ret = 0;

	/* Discard the per-thread pointer. */
	if (dblp->mutexp != NULL) {
		LOCK_LOGREGION(dblp);
		__db_shalloc_free(dblp->addr, dblp->mutexp);
		UNLOCK_LOGREGION(dblp);
	}

	/* Close the region. */
	if ((t_ret =
	    __db_rclose(dblp->dbenv, dblp->fd, dblp->maddr)) != 0 && ret == 0)
		ret = t_ret;

	/* Close open files, release allocated memory. */
	if (dblp->lfd != -1 && (t_ret = __db_close(dblp->lfd)) != 0 && ret == 0)
		ret = t_ret;
	if (dblp->c_dbt.data != NULL)
		FREE(dblp->c_dbt.data, dblp->c_dbt.ulen);
	if (dblp->c_fd != -1 &&
	    (t_ret = __db_close(dblp->c_fd)) != 0 && ret == 0)
		ret = t_ret;
	if (dblp->dbentry != NULL)
		FREE(dblp->dbentry, (dblp->dbentry_cnt * sizeof(DB_ENTRY)));
	if (dblp->dir != NULL)
		FREES(dblp->dir);

	/* Free the structure. */
	FREE(dblp, sizeof(DB_LOG));

	return (ret);
}

/*
 * log_unlink --
 *	Exit a log.
 */
int
log_unlink(path, force, dbenv)
	const char *path;
	int force;
	DB_ENV *dbenv;
{
	return (__db_runlink(dbenv,
	    DB_APP_LOG, path, DB_DEFAULT_LOG_FILE, force));
}

/*
 * log_stat --
 *	Return LOG statistics.
 */
int
log_stat(dblp, gspp, db_malloc)
	DB_LOG *dblp;
	DB_LOG_STAT **gspp;
	void *(*db_malloc) __P((size_t));
{
	LOG *lp;

	*gspp = NULL;
	lp = dblp->lp;

	if ((*gspp = db_malloc == NULL ?
	    (DB_LOG_STAT *)__db_malloc(sizeof(**gspp)) :
	    (DB_LOG_STAT *)db_malloc(sizeof(**gspp))) == NULL)
		return (ENOMEM);

	/* Copy out the global statistics. */
	LOCK_LOGREGION(dblp);
	**gspp = lp->stat;

	(*gspp)->st_magic = lp->persist.magic;
	(*gspp)->st_version = lp->persist.version;
	(*gspp)->st_mode = lp->persist.mode;
	(*gspp)->st_lg_max = lp->persist.lg_max;

	(*gspp)->st_region_nowait = lp->rlayout.lock.mutex_set_nowait;
	(*gspp)->st_region_wait = lp->rlayout.lock.mutex_set_wait;
	UNLOCK_LOGREGION(dblp);

	return (0);
}
