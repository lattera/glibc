/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_dir.c	10.7 (Sleepycat) 8/23/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_ext.h"
#include "common_ext.h"

/*
 * __db_dir --
 *	Return a list of the files in a directory.
 *
 * PUBLIC: int __db_dir __P((DB_ENV *, char *, char ***, int *));
 */
int
__db_dir(dbenv, dir, namesp, cntp)
	DB_ENV *dbenv;
	const char *dir;
	char ***namesp;
	int *cntp;
{
	int arraysz, cnt;
	char **names;
#ifdef _WIN32
	struct _finddata_t fdata;
	long dirhandle;
	int finished;

	if ((dirhandle = _findfirst(dir,&fdata)) == -1) {
		__db_err(dbenv, "%s: %s", dir, strerror(errno));
		return (errno);
	}

	names = NULL;
	finished = 0;
	for (arraysz = cnt = 0; finished != 1; ++cnt) {
		if (cnt >= arraysz) {
			arraysz += 100;
			names = (char **)(names == NULL ?
			    malloc(arraysz * sizeof(names[0])) :
			    realloc(names, arraysz * sizeof(names[0])));
			if (names == NULL)
				goto nomem;
		}
		if ((names[cnt] = (char *)strdup(fdata.name)) == NULL)
			goto nomem;
		if (_findnext(dirhandle,&fdata) != 0)
			finished = 1;
	}
	_findclose(dirhandle);
#else /* !_WIN32 */
	struct dirent *dp;
	DIR *dirp;

	if ((dirp = opendir(dir)) == NULL) {
		__db_err(dbenv, "%s: %s", dir, strerror(errno));
		return (errno);
	}
	names = NULL;
	for (arraysz = cnt = 0; (dp = readdir(dirp)) != NULL; ++cnt) {
		if (cnt >= arraysz) {
			arraysz += 100;
			names = (char **)(names == NULL ?
			    malloc(arraysz * sizeof(names[0])) :
			    realloc(names, arraysz * sizeof(names[0])));
			if (names == NULL)
				goto nomem;
		}
		if ((names[cnt] = (char *)strdup(dp->d_name)) == NULL)
			goto nomem;
	}
	(void)closedir(dirp);
#endif /* !_WIN32 */

	*namesp = names;
	*cntp = cnt;
	return (0);

nomem:	if (names != NULL)
		__db_dirf(dbenv, names, cnt);
	__db_err(dbenv, "%s", strerror(ENOMEM));
	return (ENOMEM);
}

/*
 * __db_dirf --
 *	Free the list of files.
 *
 * PUBLIC: void __db_dirf __P((DB_ENV *, char **, int));
 */
void
__db_dirf(dbenv, names, cnt)
	DB_ENV *dbenv;
	char **names;
	int cnt;
{
	dbenv = dbenv;			/* XXX: Shut the compiler up. */
	while (cnt > 0)
		free(names[--cnt]);
	free (names);
}
