/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_dir.c	10.19 (Sleepycat) 10/12/98";
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
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * __os_dirlist --
 *	Return a list of the files in a directory.
 *
 * PUBLIC: int __os_dirlist __P((const char *, char ***, int *));
 */
int
__os_dirlist(dir, namesp, cntp)
	const char *dir;
	char ***namesp;
	int *cntp;
{
	struct dirent *dp;
	DIR *dirp;
	int arraysz, cnt, ret;
	char **names;

	if (__db_jump.j_dirlist != NULL)
		return (__db_jump.j_dirlist(dir, namesp, cntp));

	if ((dirp = opendir(dir)) == NULL)
		return (errno);
	names = NULL;
	for (arraysz = cnt = 0; (dp = readdir(dirp)) != NULL; ++cnt) {
		if (cnt >= arraysz) {
			arraysz += 100;
			if ((ret = __os_realloc(&names,
			    arraysz * sizeof(names[0]))) != 0)
				goto nomem;
		}
		if ((ret = __os_strdup(dp->d_name, &names[cnt])) != 0)
			goto nomem;
	}
	(void)closedir(dirp);

	*namesp = names;
	*cntp = cnt;
	return (0);

nomem:	if (names != NULL)
		__os_dirfree(names, cnt);
	return (ret);
}

/*
 * __os_dirfree --
 *	Free the list of files.
 *
 * PUBLIC: void __os_dirfree __P((char **, int));
 */
void
__os_dirfree(names, cnt)
	char **names;
	int cnt;
{
	if (__db_jump.j_dirfree != NULL)
		__db_jump.j_dirfree(names, cnt);

	while (cnt > 0)
		__os_free(names[--cnt], 0);
	__os_free(names, 0);
}
