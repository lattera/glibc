/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_appinit.c	10.66 (Sleepycat) 12/7/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_page.h"
#include "btree.h"
#include "hash.h"
#include "log.h"
#include "txn.h"
#include "clib_ext.h"
#include "common_ext.h"

static int __db_home __P((DB_ENV *, const char *, u_int32_t));
static int __db_parse __P((DB_ENV *, char *));
static int __db_tmp_open __P((DB_ENV *, u_int32_t, char *, int *));

/*
 * This conflict array is used for concurrent db access (cdb).  It
 * uses the same locks as the db_rw_conflict array, but adds an IW
 * mode to be used for write cursors.
 */
static u_int8_t const db_cdb_conflicts[] = {
	/*		N   R   W  IW */
	/*    N */	0,  0,  0,  0,
	/*    R */	0,  0,  1,  0,
	/*    W */	0,  1,  1,  1,
	/*   IW */	0,  0,  1,  1
};

/*
 * db_version --
 *	Return version information.
 */
char *
db_version(majverp, minverp, patchp)
	int *majverp, *minverp, *patchp;
{
	if (majverp != NULL)
		*majverp = DB_VERSION_MAJOR;
	if (minverp != NULL)
		*minverp = DB_VERSION_MINOR;
	if (patchp != NULL)
		*patchp = DB_VERSION_PATCH;
	return ((char *)DB_VERSION_STRING);
}

/*
 * db_appinit --
 *	Initialize the application environment.
 */
int
db_appinit(db_home, db_config, dbenv, flags)
	const char *db_home;
	char * const *db_config;
	DB_ENV *dbenv;
	u_int32_t flags;
{
	FILE *fp;
	int mode, ret;
	char * const *p;
	char *lp, buf[MAXPATHLEN * 2];

	fp = NULL;

	/* Validate arguments. */
	if (dbenv == NULL)
		return (EINVAL);

#ifdef HAVE_SPINLOCKS
#define	OKFLAGS								\
    (DB_CREATE | DB_INIT_CDB | DB_INIT_LOCK | DB_INIT_LOG |		\
    DB_INIT_MPOOL | DB_INIT_TXN | DB_MPOOL_PRIVATE | DB_NOMMAP |	\
    DB_RECOVER | DB_RECOVER_FATAL | DB_THREAD | DB_TXN_NOSYNC |		\
    DB_USE_ENVIRON | DB_USE_ENVIRON_ROOT)
#else
#define	OKFLAGS								\
    (DB_CREATE | DB_INIT_CDB | DB_INIT_LOCK | DB_INIT_LOG |		\
    DB_INIT_MPOOL | DB_INIT_TXN | DB_MPOOL_PRIVATE | DB_NOMMAP |	\
    DB_RECOVER | DB_RECOVER_FATAL | DB_TXN_NOSYNC |			\
    DB_USE_ENVIRON | DB_USE_ENVIRON_ROOT)
#endif
	if ((ret = __db_fchk(dbenv, "db_appinit", flags, OKFLAGS)) != 0)
		return (ret);

	/* Transactions imply logging. */
	if (LF_ISSET(DB_INIT_TXN))
		LF_SET(DB_INIT_LOG);

	/* Convert the db_appinit(3) flags. */
	if (LF_ISSET(DB_THREAD))
		F_SET(dbenv, DB_ENV_THREAD);

	/* Set the database home. */
	if ((ret = __db_home(dbenv, db_home, flags)) != 0)
		goto err;

	/* Parse the config array. */
	for (p = db_config; p != NULL && *p != NULL; ++p)
		if ((ret = __db_parse(dbenv, *p)) != 0)
			goto err;

	/*
	 * Parse the config file.
	 *
	 * XXX
	 * Don't use sprintf(3)/snprintf(3) -- the former is dangerous, and
	 * the latter isn't standard, and we're manipulating strings handed
	 * us by the application.
	 */
	if (dbenv->db_home != NULL) {
#define	CONFIG_NAME	"/DB_CONFIG"
		if (strlen(dbenv->db_home) +
		    strlen(CONFIG_NAME) + 1 > sizeof(buf)) {
			ret = ENAMETOOLONG;
			goto err;
		}
		(void)strcpy(buf, dbenv->db_home);
		(void)strcat(buf, CONFIG_NAME);
		if ((fp = fopen(buf, "r")) != NULL) {
			while (fgets(buf, sizeof(buf), fp) != NULL) {
				if ((lp = strchr(buf, '\n')) == NULL) {
					__db_err(dbenv,
					    "%s: line too long", CONFIG_NAME);
					ret = EINVAL;
					goto err;
				}
				*lp = '\0';
				if (buf[0] == '\0' ||
				    buf[0] == '#' || isspace(buf[0]))
					continue;

				if ((ret = __db_parse(dbenv, buf)) != 0)
					goto err;
			}
			(void)fclose(fp);
			fp = NULL;
		}
	}

	/* Set up the tmp directory path. */
	if (dbenv->db_tmp_dir == NULL && (ret = __os_tmpdir(dbenv, flags)) != 0)
		goto err;

	/*
	 * Flag that the structure has been initialized by the application.
	 * Note, this must be set before calling into the subsystems as it
	 * is used when we're doing file naming.
	 */
	F_SET(dbenv, DB_ENV_APPINIT);

	/*
	 * If we are doing recovery, remove all the old shared memory
	 * regions.
	 */
	if (LF_ISSET(DB_RECOVER | DB_RECOVER_FATAL)) {
		if ((ret = log_unlink(NULL, 1, dbenv)) != 0)
			goto err;
		if ((ret = memp_unlink(NULL, 1, dbenv)) != 0)
			goto err;
		if ((ret = lock_unlink(NULL, 1, dbenv)) != 0)
			goto err;
		if ((ret = txn_unlink(NULL, 1, dbenv)) != 0)
			goto err;
	}

	/*
	 * Create the new shared regions.
	 *
	 * Default permissions are read-write for both owner and group.
	 */
	mode = __db_omode("rwrw--");
	if (LF_ISSET(DB_INIT_CDB)) {
		if (LF_ISSET(DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_TXN)) {
			ret = EINVAL;
			goto err;
		}
		F_SET(dbenv, DB_ENV_CDB);
		dbenv->lk_conflicts = db_cdb_conflicts;
		dbenv->lk_modes = DB_LOCK_RW_N + 1;
		if ((ret = lock_open(NULL, LF_ISSET(DB_CREATE | DB_THREAD),
		    mode, dbenv, &dbenv->lk_info)) != 0)
			goto err;
	}
	if (LF_ISSET(DB_INIT_LOCK) && (ret = lock_open(NULL,
	    LF_ISSET(DB_CREATE | DB_THREAD),
	    mode, dbenv, &dbenv->lk_info)) != 0)
		goto err;
	if (LF_ISSET(DB_INIT_LOG) && (ret = log_open(NULL,
	    LF_ISSET(DB_CREATE | DB_THREAD),
	    mode, dbenv, &dbenv->lg_info)) != 0)
		goto err;
	if (LF_ISSET(DB_INIT_MPOOL) && (ret = memp_open(NULL,
	    LF_ISSET(DB_CREATE | DB_MPOOL_PRIVATE | DB_NOMMAP | DB_THREAD),
	    mode, dbenv, &dbenv->mp_info)) != 0)
		goto err;
	if (LF_ISSET(DB_INIT_TXN) && (ret = txn_open(NULL,
	    LF_ISSET(DB_CREATE | DB_THREAD | DB_TXN_NOSYNC),
	    mode, dbenv, &dbenv->tx_info)) != 0)
		goto err;

	/*
	 * If the application is running with transactions, initialize the
	 * function tables.  Once that's done, do recovery for any previous
	 * run.
	 */
	if (LF_ISSET(DB_INIT_TXN)) {
		if ((ret = __bam_init_recover(dbenv)) != 0)
			goto err;
		if ((ret = __db_init_recover(dbenv)) != 0)
			goto err;
		if ((ret = __ham_init_recover(dbenv)) != 0)
			goto err;
		if ((ret = __log_init_recover(dbenv)) != 0)
			goto err;
		if ((ret = __txn_init_recover(dbenv)) != 0)
			goto err;

		if (LF_ISSET(DB_RECOVER | DB_RECOVER_FATAL) &&
		    (ret = __db_apprec(dbenv,
		    LF_ISSET(DB_RECOVER | DB_RECOVER_FATAL))) != 0)
			goto err;
	}

	return (ret);

err:	if (fp != NULL)
		(void)fclose(fp);

	(void)db_appexit(dbenv);
	return (ret);
}

/*
 * db_appexit --
 *	Close down the default application environment.
 */
int
db_appexit(dbenv)
	DB_ENV *dbenv;
{
	int ret, t_ret;
	char **p;

	ret = 0;

	/* Close subsystems. */
	if (dbenv->tx_info && (t_ret = txn_close(dbenv->tx_info)) != 0)
		if (ret == 0)
			ret = t_ret;
	if (dbenv->lg_info && (t_ret = log_close(dbenv->lg_info)) != 0)
		if (ret == 0)
			ret = t_ret;
	if (dbenv->mp_info && (t_ret = memp_close(dbenv->mp_info)) != 0)
		if (ret == 0)
			ret = t_ret;
	if (dbenv->lk_info && (t_ret = lock_close(dbenv->lk_info)) != 0)
		if (ret == 0)
			ret = t_ret;

	/* Clear initialized flag (after subsystems, it affects naming). */
	F_CLR(dbenv, DB_ENV_APPINIT);

	/* Free allocated memory. */
	if (dbenv->db_home != NULL)
		__os_freestr(dbenv->db_home);
	if ((p = dbenv->db_data_dir) != NULL) {
		for (; *p != NULL; ++p)
			__os_freestr(*p);
		__os_free(dbenv->db_data_dir,
		    dbenv->data_cnt * sizeof(char **));
	}
	if (dbenv->db_log_dir != NULL)
		__os_freestr(dbenv->db_log_dir);
	if (dbenv->db_tmp_dir != NULL)
		__os_freestr(dbenv->db_tmp_dir);

	return (ret);
}

#define	DB_ADDSTR(str) {						\
	if ((str) != NULL) {						\
		/* If leading slash, start over. */			\
		if (__os_abspath(str)) {				\
			p = start;					\
			slash = 0;					\
		}							\
		/* Append to the current string. */			\
		len = strlen(str);					\
		if (slash)						\
			*p++ = PATH_SEPARATOR[0];			\
		memcpy(p, str, len);					\
		p += len;						\
		slash = strchr(PATH_SEPARATOR, p[-1]) == NULL;		\
	}								\
}

/*
 * __db_appname --
 *	Given an optional DB environment, directory and file name and type
 *	of call, build a path based on the db_appinit(3) rules, and return
 *	it in allocated space.
 *
 * PUBLIC: int __db_appname __P((DB_ENV *,
 * PUBLIC:    APPNAME, const char *, const char *, u_int32_t, int *, char **));
 */
int
__db_appname(dbenv, appname, dir, file, tmp_oflags, fdp, namep)
	DB_ENV *dbenv;
	APPNAME appname;
	const char *dir, *file;
	u_int32_t tmp_oflags;
	int *fdp;
	char **namep;
{
	DB_ENV etmp;
	size_t len;
	int data_entry, ret, slash, tmp_create, tmp_free;
	const char *a, *b, *c;
	char *p, *start;

	a = b = c = NULL;
	data_entry = -1;
	tmp_create = tmp_free = 0;

	/*
	 * We don't return a name when creating temporary files, just an fd.
	 * Default to error now.
	 */
	if (fdp != NULL)
		*fdp = -1;
	if (namep != NULL)
		*namep = NULL;

	/*
	 * Absolute path names are never modified.  If the file is an absolute
	 * path, we're done.  If the directory is, simply append the file and
	 * return.
	 */
	if (file != NULL && __os_abspath(file))
		return (__os_strdup(file, namep));
	if (dir != NULL && __os_abspath(dir)) {
		a = dir;
		goto done;
	}

	/*
	 * DB_ENV  DIR	   APPNAME	   RESULT
	 * -------------------------------------------
	 * null	   null	   none		   <tmp>/file
	 * null	   set	   none		   DIR/file
	 * set	   null	   none		   DB_HOME/file
	 * set	   set	   none		   DB_HOME/DIR/file
	 *
	 * DB_ENV  FILE	   APPNAME	   RESULT
	 * -------------------------------------------
	 * null	   null	   DB_APP_DATA	   <tmp>/<create>
	 * null	   set	   DB_APP_DATA	   ./file
	 * set	   null	   DB_APP_DATA	   <tmp>/<create>
	 * set	   set	   DB_APP_DATA	   DB_HOME/DB_DATA_DIR/file
	 *
	 * DB_ENV  DIR	   APPNAME	   RESULT
	 * -------------------------------------------
	 * null	   null	   DB_APP_LOG	   <tmp>/file
	 * null	   set	   DB_APP_LOG	   DIR/file
	 * set	   null	   DB_APP_LOG	   DB_HOME/DB_LOG_DIR/file
	 * set	   set	   DB_APP_LOG	   DB_HOME/DB_LOG_DIR/DIR/file
	 *
	 * DB_ENV	   APPNAME	   RESULT
	 * -------------------------------------------
	 * null		   DB_APP_TMP*	   <tmp>/<create>
	 * set		   DB_APP_TMP*	   DB_HOME/DB_TMP_DIR/<create>
	 */
retry:	switch (appname) {
	case DB_APP_NONE:
		if (dbenv == NULL || !F_ISSET(dbenv, DB_ENV_APPINIT)) {
			if (dir == NULL)
				goto tmp;
			a = dir;
		} else {
			a = dbenv->db_home;
			b = dir;
		}
		break;
	case DB_APP_DATA:
		if (dir != NULL) {
			__db_err(dbenv,
			    "DB_APP_DATA: illegal directory specification");
			return (EINVAL);
		}

		if (file == NULL) {
			tmp_create = 1;
			goto tmp;
		}
		if (dbenv == NULL || !F_ISSET(dbenv, DB_ENV_APPINIT))
			a = PATH_DOT;
		else {
			a = dbenv->db_home;
			if (dbenv->db_data_dir != NULL &&
			    (b = dbenv->db_data_dir[++data_entry]) == NULL) {
				data_entry = -1;
				b = dbenv->db_data_dir[0];
			}
		}
		break;
	case DB_APP_LOG:
		if (dbenv == NULL || !F_ISSET(dbenv, DB_ENV_APPINIT)) {
			if (dir == NULL)
				goto tmp;
			a = dir;
		} else {
			a = dbenv->db_home;
			b = dbenv->db_log_dir;
			c = dir;
		}
		break;
	case DB_APP_TMP:
		if (dir != NULL || file != NULL) {
			__db_err(dbenv,
		    "DB_APP_TMP: illegal directory or file specification");
			return (EINVAL);
		}

		tmp_create = 1;
		if (dbenv == NULL || !F_ISSET(dbenv, DB_ENV_APPINIT))
			goto tmp;
		else {
			a = dbenv->db_home;
			b = dbenv->db_tmp_dir;
		}
		break;
	}

	/* Reference a file from the appropriate temporary directory. */
	if (0) {
tmp:		if (dbenv == NULL || !F_ISSET(dbenv, DB_ENV_APPINIT)) {
			memset(&etmp, 0, sizeof(etmp));
			if ((ret = __os_tmpdir(&etmp, DB_USE_ENVIRON)) != 0)
				return (ret);
			tmp_free = 1;
			a = etmp.db_tmp_dir;
		} else
			a = dbenv->db_tmp_dir;
	}

done:	len =
	    (a == NULL ? 0 : strlen(a) + 1) +
	    (b == NULL ? 0 : strlen(b) + 1) +
	    (c == NULL ? 0 : strlen(c) + 1) +
	    (file == NULL ? 0 : strlen(file) + 1);

	/*
	 * Allocate space to hold the current path information, as well as any
	 * temporary space that we're going to need to create a temporary file
	 * name.
	 */
#define	DB_TRAIL	"XXXXXX"
	if ((ret =
	    __os_malloc(len + sizeof(DB_TRAIL) + 10, NULL, &start)) != 0) {
		if (tmp_free)
			__os_freestr(etmp.db_tmp_dir);
		return (ret);
	}

	slash = 0;
	p = start;
	DB_ADDSTR(a);
	DB_ADDSTR(b);
	DB_ADDSTR(file);
	*p = '\0';

	/* Discard any space allocated to find the temp directory. */
	if (tmp_free) {
		__os_freestr(etmp.db_tmp_dir);
		tmp_free = 0;
	}

	/*
	 * If we're opening a data file, see if it exists.  If it does,
	 * return it, otherwise, try and find another one to open.
	 */
	if (data_entry != -1 && __os_exists(start, NULL) != 0) {
		__os_freestr(start);
		a = b = c = NULL;
		goto retry;
	}

	/* Create the file if so requested. */
	if (tmp_create &&
	    (ret = __db_tmp_open(dbenv, tmp_oflags, start, fdp)) != 0) {
		__os_freestr(start);
		return (ret);
	}

	if (namep == NULL)
		__os_freestr(start);
	else
		*namep = start;
	return (0);
}

/*
 * __db_home --
 *	Find the database home.
 */
static int
__db_home(dbenv, db_home, flags)
	DB_ENV *dbenv;
	const char *db_home;
	u_int32_t flags;
{
	const char *p;

	p = db_home;

	/* Use the environment if it's permitted and initialized. */
#ifdef HAVE_GETUID
	if (LF_ISSET(DB_USE_ENVIRON) ||
	    (LF_ISSET(DB_USE_ENVIRON_ROOT) && getuid() == 0)) {
#else
	if (LF_ISSET(DB_USE_ENVIRON)) {
#endif
		if ((p = getenv("DB_HOME")) == NULL)
			p = db_home;
		else if (p[0] == '\0') {
			__db_err(dbenv,
			    "illegal DB_HOME environment variable");
			return (EINVAL);
		}
	}

	if (p == NULL)
		return (0);

	return (__os_strdup(p, &dbenv->db_home));
}

/*
 * __db_parse --
 *	Parse a single NAME VALUE pair.
 */
static int
__db_parse(dbenv, s)
	DB_ENV *dbenv;
	char *s;
{
	int ret;
	char *local_s, *name, *value, **p, *tp;

	/*
	 * We need to strdup the argument in case the caller passed us
	 * static data.
	 */
	if ((ret = __os_strdup(s, &local_s)) != 0)
		return (ret);

	/*
	 * Name/value pairs are parsed as two white-space separated strings.
	 * Leading and trailing white-space is trimmed from the value, but
	 * it may contain embedded white-space.  Note: we use the isspace(3)
	 * macro because it's more portable, but that means that you can use
	 * characters like form-feed to separate the strings.
	 */
	name = local_s;
	for (tp = name; *tp != '\0' && !isspace(*tp); ++tp)
		;
	if (*tp == '\0' || tp == name)
		goto illegal;
	*tp = '\0';
	for (++tp; isspace(*tp); ++tp)
		;
	if (*tp == '\0')
		goto illegal;
	value = tp;
	for (++tp; *tp != '\0'; ++tp)
		;
	for (--tp; isspace(*tp); --tp)
		;
	if (tp == value) {
illegal:	ret = EINVAL;
		__db_err(dbenv, "illegal name-value pair: %s", s);
		goto err;
	}
	*++tp = '\0';

#define	DATA_INIT_CNT	20			/* Start with 20 data slots. */
	if (!strcmp(name, "DB_DATA_DIR")) {
		if (dbenv->db_data_dir == NULL) {
			if ((ret = __os_calloc(DATA_INIT_CNT,
			    sizeof(char **), &dbenv->db_data_dir)) != 0)
				goto err;
			dbenv->data_cnt = DATA_INIT_CNT;
		} else if (dbenv->data_next == dbenv->data_cnt - 1) {
			dbenv->data_cnt *= 2;
			if ((ret = __os_realloc(&dbenv->db_data_dir,
			    dbenv->data_cnt * sizeof(char **))) != 0)
				goto err;
		}
		p = &dbenv->db_data_dir[dbenv->data_next++];
	} else if (!strcmp(name, "DB_LOG_DIR")) {
		if (dbenv->db_log_dir != NULL)
			__os_freestr(dbenv->db_log_dir);
		p = &dbenv->db_log_dir;
	} else if (!strcmp(name, "DB_TMP_DIR")) {
		if (dbenv->db_tmp_dir != NULL)
			__os_freestr(dbenv->db_tmp_dir);
		p = &dbenv->db_tmp_dir;
	} else
		goto err;

	ret = __os_strdup(value, p);

err:	__os_freestr(local_s);
	return (ret);
}

/*
 * __db_tmp_open --
 *	Create a temporary file.
 */
static int
__db_tmp_open(dbenv, flags, path, fdp)
	DB_ENV *dbenv;
	u_int32_t flags;
	char *path;
	int *fdp;
{
	u_long pid;
	int mode, isdir, ret;
	const char *p;
	char *trv;

	/*
	 * Check the target directory; if you have six X's and it doesn't
	 * exist, this runs for a *very* long time.
	 */
	if ((ret = __os_exists(path, &isdir)) != 0) {
		__db_err(dbenv, "%s: %s", path, strerror(ret));
		return (ret);
	}
	if (!isdir) {
		__db_err(dbenv, "%s: %s", path, strerror(EINVAL));
		return (EINVAL);
	}

	/* Build the path. */
	for (trv = path; *trv != '\0'; ++trv)
		;
	*trv = PATH_SEPARATOR[0];
	for (p = DB_TRAIL; (*++trv = *p) != '\0'; ++p)
		;

	/*
	 * Replace the X's with the process ID.  Pid should be a pid_t,
	 * but we use unsigned long for portability.
	 */
	for (pid = getpid(); *--trv == 'X'; pid /= 10)
		switch (pid % 10) {
		case 0: *trv = '0'; break;
		case 1: *trv = '1'; break;
		case 2: *trv = '2'; break;
		case 3: *trv = '3'; break;
		case 4: *trv = '4'; break;
		case 5: *trv = '5'; break;
		case 6: *trv = '6'; break;
		case 7: *trv = '7'; break;
		case 8: *trv = '8'; break;
		case 9: *trv = '9'; break;
		}
	++trv;

	/* Set up open flags and mode. */
	LF_SET(DB_CREATE | DB_EXCL);
	mode = __db_omode("rw----");

	/* Loop, trying to open a file. */
	for (;;) {
		if ((ret = __db_open(path, flags, flags, mode, fdp)) == 0)
			return (0);

		/*
		 * XXX:
		 * If we don't get an EEXIST error, then there's something
		 * seriously wrong.  Unfortunately, if the implementation
		 * doesn't return EEXIST for O_CREAT and O_EXCL regardless
		 * of other possible errors, we've lost.
		 */
		if (ret != EEXIST) {
			__db_err(dbenv,
			    "tmp_open: %s: %s", path, strerror(ret));
			return (ret);
		}

		/*
		 * Tricky little algorithm for backward compatibility.
		 * Assumes the ASCII ordering of lower-case characters.
		 */
		for (;;) {
			if (*trv == '\0')
				return (EINVAL);
			if (*trv == 'z')
				*trv++ = 'a';
			else {
				if (isdigit(*trv))
					*trv = 'a';
				else
					++*trv;
				break;
			}
		}
	}
	/* NOTREACHED */
}
