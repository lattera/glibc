/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_iface.c	10.40 (Sleepycat) 12/19/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_auto.h"
#include "db_ext.h"
#include "common_ext.h"

static int __db_keyempty __P((const DB_ENV *));
static int __db_rdonly __P((const DB_ENV *, const char *));
static int __dbt_ferr __P((const DB *, const char *, const DBT *, int));

/*
 * __db_cdelchk --
 *	Common cursor delete argument checking routine.
 *
 * PUBLIC: int __db_cdelchk __P((const DB *, u_int32_t, int, int));
 */
int
__db_cdelchk(dbp, flags, isrdonly, isvalid)
	const DB *dbp;
	u_int32_t flags;
	int isrdonly, isvalid;
{
	/* Check for changes to a read-only tree. */
	if (isrdonly)
		return (__db_rdonly(dbp->dbenv, "c_del"));

	/* Check for invalid function flags. */
	switch (flags) {
	case 0:
		break;
	default:
		return (__db_ferr(dbp->dbenv, "DBcursor->c_del", 0));
	}

	/*
	 * The cursor must be initialized, return -1 for an invalid cursor,
	 * otherwise 0.
	 */
	return (isvalid ? 0 : EINVAL);
}

/*
 * __db_cgetchk --
 *	Common cursor get argument checking routine.
 *
 * PUBLIC: int __db_cgetchk __P((const DB *, DBT *, DBT *, u_int32_t, int));
 */
int
__db_cgetchk(dbp, key, data, flags, isvalid)
	const DB *dbp;
	DBT *key, *data;
	u_int32_t flags;
	int isvalid;
{
	int key_einval, key_flags, ret;

	key_einval = key_flags = 0;

	/* Check for invalid function flags. */
	LF_CLR(DB_RMW);
	switch (flags) {
	case DB_NEXT_DUP:
		if (dbp->type == DB_RECNO)
			goto err;
		/* FALLTHROUGH */
	case DB_CURRENT:
	case DB_FIRST:
	case DB_LAST:
	case DB_NEXT:
	case DB_PREV:
		key_flags = 1;
		break;
	case DB_GET_BOTH:
	case DB_SET_RANGE:
		key_einval = key_flags = 1;
		break;
	case DB_SET:
		key_einval = 1;
		break;
	case DB_GET_RECNO:
		if (!F_ISSET(dbp, DB_BT_RECNUM))
			goto err;
		break;
	case DB_SET_RECNO:
		if (!F_ISSET(dbp, DB_BT_RECNUM))
			goto err;
		key_einval = key_flags = 1;
		break;
	default:
err:		return (__db_ferr(dbp->dbenv, "DBcursor->c_get", 0));
	}

	/* Check for invalid key/data flags. */
	if ((ret = __dbt_ferr(dbp, "key", key, 0)) != 0)
		return (ret);
	if ((ret = __dbt_ferr(dbp, "data", data, 0)) != 0)
		return (ret);

	/* Check for missing keys. */
	if (key_einval && (key->data == NULL || key->size == 0))
		return (__db_keyempty(dbp->dbenv));

	/*
	 * The cursor must be initialized for DB_CURRENT, return -1 for an
	 * invalid cursor, otherwise 0.
	 */
	return (isvalid || flags != DB_CURRENT ? 0 : EINVAL);
}

/*
 * __db_cputchk --
 *	Common cursor put argument checking routine.
 *
 * PUBLIC: int __db_cputchk __P((const DB *,
 * PUBLIC:    const DBT *, DBT *, u_int32_t, int, int));
 */
int
__db_cputchk(dbp, key, data, flags, isrdonly, isvalid)
	const DB *dbp;
	const DBT *key;
	DBT *data;
	u_int32_t flags;
	int isrdonly, isvalid;
{
	int key_einval, key_flags, ret;

	key_einval = key_flags = 0;

	/* Check for changes to a read-only tree. */
	if (isrdonly)
		return (__db_rdonly(dbp->dbenv, "c_put"));

	/* Check for invalid function flags. */
	switch (flags) {
	case DB_AFTER:
	case DB_BEFORE:
		if (dbp->dup_compare != NULL)
			goto err;
		if (dbp->type == DB_RECNO && !F_ISSET(dbp, DB_RE_RENUMBER))
			goto err;
		if (dbp->type != DB_RECNO && !F_ISSET(dbp, DB_AM_DUP))
			goto err;
		break;
	case DB_CURRENT:
		/*
		 * If there is a comparison function, doing a DB_CURRENT
		 * must not change the part of the data item that is used
		 * for the comparison.
		 */
		break;
	case DB_KEYFIRST:
	case DB_KEYLAST:
		if (dbp->type == DB_RECNO)
			goto err;
		key_einval = key_flags = 1;
		break;
	default:
err:		return (__db_ferr(dbp->dbenv, "DBcursor->c_put", 0));
	}

	/* Check for invalid key/data flags. */
	if (key_flags && (ret = __dbt_ferr(dbp, "key", key, 0)) != 0)
		return (ret);
	if ((ret = __dbt_ferr(dbp, "data", data, 0)) != 0)
		return (ret);

	/* Check for missing keys. */
	if (key_einval && (key->data == NULL || key->size == 0))
		return (__db_keyempty(dbp->dbenv));

	/*
	 * The cursor must be initialized for anything other than DB_KEYFIRST
	 * and DB_KEYLAST, return -1 for an invalid cursor, otherwise 0.
	 */
	return (isvalid ||
	    flags == DB_KEYFIRST || flags == DB_KEYLAST ? 0 : EINVAL);
}

/*
 * __db_closechk --
 *	DB->close flag check.
 *
 * PUBLIC: int __db_closechk __P((const DB *, u_int32_t));
 */
int
__db_closechk(dbp, flags)
	const DB *dbp;
	u_int32_t flags;
{
	/* Check for invalid function flags. */
	if (flags != 0 && flags != DB_NOSYNC)
		return (__db_ferr(dbp->dbenv, "DB->close", 0));

	return (0);
}

/*
 * __db_delchk --
 *	Common delete argument checking routine.
 *
 * PUBLIC: int __db_delchk __P((const DB *, DBT *, u_int32_t, int));
 */
int
__db_delchk(dbp, key, flags, isrdonly)
	const DB *dbp;
	DBT *key;
	u_int32_t flags;
	int isrdonly;
{
	/* Check for changes to a read-only tree. */
	if (isrdonly)
		return (__db_rdonly(dbp->dbenv, "delete"));

	/* Check for invalid function flags. */
	switch (flags) {
	case 0:
		break;
	default:
		return (__db_ferr(dbp->dbenv, "DB->del", 0));
	}

	/* Check for missing keys. */
	if (key->data == NULL || key->size == 0)
		return (__db_keyempty(dbp->dbenv));

	return (0);
}

/*
 * __db_getchk --
 *	Common get argument checking routine.
 *
 * PUBLIC: int __db_getchk __P((const DB *, const DBT *, DBT *, u_int32_t));
 */
int
__db_getchk(dbp, key, data, flags)
	const DB *dbp;
	const DBT *key;
	DBT *data;
	u_int32_t flags;
{
	int ret;

	/* Check for invalid function flags. */
	LF_CLR(DB_RMW);
	switch (flags) {
	case 0:
	case DB_GET_BOTH:
		break;
	case DB_SET_RECNO:
		if (!F_ISSET(dbp, DB_BT_RECNUM))
			goto err;
		break;
	default:
err:		return (__db_ferr(dbp->dbenv, "DB->get", 0));
	}

	/* Check for invalid key/data flags. */
	if ((ret = __dbt_ferr(dbp, "key", key, flags == DB_SET_RECNO)) != 0)
		return (ret);
	if ((ret = __dbt_ferr(dbp, "data", data, 1)) != 0)
		return (ret);

	/* Check for missing keys. */
	if (key->data == NULL || key->size == 0)
		return (__db_keyempty(dbp->dbenv));

	return (0);
}

/*
 * __db_joinchk --
 *	Common join argument checking routine.
 *
 * PUBLIC: int __db_joinchk __P((const DB *, u_int32_t));
 */
int
__db_joinchk(dbp, flags)
	const DB *dbp;
	u_int32_t flags;
{
	if (flags != 0)
		return (__db_ferr(dbp->dbenv, "DB->join", 0));

	return (0);
}

/*
 * __db_putchk --
 *	Common put argument checking routine.
 *
 * PUBLIC: int __db_putchk
 * PUBLIC:    __P((const DB *, DBT *, const DBT *, u_int32_t, int, int));
 */
int
__db_putchk(dbp, key, data, flags, isrdonly, isdup)
	const DB *dbp;
	DBT *key;
	const DBT *data;
	u_int32_t flags;
	int isrdonly, isdup;
{
	int ret;

	/* Check for changes to a read-only tree. */
	if (isrdonly)
		return (__db_rdonly(dbp->dbenv, "put"));

	/* Check for invalid function flags. */
	switch (flags) {
	case 0:
	case DB_NOOVERWRITE:
		break;
	case DB_APPEND:
		if (dbp->type != DB_RECNO)
			goto err;
		break;
	default:
err:		return (__db_ferr(dbp->dbenv, "DB->put", 0));
	}

	/* Check for invalid key/data flags. */
	if ((ret = __dbt_ferr(dbp, "key", key, 0)) != 0)
		return (ret);
	if ((ret = __dbt_ferr(dbp, "data", data, 0)) != 0)
		return (ret);

	/* Check for missing keys. */
	if (key->data == NULL || key->size == 0)
		return (__db_keyempty(dbp->dbenv));

	/* Check for partial puts in the presence of duplicates. */
	if (isdup && F_ISSET(data, DB_DBT_PARTIAL)) {
		__db_err(dbp->dbenv,
"a partial put in the presence of duplicates requires a cursor operation");
		return (EINVAL);
	}

	return (0);
}

/*
 * __db_statchk --
 *	Common stat argument checking routine.
 *
 * PUBLIC: int __db_statchk __P((const DB *, u_int32_t));
 */
int
__db_statchk(dbp, flags)
	const DB *dbp;
	u_int32_t flags;
{
	/* Check for invalid function flags. */
	switch (flags) {
	case 0:
		break;
	case DB_RECORDCOUNT:
		if (dbp->type == DB_RECNO)
			break;
		if (dbp->type == DB_BTREE && F_ISSET(dbp, DB_BT_RECNUM))
			break;
		goto err;
	default:
err:		return (__db_ferr(dbp->dbenv, "DB->stat", 0));
	}

	return (0);
}

/*
 * __db_syncchk --
 *	Common sync argument checking routine.
 *
 * PUBLIC: int __db_syncchk __P((const DB *, u_int32_t));
 */
int
__db_syncchk(dbp, flags)
	const DB *dbp;
	u_int32_t flags;
{
	/* Check for invalid function flags. */
	switch (flags) {
	case 0:
		break;
	default:
		return (__db_ferr(dbp->dbenv, "DB->sync", 0));
	}

	return (0);
}

/*
 * __dbt_ferr --
 *	Check a DBT for flag errors.
 */
static int
__dbt_ferr(dbp, name, dbt, check_thread)
	const DB *dbp;
	const char *name;
	const DBT *dbt;
	int check_thread;
{
	int ret;

	/*
	 * Check for invalid DBT flags.  We allow any of the flags to be
	 * specified to any DB or DBcursor call so that applications can
	 * set DB_DBT_MALLOC when retrieving a data item from a secondary
	 * database and then specify that same DBT as a key to a primary
	 * database, without having to clear flags.
	 */
	if ((ret = __db_fchk(dbp->dbenv, name, dbt->flags,
	    DB_DBT_MALLOC | DB_DBT_USERMEM | DB_DBT_PARTIAL)) != 0)
		return (ret);
	if ((ret = __db_fcchk(dbp->dbenv, name,
	    dbt->flags, DB_DBT_MALLOC, DB_DBT_USERMEM)) != 0)
		return (ret);

	if (check_thread && F_ISSET(dbp, DB_AM_THREAD) &&
	    !F_ISSET(dbt, DB_DBT_MALLOC | DB_DBT_USERMEM)) {
		__db_err(dbp->dbenv,
		    "missing flag thread flag for %s DBT", name);
		return (EINVAL);
	}
	return (0);
}

/*
 * __db_eopnotsup --
 *	Common operation not supported message.
 *
 * PUBLIC: int __db_eopnotsup __P((const DB_ENV *));
 */
int
__db_eopnotsup(dbenv)
	const DB_ENV *dbenv;
{
	__db_err(dbenv, "operation not supported");
#ifdef EOPNOTSUPP
	return (EOPNOTSUPP);
#else
	return (EINVAL);
#endif
}

/*
 * __db_keyempty --
 *	Common missing or empty key value message.
 */
static int
__db_keyempty(dbenv)
	const DB_ENV *dbenv;
{
	__db_err(dbenv, "missing or empty key value specified");
	return (EINVAL);
}

/*
 * __db_rdonly --
 *	Common readonly message.
 */
static int
__db_rdonly(dbenv, name)
	const DB_ENV *dbenv;
	const char *name;
{
	__db_err(dbenv, "%s: attempt to modify a read-only tree", name);
	return (EACCES);
}
