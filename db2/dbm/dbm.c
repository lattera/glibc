/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
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
static const char sccsid[] = "@(#)dbm.c	10.5 (Sleepycat) 7/19/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/param.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#endif

#define	DB_DBM_HSEARCH
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

/* Provide prototypes here since there are none in db.h.  */
int	 dbm_error __P((DBM *));
int	 dbm_clearerr __P((DBM *));
int	 dbm_dirfno __P((DBM *));
int	 dbm_pagfno __P((DBM *));

int
dbminit(file)
	char *file;
{
	if (__cur_db != NULL)
		(void)dbm_close(__cur_db);
	if ((__cur_db =
	    dbm_open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) != NULL)
		return (0);
	if ((__cur_db = dbm_open(file, O_RDONLY, 0)) != NULL)
		return (0);
	return (-1);
}

datum
fetch(key)
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

datum
firstkey()
{
	datum item;

	if (__cur_db == NULL) {
		__db_no_open();
		item.dptr = 0;
		return (item);
	}
	return (dbm_firstkey(__cur_db));
}

datum
nextkey(key)
	datum key;
{
	datum item;

	if (__cur_db == NULL) {
		__db_no_open();
		item.dptr = 0;
		return (item);
	}
	return (dbm_nextkey(__cur_db));
}

int
delete(key)
	datum key;
{
	int ret;

	if (__cur_db == NULL) {
		__db_no_open();
		return (-1);
	}
	ret = dbm_delete(__cur_db, key);
	if (ret == 0)
		ret = (((DB *)__cur_db)->sync)((DB *)__cur_db, 0);
	return (ret);
}

int
store(key, dat)
	datum key, dat;
{
	int ret;

	if (__cur_db == NULL) {
		__db_no_open();
		return (-1);
	}
	ret = dbm_store(__cur_db, key, dat, DBM_REPLACE);
	if (ret == 0)
		ret = (((DB *)__cur_db)->sync)((DB *)__cur_db, 0);
	return (ret);
}

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
dbm_open(file, oflags, mode)
	const char *file;
	int oflags, mode;
{
	DB *dbp;
	DB_INFO dbinfo;
	char path[MAXPATHLEN];

	memset(&dbinfo, 0, sizeof(dbinfo));
	dbinfo.db_pagesize = 4096;
	dbinfo.h_ffactor = 40;
	dbinfo.h_nelem = 1;

	(void)snprintf(path, sizeof(path), "%s%s", file, DBM_SUFFIX);
	if ((errno = db_open(path,
	    DB_HASH, __db_oflags(oflags), mode, NULL, &dbinfo, &dbp)) != 0)
		return (NULL);
	return ((DBM *)dbp);
}

/*
 * Returns:
 *	Nothing.
 */
void
dbm_close(db)
	DBM *db;
{
	(void)db->close(db, 0);
}

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
datum
dbm_fetch(db, key)
	DBM *db;
	datum key;
{
	DBT _key, _data;
	datum data;
	int status;

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));
	_key.size = key.dsize;
	_key.data = key.dptr;
	status = db->get((DB *)db, NULL, &_key, &_data, 0);
	if (status) {
		data.dptr = NULL;
		data.dsize = 0;
	} else {
		data.dptr = _data.data;
		data.dsize = _data.size;
	}
	return (data);
}

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
datum
dbm_firstkey(db)
	DBM *db;
{
	DBT _key, _data;
	datum key;
	int status;

	DBC *cp;

	if ((cp = TAILQ_FIRST(&db->curs_queue)) == NULL)
		if ((errno = db->cursor(db, NULL, &cp)) != 0) {
			memset(&key, 0, sizeof(key));
			return (key);
		}

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));
	status = (cp->c_get)(cp, &_key, &_data, DB_FIRST);
	if (status) {
		key.dptr = NULL;
		key.dsize = 0;
	} else {
		key.dptr = _key.data;
		key.dsize = _key.size;
	}
	return (key);
}

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
datum
dbm_nextkey(db)
	DBM *db;
{
	DBC *cp;
	DBT _key, _data;
	datum key;
	int status;

	if ((cp = TAILQ_FIRST(&db->curs_queue)) == NULL)
		if ((errno = db->cursor(db, NULL, &cp)) != 0) {
			memset(&key, 0, sizeof(key));
			return (key);
		}

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));
	status = (cp->c_get)(cp, &_key, &_data, DB_NEXT);
	if (status) {
		key.dptr = NULL;
		key.dsize = 0;
	} else {
		key.dptr = _key.data;
		key.dsize = _key.size;
	}
	return (key);
}

/*
 * Returns:
 *	 0 on success
 *	<0 failure
 */
int
dbm_delete(db, key)
	DBM *db;
	datum key;
{
	DBT _key;
	int ret;

	memset(&_key, 0, sizeof(DBT));
	_key.data = key.dptr;
	_key.size = key.dsize;
	ret = (((DB *)db)->del)((DB *)db, NULL, &_key, 0);
	if (ret < 0)
		errno = ENOENT;
	else if (ret > 0) {
		errno = ret;
		ret = -1;
	}
	return (ret);
}

/*
 * Returns:
 *	 0 on success
 *	<0 failure
 *	 1 if DBM_INSERT and entry exists
 */
int
dbm_store(db, key, data, flags)
	DBM *db;
	datum key, data;
	int flags;
{
	DBT _key, _data;

	memset(&_key, 0, sizeof(DBT));
	memset(&_data, 0, sizeof(DBT));
	_key.data = key.dptr;
	_key.size = key.dsize;
	_data.data = data.dptr;
	_data.size = data.dsize;
	return (db->put((DB *)db,
	    NULL, &_key, &_data, (flags == DBM_INSERT) ? DB_NOOVERWRITE : 0));
}

int
dbm_error(db)
	DBM *db;
{
	HTAB *hp;

	hp = (HTAB *)db->internal;
	return (hp->local_errno);
}

int
dbm_clearerr(db)
	DBM *db;
{
	HTAB *hp;

	hp = (HTAB *)db->internal;
	hp->local_errno = 0;
	return (0);
}

/*
 * XXX
 * We only have a single file descriptor that we can return, not two.  Return
 * the same one for both files.  Hopefully, the user is using it for locking
 * and picked one to use at random.
 */
int
dbm_dirfno(db)
	DBM *db;
{
	int fd;

	(void)db->fd(db, &fd);
	return (fd);
}

int
dbm_pagfno(db)
	DBM *db;
{
	int fd;

	(void)db->fd(db, &fd);
	return (fd);
}
