/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_am.c	10.15 (Sleepycat) 12/30/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_page.h"
#include "db_shash.h"
#include "mp.h"
#include "btree.h"
#include "hash.h"
#include "db_am.h"
#include "db_ext.h"

static int __db_c_close __P((DBC *));
static int __db_cursor __P((DB *, DB_TXN *, DBC **, u_int32_t));
static int __db_fd __P((DB *, int *));
static int __db_get __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
static int __db_put __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));

/*
 * __db_init_wrapper --
 *	Wrapper layer to implement generic DB functions.
 *
 * PUBLIC: int __db_init_wrapper __P((DB *));
 */
int
__db_init_wrapper(dbp)
	DB *dbp;
{
	dbp->close = __db_close;
	dbp->cursor = __db_cursor;
	dbp->del = NULL;		/* !!! Must be set by access method. */
	dbp->fd = __db_fd;
	dbp->get = __db_get;
	dbp->join = __db_join;
	dbp->put = __db_put;
	dbp->stat = NULL;		/* !!! Must be set by access method. */
	dbp->sync = __db_sync;

	return (0);
}

/*
 * __db_cursor --
 *	Allocate and return a cursor.
 */
static int
__db_cursor(dbp, txn, dbcp, flags)
	DB *dbp;
	DB_TXN *txn;
	DBC **dbcp;
	u_int32_t flags;
{
	DBC *dbc, *adbc;
	int ret;
	db_lockmode_t mode;
	u_int32_t op;

	DB_PANIC_CHECK(dbp);

	/* Take one from the free list if it's available. */
	DB_THREAD_LOCK(dbp);
	if ((dbc = TAILQ_FIRST(&dbp->free_queue)) != NULL)
		TAILQ_REMOVE(&dbp->free_queue, dbc, links);
	else {
		DB_THREAD_UNLOCK(dbp);

		if ((ret = __os_calloc(1, sizeof(DBC), &dbc)) != 0)
			return (ret);

		dbc->dbp = dbp;
		dbc->c_close = __db_c_close;

		/* Set up locking information. */
		if (F_ISSET(dbp, DB_AM_LOCKING | DB_AM_CDB)) {
 			/*
 			 * If we are not threaded, then there is no need to
 			 * create new locker ids.  We know that no one else
 			 * is running concurrently using this DB, so we can
 			 * take a peek at any cursors on the active queue.
 			 */
 			if (!F_ISSET(dbp, DB_AM_THREAD) &&
 			    (adbc = TAILQ_FIRST(&dbp->active_queue)) != NULL)
 				dbc->lid = adbc->lid;
 			else
 				if ((ret = lock_id(dbp->dbenv->lk_info,
 				    &dbc->lid)) != 0)
 					goto err;
 
			memcpy(dbc->lock.fileid, dbp->fileid, DB_FILE_ID_LEN);
			if (F_ISSET(dbp, DB_AM_CDB)) {
				dbc->lock_dbt.size = DB_FILE_ID_LEN;
				dbc->lock_dbt.data = dbc->lock.fileid;
			} else {
				dbc->lock_dbt.size = sizeof(dbc->lock);
				dbc->lock_dbt.data = &dbc->lock;
			}
		}

		switch (dbp->type) {
		case DB_BTREE:
		case DB_RECNO:
			if ((ret = __bam_c_init(dbc)) != 0)
				goto err;
			break;
		case DB_HASH:
			if ((ret = __ham_c_init(dbc)) != 0)
				goto err;
			break;
		default:
			ret = EINVAL;
			goto err;
		}

		DB_THREAD_LOCK(dbp);
	}

	if ((dbc->txn = txn) == NULL)
		dbc->locker = dbc->lid;
	else
		dbc->locker = txn->txnid;

	TAILQ_INSERT_TAIL(&dbp->active_queue, dbc, links);
	DB_THREAD_UNLOCK(dbp);

	/*
	 * If this is the concurrent DB product, then we do all locking
	 * in the interface, which is right here.
	 */
	if (F_ISSET(dbp, DB_AM_CDB)) {
		op = LF_ISSET(DB_OPFLAGS_MASK);
		mode = (op == DB_WRITELOCK) ? DB_LOCK_WRITE :
		    (LF_ISSET(DB_RMW) ? DB_LOCK_IWRITE : DB_LOCK_READ);
		if ((ret = lock_get(dbp->dbenv->lk_info, dbc->locker, 0,
		    &dbc->lock_dbt, mode, &dbc->mylock)) != 0) {
			(void)__db_c_close(dbc);
			return (EAGAIN);
		}
		if (LF_ISSET(DB_RMW))
			F_SET(dbc, DBC_RMW);
		if (op == DB_WRITELOCK)
			F_SET(dbc, DBC_WRITER);
	}

	*dbcp = dbc;
	return (0);

err:	__os_free(dbc, sizeof(*dbc));
	return (ret);
}

/*
 * __db_c_close --
 *	Close the cursor (recycle for later use).
 */
static int
__db_c_close(dbc)
	DBC *dbc;
{
	DB *dbp;
	int ret, t_ret;

	dbp = dbc->dbp;

	DB_PANIC_CHECK(dbp);

	ret = 0;

	/*
	 * We cannot release the lock until after we've called the
	 * access method specific routine, since btrees may have pending
	 * deletes.
	 */

	/* Remove the cursor from the active queue. */
	DB_THREAD_LOCK(dbp);
	TAILQ_REMOVE(&dbp->active_queue, dbc, links);
	DB_THREAD_UNLOCK(dbp);

	/* Call the access specific cursor close routine. */
	if ((t_ret = dbc->c_am_close(dbc)) != 0 && ret == 0)
		t_ret = ret;

	/* Release the lock. */
	if (F_ISSET(dbc->dbp, DB_AM_CDB) && dbc->mylock != LOCK_INVALID) {
		ret = lock_put(dbc->dbp->dbenv->lk_info, dbc->mylock);
		dbc->mylock = LOCK_INVALID;
	}

	/* Clean up the cursor. */
	dbc->flags = 0;

#ifdef DEBUG
	/*
	 * Check for leftover locks, unless we're running with transactions.
	 *
	 * If we're running tests, display any locks currently held.  It's
	 * possible that some applications may hold locks for long periods,
	 * e.g., conference room locks, but the DB tests should never close
	 * holding locks.
	 */
	if (F_ISSET(dbp, DB_AM_LOCKING) && dbc->lid == dbc->locker) {
		DB_LOCKREQ request;

		request.op = DB_LOCK_DUMP;
		if ((t_ret = lock_vec(dbp->dbenv->lk_info,
		    dbc->locker, 0, &request, 1, NULL)) != 0 && ret == 0)
			ret = EAGAIN;
	}
#endif
	/* Move the cursor to the free queue. */
	DB_THREAD_LOCK(dbp);
	TAILQ_INSERT_TAIL(&dbp->free_queue, dbc, links);
	DB_THREAD_UNLOCK(dbp);

	return (ret);
}

#ifdef DEBUG
/*
 * __db_cprint --
 *	Display the current cursor list.
 *
 * PUBLIC: int __db_cprint __P((DB *));
 */
int
__db_cprint(dbp)
	DB *dbp;
{
	static const FN fn[] = {
		{ DBC_RECOVER, 	"recover" },
		{ DBC_RMW, 	"read-modify-write" },
		{ 0 },
	};
	DBC *dbc;

	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		fprintf(stderr,
		    "%#0x: dbp: %#0x txn: %#0x lid: %lu locker: %lu",
		    (u_int)dbc, (u_int)dbc->dbp, (u_int)dbc->txn,
		    (u_long)dbc->lid, (u_long)dbc->locker);
		__db_prflags(dbc->flags, fn, stderr);
		fprintf(stderr, "\n");
	}
	DB_THREAD_UNLOCK(dbp);

	return (0);
}
#endif /* DEBUG */

/*
 * __db_c_destroy --
 *	Destroy the cursor.
 *
 * PUBLIC: int __db_c_destroy __P((DBC *));
 */
int
__db_c_destroy(dbc)
	DBC *dbc;
{
	DB *dbp;
	int ret;

	dbp = dbc->dbp;

	/* Remove the cursor from the free queue. */
	DB_THREAD_LOCK(dbp);
	TAILQ_REMOVE(&dbp->free_queue, dbc, links);
	DB_THREAD_UNLOCK(dbp);

	/* Call the access specific cursor destroy routine. */
	ret = dbc->c_am_destroy == NULL ? 0 : dbc->c_am_destroy(dbc);

	/* Free up allocated memory. */
	if (dbc->rkey.data != NULL)
		__os_free(dbc->rkey.data, dbc->rkey.ulen);
	if (dbc->rdata.data != NULL)
		__os_free(dbc->rdata.data, dbc->rdata.ulen);
	__os_free(dbc, sizeof(*dbc));

	return (0);
}

/*
 * db_fd --
 *	Return a file descriptor for flock'ing.
 */
static int
__db_fd(dbp, fdp)
        DB *dbp;
	int *fdp;
{
	DB_PANIC_CHECK(dbp);

	/*
	 * XXX
	 * Truly spectacular layering violation.
	 */
	return (__mp_xxx_fd(dbp->mpf, fdp));
}

/*
 * __db_get --
 *	Return a key/data pair.
 */
static int
__db_get(dbp, txn, key, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key, *data;
	u_int32_t flags;
{
	DBC *dbc;
	int ret, t_ret;

	DB_PANIC_CHECK(dbp);

	if ((ret = __db_getchk(dbp, key, data, flags)) != 0)
		return (ret);

	if ((ret = dbp->cursor(dbp, txn, &dbc, 0)) != 0)
		return (ret);

	DEBUG_LREAD(dbc, txn, "__db_get", key, NULL, flags);

	ret = dbc->c_get(dbc, key, data,
	    flags == 0 || flags == DB_RMW ? flags | DB_SET : flags);

	if ((t_ret = __db_c_close(dbc)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}

/*
 * __db_put --
 *	Store a key/data pair.
 */
static int
__db_put(dbp, txn, key, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key, *data;
	u_int32_t flags;
{
	DBC *dbc;
	DBT tdata;
	int ret, t_ret;

	DB_PANIC_CHECK(dbp);

	if ((ret = __db_putchk(dbp, key, data,
	    flags, F_ISSET(dbp, DB_AM_RDONLY), F_ISSET(dbp, DB_AM_DUP))) != 0)
		return (ret);

	if ((ret = dbp->cursor(dbp, txn, &dbc, DB_WRITELOCK)) != 0)
		return (ret);

	DEBUG_LWRITE(dbc, txn, "__db_put", key, data, flags);

	if (flags == DB_NOOVERWRITE) {
		/*
		 * Set DB_DBT_USERMEM, this might be a threaded application and
		 * the flags checking will catch us.  We don't want the actual
		 * data, so request a partial of length 0.
		 */
		memset(&tdata, 0, sizeof(tdata));
		F_SET(&tdata, DB_DBT_USERMEM | DB_DBT_PARTIAL);
		if ((ret = dbc->c_get(dbc, key, &tdata, DB_SET | DB_RMW)) == 0)
			ret = DB_KEYEXIST;
		else
			ret = 0;
	}
	if (ret == 0)
		ret = dbc->c_put(dbc, key, data, DB_KEYLAST);

	if ((t_ret = __db_c_close(dbc)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}

/*
 * __db_sync --
 *	Flush the database cache.
 *
 * PUBLIC: int __db_sync __P((DB *, u_int32_t));
 */
int
__db_sync(dbp, flags)
	DB *dbp;
	u_int32_t flags;
{
	int ret;

	DB_PANIC_CHECK(dbp);

	if ((ret = __db_syncchk(dbp, flags)) != 0)
		return (ret);

	/* If it wasn't possible to modify the file, we're done. */
	if (F_ISSET(dbp, DB_AM_INMEM | DB_AM_RDONLY))
		return (0);

	/* Flush any dirty pages from the cache to the backing file. */
	if ((ret = memp_fsync(dbp->mpf)) == DB_INCOMPLETE)
		ret = 0;

	return (ret);
}
