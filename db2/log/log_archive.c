/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)log_archive.c	10.44 (Sleepycat) 10/9/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "db_dispatch.h"
#include "shqueue.h"
#include "log.h"
#include "common_ext.h"
#include "clib_ext.h"			/* XXX: needed for getcwd. */

static int __absname __P((char *, char *, char **));
static int __build_data __P((DB_LOG *, char *, char ***, void *(*)(size_t)));
static int __cmpfunc __P((const void *, const void *));
static int __usermem __P((char ***, void *(*)(size_t)));

/*
 * log_archive --
 *	Supporting function for db_archive(1).
 */
int
log_archive(dblp, listp, flags, db_malloc)
	DB_LOG *dblp;
	char ***listp;
	u_int32_t flags;
	void *(*db_malloc) __P((size_t));
{
	DBT rec;
	DB_LSN stable_lsn;
	u_int32_t fnum;
	int array_size, n, ret;
	char **array, **arrayp, *name, *p, *pref, buf[MAXPATHLEN];

	name = NULL;
	COMPQUIET(fnum, 0);

	LOG_PANIC_CHECK(dblp);

#define	OKFLAGS	(DB_ARCH_ABS | DB_ARCH_DATA | DB_ARCH_LOG)
	if (flags != 0) {
		if ((ret =
		    __db_fchk(dblp->dbenv, "log_archive", flags, OKFLAGS)) != 0)
			return (ret);
		if ((ret =
		    __db_fcchk(dblp->dbenv,
		        "log_archive", flags, DB_ARCH_DATA, DB_ARCH_LOG)) != 0)
			return (ret);
	}

	/*
	 * Get the absolute pathname of the current directory.  It would
	 * be nice to get the shortest pathname of the database directory,
	 * but that's just not possible.
	 */
	if (LF_ISSET(DB_ARCH_ABS)) {
		errno = 0;
		if ((pref = getcwd(buf, sizeof(buf))) == NULL)
			return (errno == 0 ? ENOMEM : errno);
	} else
		pref = NULL;

	switch (LF_ISSET(~DB_ARCH_ABS)) {
	case DB_ARCH_DATA:
		return (__build_data(dblp, pref, listp, db_malloc));
	case DB_ARCH_LOG:
		memset(&rec, 0, sizeof(rec));
		if (F_ISSET(dblp, DB_AM_THREAD))
			F_SET(&rec, DB_DBT_MALLOC);
		if ((ret = log_get(dblp, &stable_lsn, &rec, DB_LAST)) != 0)
			return (ret);
		if (F_ISSET(dblp, DB_AM_THREAD))
			__os_free(rec.data, rec.size);
		fnum = stable_lsn.file;
		break;
	case 0:
		if ((ret = __log_findckp(dblp, &stable_lsn)) != 0) {
			/*
			 * A return of DB_NOTFOUND means that we didn't find
			 * any records in the log (so we are not going to be
			 * deleting any log files).
			 */
			if (ret != DB_NOTFOUND)
				return (ret);
			*listp = NULL;
			return (0);
		}
		/* Remove any log files before the last stable LSN. */
		fnum = stable_lsn.file - 1;
		break;
	}

#define	LIST_INCREMENT	64
	/* Get some initial space. */
	array_size = 10;
	if ((ret = __os_malloc(sizeof(char *) * array_size, NULL, &array)) != 0)
		return (ret);
	array[0] = NULL;

	/* Build an array of the file names. */
	for (n = 0; fnum > 0; --fnum) {
		if ((ret = __log_name(dblp, fnum, &name, NULL, 0)) != 0)
			goto err;
		if (__os_exists(name, NULL) != 0) {
			__os_freestr(name);
			name = NULL;
			break;
		}

		if (n >= array_size - 1) {
			array_size += LIST_INCREMENT;
			if ((ret = __os_realloc(&array,
			    sizeof(char *) * array_size)) != 0)
				goto err;
		}

		if (LF_ISSET(DB_ARCH_ABS)) {
			if ((ret = __absname(pref, name, &array[n])) != 0)
				goto err;
			__os_freestr(name);
		} else if ((p = __db_rpath(name)) != NULL) {
			if ((ret = __os_strdup(p + 1, &array[n])) != 0)
				goto err;
			__os_freestr(name);
		} else
			array[n] = name;

		name = NULL;
		array[++n] = NULL;
	}

	/* If there's nothing to return, we're done. */
	if (n == 0) {
		*listp = NULL;
		ret = 0;
		goto err;
	}

	/* Sort the list. */
	qsort(array, (size_t)n, sizeof(char *), __cmpfunc);

	/* Rework the memory. */
	if ((ret = __usermem(&array, db_malloc)) != 0)
		goto err;

	*listp = array;
	return (0);

err:	if (array != NULL) {
		for (arrayp = array; *arrayp != NULL; ++arrayp)
			__os_freestr(*arrayp);
		__os_free(array, sizeof(char *) * array_size);
	}
	if (name != NULL)
		__os_freestr(name);
	return (ret);
}

/*
 * __build_data --
 *	Build a list of datafiles for return.
 */
static int
__build_data(dblp, pref, listp, db_malloc)
	DB_LOG *dblp;
	char *pref, ***listp;
	void *(*db_malloc) __P((size_t));
{
	DBT rec;
	DB_LSN lsn;
	__log_register_args *argp;
	u_int32_t rectype;
	int array_size, last, n, nxt, ret;
	char **array, **arrayp, *p, *real_name;

	/* Get some initial space. */
	array_size = 10;
	if ((ret = __os_malloc(sizeof(char *) * array_size, NULL, &array)) != 0)
		return (ret);
	array[0] = NULL;

	memset(&rec, 0, sizeof(rec));
	if (F_ISSET(dblp, DB_AM_THREAD))
		F_SET(&rec, DB_DBT_MALLOC);
	for (n = 0, ret = log_get(dblp, &lsn, &rec, DB_FIRST);
	    ret == 0; ret = log_get(dblp, &lsn, &rec, DB_NEXT)) {
		if (rec.size < sizeof(rectype)) {
			ret = EINVAL;
			__db_err(dblp->dbenv, "log_archive: bad log record");
			goto lg_free;
		}

		memcpy(&rectype, rec.data, sizeof(rectype));
		if (rectype != DB_log_register) {
			if (F_ISSET(dblp, DB_AM_THREAD)) {
				__os_free(rec.data, rec.size);
				rec.data = NULL;
			}
			continue;
		}
		if ((ret = __log_register_read(rec.data, &argp)) != 0) {
			ret = EINVAL;
			__db_err(dblp->dbenv,
			    "log_archive: unable to read log record");
			goto lg_free;
		}

		if (n >= array_size - 1) {
			array_size += LIST_INCREMENT;
			if ((ret = __os_realloc(&array,
			    sizeof(char *) * array_size)) != 0)
				goto lg_free;
		}

		if ((ret = __os_strdup(argp->name.data, &array[n])) != 0) {
lg_free:		if (F_ISSET(&rec, DB_DBT_MALLOC) && rec.data != NULL)
				__os_free(rec.data, rec.size);
			goto err1;
		}

		array[++n] = NULL;
		__os_free(argp, 0);

		if (F_ISSET(dblp, DB_AM_THREAD)) {
			__os_free(rec.data, rec.size);
			rec.data = NULL;
		}
	}

	/* If there's nothing to return, we're done. */
	if (n == 0) {
		ret = 0;
		*listp = NULL;
		goto err1;
	}

	/* Sort the list. */
	qsort(array, (size_t)n, sizeof(char *), __cmpfunc);

	/*
	 * Build the real pathnames, discarding nonexistent files and
	 * duplicates.
	 */
	for (last = nxt = 0; nxt < n;) {
		/*
		 * Discard duplicates.  Last is the next slot we're going
		 * to return to the user, nxt is the next slot that we're
		 * going to consider.
		 */
		if (last != nxt) {
			array[last] = array[nxt];
			array[nxt] = NULL;
		}
		for (++nxt; nxt < n &&
		    strcmp(array[last], array[nxt]) == 0; ++nxt) {
			__os_freestr(array[nxt]);
			array[nxt] = NULL;
		}

		/* Get the real name. */
		if ((ret = __db_appname(dblp->dbenv,
		    DB_APP_DATA, NULL, array[last], 0, NULL, &real_name)) != 0)
			goto err2;

		/* If the file doesn't exist, ignore it. */
		if (__os_exists(real_name, NULL) != 0) {
			__os_freestr(real_name);
			__os_freestr(array[last]);
			array[last] = NULL;
			continue;
		}

		/* Rework the name as requested by the user. */
		__os_freestr(array[last]);
		array[last] = NULL;
		if (pref != NULL) {
			ret = __absname(pref, real_name, &array[last]);
			__os_freestr(real_name);
			if (ret != 0)
				goto err2;
		} else if ((p = __db_rpath(real_name)) != NULL) {
			ret = __os_strdup(p + 1, &array[last]);
			__os_freestr(real_name);
			if (ret != 0)
				goto err2;
		} else
			array[last] = real_name;
		++last;
	}

	/* NULL-terminate the list. */
	array[last] = NULL;

	/* Rework the memory. */
	if ((ret = __usermem(&array, db_malloc)) != 0)
		goto err1;

	*listp = array;
	return (0);

err2:	/*
	 * XXX
	 * We've possibly inserted NULLs into the array list, so clean up a
	 * bit so that the other error processing works.
	 */
	if (array != NULL)
		for (; nxt < n; ++nxt)
			__os_freestr(array[nxt]);
	/* FALLTHROUGH */

err1:	if (array != NULL) {
		for (arrayp = array; *arrayp != NULL; ++arrayp)
			__os_freestr(*arrayp);
		__os_free(array, array_size * sizeof(char *));
	}
	return (ret);
}

/*
 * __absname --
 *	Return an absolute path name for the file.
 */
static int
__absname(pref, name, newnamep)
	char *pref, *name, **newnamep;
{
	size_t l_pref, l_name;
	int isabspath, ret;
	char *newname;

	l_name = strlen(name);
	isabspath = __os_abspath(name);
	l_pref = isabspath ? 0 : strlen(pref);

	/* Malloc space for concatenating the two. */
	if ((ret = __os_malloc(l_pref + l_name + 2, NULL, &newname)) != 0)
		return (ret);
	*newnamep = newname;

	/* Build the name.  If `name' is an absolute path, ignore any prefix. */
	if (!isabspath) {
		memcpy(newname, pref, l_pref);
		if (strchr(PATH_SEPARATOR, newname[l_pref - 1]) == NULL)
			newname[l_pref++] = PATH_SEPARATOR[0];
	}
	memcpy(newname + l_pref, name, l_name + 1);

	return (0);
}

/*
 * __usermem --
 *	Create a single chunk of memory that holds the returned information.
 *	If the user has their own malloc routine, use it.
 */
static int
__usermem(listp, db_malloc)
	char ***listp;
	void *(*db_malloc) __P((size_t));
{
	size_t len;
	int ret;
	char **array, **arrayp, **orig, *strp;

	/* Find out how much space we need. */
	for (len = 0, orig = *listp; *orig != NULL; ++orig)
		len += sizeof(char *) + strlen(*orig) + 1;
	len += sizeof(char *);

	/* Allocate it and set up the pointers. */
	if ((ret = __os_malloc(len, db_malloc, &array)) != 0)
		return (ret);

	strp = (char *)(array + (orig - *listp) + 1);

	/* Copy the original information into the new memory. */
	for (orig = *listp, arrayp = array; *orig != NULL; ++orig, ++arrayp) {
		len = strlen(*orig);
		memcpy(strp, *orig, len + 1);
		*arrayp = strp;
		strp += len + 1;

		__os_freestr(*orig);
	}

	/* NULL-terminate the list. */
	*arrayp = NULL;

	__os_free(*listp, 0);
	*listp = array;

	return (0);
}

static int
__cmpfunc(p1, p2)
	const void *p1, *p2;
{
	return (strcmp(*((char * const *)p1), *((char * const *)p2)));
}
