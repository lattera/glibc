/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993
 *	Margo Seltzer.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)dbm.c	10.23 (Sleepycat) 11/22/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#endif

#define	DB_DBM_HSEARCH	1
#include "db_int.h"

#include "db_page.h"
#include "hash.h"

/*
 *
 * This package provides dbm and ndbm compatible interfaces to DB.
 *
 * The DBM routines, which call the NDBM routines.
 */
static DBM *__cur_db;

static void __db_no_open __P((void));

int
__db_dbm_init(file)
	char *file;
{
	if (__cur_db != NULL)
		(void)dbm_close(__cur_db);
	if ((__cur_db =
	    dbm_open(file, O_CREAT | O_RDWR, __db_omode("rw----"))) != NULL)
		return (0);
	if ((__cur_db = dbm_open(file, O_RDONLY, 0)) != NULL)
		return (0);
	return (-1);
}
weak_alias (__db_dbm_init, dbminit)

int
__db_dbm_close()
{
	if (__cur_db != NULL) {
		dbm_close(__cur_db);
		__cur_db = NULL;
	}
	return (0);
}

datum
__db_dbm_fetch(key)
	datum key;
{
	datum item;

	if (__cur_db == NULL) {
		__db_no_open();
		item.dptr = 0;
		return (item);
	}
	return (dbm_fetch(__cur_db, key));
}
weak_alias (__db_dbm_fetch, fetch)

datum
__db_dbm_firstkey()
{
	datum item;

	if (__cur_db == NULL) {
		__db_no_open();
		item.dptr = 0;
		return (item);
	}
	return (dbm_firstkey(__cur_db));
}
#undef firstkey
weak_alias (__db_dbm_firstkey, firstkey)

datum
__db_dbm_nextkey(key)
	datum key;
{
	datum item;

	COMPQUIET(key.dsize, 0);

	if (__cur_db == NULL) {
		__db_no_open();
		item.dptr = 0;
		return (item);
	}
	return (dbm_nextkey(__cur_db));
}
weak_alias (__db_dbm_nextkey, nextkey)

int
__db_dbm_delete(key)
	datum key;
{
	if (__cur_db == NULL) {
		__db_no_open();
		return (-1);
	}
	return (dbm_delete(__cur_db, key));
}
weak_alias (__db_dbm_delete, delete)

int
__db_dbm_store(key, dat)
	datum key, dat;
{
	if (__cur_db == NULL) {
		__db_no_open();
		return (-1);
	}
	return (dbm_store(__cur_db, key, dat, DBM_REPLACE));
}
weak_alias (__db_dbm_store, store)

static void
__db_no_open()
{
	(void)fprintf(stderr, "dbm: no open database.\n");
}

/*
 * This package provides dbm and ndbm compatible interfaces to DB.
 *
 * The NDBM routines, which call the DB routines.
 */
/*
 * Returns:
 * 	*DBM on success
 *	 NULL on failure
 */
DBM *
__db_ndbm_open(file, oflags, mode)
	const char *file;
	int oflags, mode;
{
	DB *dbp;
	DBC *dbc;
	DB_INFO dbinfo;
	int sv_errno;
	char path[MAXPATHLEN];

	memset(&dbinfo, 0, sizeof(dbinfo));
	dbinfo.db_pagesize = 4096;
	dbinfo.h_ffactor = 40;
	dbinfo.h_nelem = 1;

	/*
	 * XXX
	 * Don't use sprintf(3)/snprintf(3) -- the former is dangerous, and
	 * the latter isn't standard, and we're manipulating strings handed
	 * us by the application.
	 */
	if (strlen(file) + strlen(DBM_SUFFIX) + 1 > sizeof(path)) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	(void)strcpy(path, file);
	(void)strcat(path, DBM_SUFFIX);
	if ((errno = db_open(path,
	    DB_HASH, __db_oflags(oflags), mode, NULL, &dbinfo, &dbp)) != 0)
		return (NULL);

	if ((errno = dbp->cursor(dbp, NULL, &dbc, 0)) != 0) {
		sv_errno = errno;
		(void)dbp->close(dbp, 0);
		errno = sv_errno;
		return (NULL);
	}

	return ((DBM *)dbc);
}
weak_alias (__db_ndbm_open, dbm_open)

/*
 * Returns:
 *	Nothing.
 */
void
__db_ndbm_close(dbm)
	DBM *dbm;
{
	DBC *dbc;

	dbc = (DBC *)dbm;

	(void)dbc->dbp->close(dbc->dbp, 0);
}
weak_alias (__db_ndbm_close, dbm_close)

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
datum
__db_ndbm_fetch(dbm, key)
	DBM *dbm;
	datum key;
{
	DBC *dbc;
	DBT _key, _data;
	datum data;
	int ret;

	dbc = (DBC *)dbm;

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));
	_key.size = key.dsize;
	_key.data = key.dptr;

	/*
	 * Note that we can't simply use the dbc we have to do a c_get/SET,
	 * because that cursor is the one used for sequential iteration and
	 * it has to remain stable in the face of intervening gets and puts.
	 */
	if ((ret = dbc->dbp->get(dbc->dbp, NULL, &_key, &_data, 0)) == 0) {
		data.dptr = _data.data;
		data.dsize = _data.size;
	} else {
		data.dptr = NULL;
		data.dsize = 0;
		if (ret == DB_NOTFOUND)
			errno = ENOENT;
		else {
			errno = ret;
			F_SET(dbc->dbp, DB_DBM_ERROR);
		}
	}
	return (data);
}
weak_alias (__db_ndbm_fetch, dbm_fetch)

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
datum
__db_ndbm_firstkey(dbm)
	DBM *dbm;
{
	DBC *dbc;
	DBT _key, _data;
	datum key;
	int ret;

	dbc = (DBC *)dbm;

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));

	if ((ret = dbc->c_get(dbc, &_key, &_data, DB_FIRST)) == 0) {
		key.dptr = _key.data;
		key.dsize = _key.size;
	} else {
		key.dptr = NULL;
		key.dsize = 0;
		if (ret == DB_NOTFOUND)
			errno = ENOENT;
		else {
			errno = ret;
			F_SET(dbc->dbp, DB_DBM_ERROR);
		}
	}
	return (key);
}
weak_alias (__db_ndbm_firstkey, dbm_firstkey)

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
datum
__db_ndbm_nextkey(dbm)
	DBM *dbm;
{
	DBC *dbc;
	DBT _key, _data;
	datum key;
	int ret;

	dbc = (DBC *)dbm;

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));

	if ((ret = dbc->c_get(dbc, &_key, &_data, DB_NEXT)) == 0) {
		key.dptr = _key.data;
		key.dsize = _key.size;
	} else {
		key.dptr = NULL;
		key.dsize = 0;
		if (ret == DB_NOTFOUND)
			errno = ENOENT;
		else {
			errno = ret;
			F_SET(dbc->dbp, DB_DBM_ERROR);
		}
	}
	return (key);
}
weak_alias (__db_ndbm_nextkey, dbm_nextkey)

/*
 * Returns:
 *	 0 on success
 *	<0 failure
 */
int
__db_ndbm_delete(dbm, key)
	DBM *dbm;
	datum key;
{
	DBC *dbc;
	DBT _key;
	int ret;

	dbc = (DBC *)dbm;

	memset(&_key, 0, sizeof(DBT));
	_key.data = key.dptr;
	_key.size = key.dsize;

	if ((ret = dbc->dbp->del(dbc->dbp, NULL, &_key, 0)) == 0)
		return (0);

	if (ret == DB_NOTFOUND)
		errno = ENOENT;
	else {
		errno = ret;
		F_SET(dbc->dbp, DB_DBM_ERROR);
	}
	return (-1);
}
weak_alias (__db_ndbm_delete, dbm_delete)

/*
 * Returns:
 *	 0 on success
 *	<0 failure
 *	 1 if DBM_INSERT and entry exists
 */
int
__db_ndbm_store(dbm, key, data, flags)
	DBM *dbm;
	datum key, data;
	int flags;
{
	DBC *dbc;
	DBT _key, _data;
	int ret;

	dbc = (DBC *)dbm;

	memset(&_key, 0, sizeof(DBT));
	_key.data = key.dptr;
	_key.size = key.dsize;

	memset(&_data, 0, sizeof(DBT));
	_data.data = data.dptr;
	_data.size = data.dsize;

	if ((ret = dbc->dbp->put(dbc->dbp, NULL,
	    &_key, &_data, flags == DBM_INSERT ? DB_NOOVERWRITE : 0)) == 0)
		return (0);

	if (ret == DB_KEYEXIST)
		return (1);

	errno = ret;
	F_SET(dbc->dbp, DB_DBM_ERROR);
	return (-1);
}
weak_alias (__db_ndbm_store, dbm_store)

int
__db_ndbm_error(dbm)
	DBM *dbm;
{
	DBC *dbc;

	dbc = (DBC *)dbm;

	return (F_ISSET(dbc->dbp, DB_DBM_ERROR));
}
weak_alias (__db_ndbm_error, dbm_error)

int
__db_ndbm_clearerr(dbm)
	DBM *dbm;
{
	DBC *dbc;

	dbc = (DBC *)dbm;

	F_CLR(dbc->dbp, DB_DBM_ERROR);
	return (0);
}
weak_alias (__db_ndbm_clearerr, dbm_clearerr)

/*
 * Returns:
 *	1 if read-only
 *	0 if not read-only
 */
int
__db_ndbm_rdonly(dbm)
	DBM *dbm;
{
	DBC *dbc;

	dbc = (DBC *)dbm;

	return (F_ISSET(dbc->dbp, DB_AM_RDONLY) ? 1 : 0);
}

/*
 * XXX
 * We only have a single file descriptor that we can return, not two.  Return
 * the same one for both files.  Hopefully, the user is using it for locking
 * and picked one to use at random.
 */
int
__db_ndbm_dirfno(dbm)
	DBM *dbm;
{
	return (dbm_pagfno(dbm));
}
weak_alias (__db_ndbm_dirfno, dbm_dirfno)

int
__db_ndbm_pagfno(dbm)
	DBM *dbm;
{
	DBC *dbc;
	int fd;

	dbc = (DBC *)dbm;

	(void)dbc->dbp->fd(dbc->dbp, &fd);
	return (fd);
}
weak_alias (__db_ndbm_pagfno, dbm_pagfno)
