/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_join.c	10.10 (Sleepycat) 10/9/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_join.h"
#include "db_am.h"
#include "common_ext.h"

static int __db_join_close __P((DBC *));
static int __db_join_del __P((DBC *, u_int32_t));
static int __db_join_get __P((DBC *, DBT *, DBT *, u_int32_t));
static int __db_join_put __P((DBC *, DBT *, DBT *, u_int32_t));

/*
 * This is the duplicate-assisted join functionality.  Right now we're
 * going to write it such that we return one item at a time, although
 * I think we may need to optimize it to return them all at once.
 * It should be easier to get it working this way, and I believe that
 * changing it should be fairly straightforward.
 *
 * XXX
 * Right now we do not maintain the number of duplicates so we do
 * not optimize the join.  If the caller does, then best performance
 * will be achieved by putting the cursor with the smallest cardinality
 * first.
 *
 * The first cursor moves sequentially through the duplicate set while
 * the others search explicitly for the duplicate in question.
 *
 */

/*
 * __db_join --
 *	This is the interface to the duplicate-assisted join functionality.
 * In the same way that cursors mark a position in a database, a cursor
 * can mark a position in a join.  While most cursors are created by the
 * cursor method of a DB, join cursors are created through an explicit
 * call to DB->join.
 *
 * The curslist is an array of existing, intialized cursors and primary
 * is the DB of the primary file.  The data item that joins all the
 * cursors in the curslist is used as the key into the primary and that
 * key and data are returned.  When no more items are left in the join
 * set, the  c_next operation off the join cursor will return DB_NOTFOUND.
 *
 * PUBLIC: int __db_join __P((DB *, DBC **, u_int32_t, DBC **));
 */
int
__db_join(primary, curslist, flags, dbcp)
	DB *primary;
	DBC **curslist, **dbcp;
	u_int32_t flags;
{
	DBC *dbc;
	JOIN_CURSOR *jc;
	int i, ret;

	DB_PANIC_CHECK(primary);

	if ((ret = __db_joinchk(primary, flags)) != 0)
		return (ret);

	if (curslist == NULL || curslist[0] == NULL)
		return (EINVAL);

	dbc = NULL;
	jc = NULL;

	if ((ret = __os_calloc(1, sizeof(DBC), &dbc)) != 0)
		goto err;

	if ((ret = __os_calloc(1, sizeof(JOIN_CURSOR), &jc)) != 0)
		goto err;

	if ((ret = __os_malloc(256, NULL, &jc->j_key.data)) != 0)
		goto err;
	jc->j_key.ulen = 256;
	F_SET(&jc->j_key, DB_DBT_USERMEM);

	for (jc->j_curslist = curslist;
	    *jc->j_curslist != NULL; jc->j_curslist++)
		;
	if ((ret = __os_calloc((jc->j_curslist - curslist + 1),
	    sizeof(DBC *), &jc->j_curslist)) != 0)
		goto err;
	for (i = 0; curslist[i] != NULL; i++) {
		if (i != 0)
			F_SET(curslist[i], DBC_KEYSET);
		jc->j_curslist[i] = curslist[i];
	}

	dbc->c_close = __db_join_close;
	dbc->c_del = __db_join_del;
	dbc->c_get = __db_join_get;
	dbc->c_put = __db_join_put;
	dbc->internal = jc;
	dbc->dbp = primary;
	jc->j_init = 1;
	jc->j_primary = primary;

	*dbcp = dbc;

	return (0);

err:	if (jc != NULL) {
		if (jc->j_curslist != NULL)
			__os_free(jc->j_curslist,
			    (jc->j_curslist - curslist + 1) * sizeof(DBC *));
		__os_free(jc, sizeof(JOIN_CURSOR));
	}
	if (dbc != NULL)
		__os_free(dbc, sizeof(DBC));
	return (ret);
}

static int
__db_join_put(dbc, key, data, flags)
	DBC *dbc;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DB_PANIC_CHECK(dbc->dbp);

	COMPQUIET(key, NULL);
	COMPQUIET(data, NULL);
	COMPQUIET(flags, 0);
	return (EINVAL);
}

static int
__db_join_del(dbc, flags)
	DBC *dbc;
	u_int32_t flags;
{
	DB_PANIC_CHECK(dbc->dbp);

	COMPQUIET(flags, 0);
	return (EINVAL);
}

static int
__db_join_get(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	u_int32_t flags;
{
	DB *dbp;
	DBC **cpp;
	JOIN_CURSOR *jc;
	int ret;
	u_int32_t operation;

	dbp = dbc->dbp;

	DB_PANIC_CHECK(dbp);

	operation = LF_ISSET(DB_OPFLAGS_MASK);
	if (operation != 0 && operation != DB_JOIN_ITEM)
		return (__db_ferr(dbp->dbenv, "DBcursor->c_get", 0));

	LF_CLR(DB_OPFLAGS_MASK);
	if ((ret =
	    __db_fchk(dbp->dbenv, "DBcursor->c_get", flags, DB_RMW)) != 0)
		return (ret);

	jc = (JOIN_CURSOR *)dbc->internal;
retry:
	ret = jc->j_curslist[0]->c_get(jc->j_curslist[0],
	    &jc->j_key, key, jc->j_init ? DB_CURRENT : DB_NEXT_DUP);

	if (ret == ENOMEM) {
		jc->j_key.ulen <<= 1;
		if ((ret = __os_realloc(&jc->j_key.data, jc->j_key.ulen)) != 0)
			return (ret);
		goto retry;
	}
	if (ret != 0)
		return (ret);

	jc->j_init = 0;
	do {
		/*
		 * We have the first element; now look for it in the
		 * other cursors.
		 */
		for (cpp = jc->j_curslist + 1; *cpp != NULL; cpp++) {
retry2:			if ((ret = ((*cpp)->c_get)(*cpp,
			    &jc->j_key, key, DB_GET_BOTH)) == DB_NOTFOUND)
				break;
			if (ret == ENOMEM) {
				jc->j_key.ulen <<= 1;
				if ((ret = __os_realloc(&jc->j_key.data,
				    jc->j_key.ulen)) != 0)
					return (ret);
				goto retry2;
			}
			if (F_ISSET(*cpp, DBC_KEYSET)) {
				F_CLR(*cpp, DBC_KEYSET);
				F_SET(*cpp, DBC_CONTINUE);
			}
		}

		/*
		 * If we got out of here with ret != 0, then we failed to
		 * find the duplicate in one of the files, so we go on to
		 * the next item in the outermost relation. If ret was
		 * equal to 0, then we've got something to return.
		 */
		if (ret == 0)
			break;
	} while ((ret = jc->j_curslist[0]->c_get(jc->j_curslist[0],
	    &jc->j_key, key,  DB_NEXT_DUP)) == 0);

	/*
	 * If ret != 0 here, we've exhausted the first file.  Otherwise,
	 * key and data are set and we need to do the lookup on the
	 * primary.
	 */
	if (ret != 0)
		return (ret);

	if (operation == DB_JOIN_ITEM)
		return (0);
	else
		return ((jc->j_primary->get)(jc->j_primary,
		    jc->j_curslist[0]->txn, key, data, 0));
}

static int
__db_join_close(dbc)
	DBC *dbc;
{
	JOIN_CURSOR *jc;
	int i;

	DB_PANIC_CHECK(dbc->dbp);

	jc = (JOIN_CURSOR *)dbc->internal;

	/*
	 * Clear the optimization flag in the cursors.
	 */
	for (i = 0; jc->j_curslist[i] != NULL; i++)
		F_CLR(jc->j_curslist[i], DBC_CONTINUE | DBC_KEYSET);

	__os_free(jc->j_curslist, 0);
	__os_free(jc->j_key.data, jc->j_key.ulen);
	__os_free(jc, sizeof(JOIN_CURSOR));
	__os_free(dbc, sizeof(DBC));

	return (0);
}
