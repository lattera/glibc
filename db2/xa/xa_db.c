/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)xa_db.c	10.6 (Sleepycat) 12/19/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#endif

#undef stat

#include "db_int.h"
#include "db_page.h"
#include "xa.h"
#include "xa_ext.h"
#include "db_am.h"
#include "db_ext.h"
#include "common_ext.h"

static int __xa_c_close __P((DBC *));
static int __xa_c_del __P((DBC *, u_int32_t));
static int __xa_c_get __P((DBC *, DBT *, DBT *, u_int32_t));
static int __xa_c_put __P((DBC *, DBT *, DBT *, u_int32_t));
static int __xa_close __P((DB *, u_int32_t));
static int __xa_cursor __P((DB *, DB_TXN *, DBC **, u_int32_t));
static int __xa_del __P((DB *, DB_TXN *, DBT *, u_int32_t));
static int __xa_fd __P((DB *, int *));
static int __xa_get __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
static int __xa_put __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
static int __xa_stat __P((DB *, void *, void *(*)(size_t), u_int32_t));
static int __xa_sync __P((DB *, u_int32_t));

int
db_xa_open(fname, type, flags, mode, dbinfo, dbpp)
	const char *fname;
	DBTYPE type;
	u_int32_t flags;
	int mode;
	DB_INFO *dbinfo;
	DB **dbpp;
{
	DB *dbp, *real_dbp;
	DB_ENV *dbenv;
	struct __rmname *rp;
	int ret;

	/*
	 * First try to open up the underlying DB.
	 *
	 * !!!
	 * The dbenv argument is taken from the global list of environments.
	 * When the transaction manager called xa_start() (__db_xa_start()),
	 * the "current" DB environment was moved to the start of the list.
	 * However, if we were called in a tpsvrinit function (which is
	 * entirely plausible), then it's possible that xa_open was called
	 * (which simply recorded the name of the environment to open) and
	 * this is the next call into DB.  In that case, we still have to
	 * open the environment.
	 *
	 * The way that we know that xa_open and nothing else was called
	 * is because the nameq is not NULL.
	 */
	if ((rp = TAILQ_FIRST(&DB_GLOBAL(db_nameq))) != NULL &&
	    (ret = __db_rmid_to_env(rp->rmid, &dbenv, 1)) != 0)
		return (ret);

	dbenv = TAILQ_FIRST(&DB_GLOBAL(db_envq));
	if ((ret = db_open(fname,
	    type, flags, mode, dbenv, dbinfo, &real_dbp)) != 0)
		return (ret);

	/*
	 * Allocate our own DB handle, and copy the exported fields and
	 * function pointers into it.  The internal pointer references
	 * the real underlying DB handle.
	 */
	if ((ret = __os_calloc(1, sizeof(DB), &dbp)) != 0) {
		(void)real_dbp->close(real_dbp, 0);
		return (ret);
	}
	dbp->type = real_dbp->type;
	dbp->byteswapped = real_dbp->byteswapped;
	dbp->dbenv = dbenv;
	dbp->internal = real_dbp;
	TAILQ_INIT(&dbp->active_queue);
	TAILQ_INIT(&dbp->free_queue);
	dbp->close = __xa_close;
	dbp->cursor = __xa_cursor;
	dbp->del = __xa_del;
	dbp->fd = __xa_fd;
	dbp->get = __xa_get;
	dbp->join = real_dbp->join;
	dbp->put = __xa_put;
	dbp->stat = __xa_stat;
	dbp->sync = __xa_sync;

	*dbpp = dbp;
	return (0);
}

static int
__xa_close(dbp, flags)
	DB *dbp;
	u_int32_t flags;
{
	DB *real_dbp;
	DBC *dbc;
	int ret;

	/* Close any associated cursors. */
	while ((dbc = TAILQ_FIRST(&dbp->active_queue)) != NULL)
		(void)dbc->c_close(dbc);

	/* Close the DB handle. */
	real_dbp = (DB *)dbp->internal;
	ret = real_dbp->close(real_dbp, flags);

	__os_free(dbp, sizeof(DB));
	return (ret);
}

static int
__xa_cursor(dbp, txn, dbcp, flags)
	DB *dbp;
	DB_TXN *txn;
	DBC **dbcp;
	u_int32_t flags;
{
	DB *real_dbp;
	DBC *real_dbc, *dbc;
	int ret;

	real_dbp = (DB *)dbp->internal;
	txn = dbp->dbenv->xa_txn;

	if ((ret = real_dbp->cursor(real_dbp, txn, &real_dbc, flags)) != 0)
		return (ret);

	/*
	 * Allocate our own DBC handle, and copy the exported fields and
	 * function pointers into it.  The internal pointer references
	 * the real underlying DBC handle.
	 */
	if ((ret = __os_calloc(1, sizeof(DBC), &dbc)) != 0) {
		(void)real_dbc->c_close(real_dbc);
		return (ret);
	}
	dbc->dbp = dbp;
	dbc->c_close = __xa_c_close;
	dbc->c_del = __xa_c_del;
	dbc->c_get = __xa_c_get;
	dbc->c_put = __xa_c_put;
	dbc->internal = real_dbc;
	TAILQ_INSERT_TAIL(&dbp->active_queue, dbc, links);

	*dbcp = dbc;
	return (0);
}

static int
__xa_fd(dbp, fdp)
	DB *dbp;
	int *fdp;
{
	DB *real_dbp;

	COMPQUIET(fdp, NULL);

	real_dbp = (DB *)dbp->internal;
	return (__db_eopnotsup(real_dbp->dbenv));
}

static int
__xa_del(dbp, txn, key, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	DB *real_dbp;

	real_dbp = (DB *)dbp->internal;
	txn = dbp->dbenv->xa_txn;

	return (real_dbp->del(real_dbp, txn, key, flags));
}

static int
__xa_get(dbp, txn, key, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DB *real_dbp;

	real_dbp = (DB *)dbp->internal;
	txn = dbp->dbenv->xa_txn;

	return (real_dbp->get(real_dbp, txn, key, data, flags));
}

static int
__xa_put(dbp, txn, key, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DB *real_dbp;

	real_dbp = (DB *)dbp->internal;
	txn = dbp->dbenv->xa_txn;

	return (real_dbp->put(real_dbp, txn, key, data, flags));
}

static int
__xa_stat(dbp, spp, db_malloc, flags)
	DB *dbp;
	void *spp;
	void *(*db_malloc) __P((size_t));
	u_int32_t flags;
{
	DB *real_dbp;

	real_dbp = (DB *)dbp->internal;
	return (real_dbp->stat(real_dbp, spp, db_malloc, flags));
}

static int
__xa_sync(dbp, flags)
	DB *dbp;
	u_int32_t flags;
{
	DB *real_dbp;

	real_dbp = (DB *)dbp->internal;
	return (real_dbp->sync(real_dbp, flags));
}

static int
__xa_c_close(dbc)
	DBC *dbc;
{
	DBC *real_dbc;
	int ret;

	real_dbc = (DBC *)dbc->internal;

	ret = real_dbc->c_close(real_dbc);

	TAILQ_REMOVE(&dbc->dbp->active_queue, dbc, links);
	__os_free(dbc, sizeof(DBC));

	return (ret);
}

static int
__xa_c_del(dbc, flags)
	DBC *dbc;
	u_int32_t flags;
{
	DBC *real_dbc;

	real_dbc = (DBC *)dbc->internal;
	return (real_dbc->c_del(real_dbc, flags));
}

static int
__xa_c_get(dbc, key, data, flags)
	DBC *dbc;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DBC *real_dbc;

	real_dbc = (DBC *)dbc->internal;
	return (real_dbc->c_get(real_dbc, key, data, flags));
}

static int
__xa_c_put(dbc, key, data, flags)
	DBC *dbc;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DBC *real_dbc;

	real_dbc = (DBC *)dbc->internal;
	return (real_dbc->c_put(real_dbc, key, data, flags));
}
