/* Copyright (C) 1998 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This file contains only wrappers around the real glob functions.  It
   became necessary since the glob_t structure changed.  */
#include <sys/types.h>
#include <glob.h>

#if defined PIC && DO_VERSIONING

/* This is the old structure.  The difference is that the gl_pathc and
   gl_offs elements have type `int'.  */
typedef struct
  {
    int gl_pathc;		/* Count of paths matched by the pattern.  */
    char **gl_pathv;		/* List of matched pathnames.  */
    int gl_offs;		/* Slots to reserve in `gl_pathv'.  */
    int gl_flags;		/* Set to FLAGS, maybe | GLOB_MAGCHAR.  */

    /* If the GLOB_ALTDIRFUNC flag is set, the following functions
       are used instead of the normal file access functions.  */
    void (*gl_closedir) __P ((void *));
    struct dirent *(*gl_readdir) __P ((void *));
    __ptr_t (*gl_opendir) __P ((__const char *));
    int (*gl_lstat) __P ((__const char *, struct stat *));
    int (*gl_stat) __P ((__const char *, struct stat *));
  } old_glob_t;


int
__old_glob (const char *pattern, int flags,
	    int (*errfunc) __P ((const char *, int)),
	    old_glob_t *pglob)
{
  glob_t correct;
  int result;

  /* Construct an object of correct type.  */
  correct.gl_pathc = pglob->gl_pathc;
  correct.gl_pathv = pglob->gl_pathv;
  correct.gl_offs = pglob->gl_offs;
  correct.gl_flags = pglob->gl_flags;
  correct.gl_closedir = pglob->gl_closedir;
  correct.gl_readdir = pglob->gl_readdir;
  correct.gl_opendir = pglob->gl_opendir;
  correct.gl_lstat = pglob->gl_lstat;
  correct.gl_stat = pglob->gl_stat;

  result = glob (pattern, flags, errfunc, &correct);

  /* And convert it back.  */
  pglob->gl_pathc = correct.gl_pathc;
  pglob->gl_pathv = correct.gl_pathv;
  pglob->gl_offs = correct.gl_offs;
  pglob->gl_flags = correct.gl_flags;
  pglob->gl_closedir = correct.gl_closedir;
  pglob->gl_readdir = correct.gl_readdir;
  pglob->gl_opendir = correct.gl_opendir;
  pglob->gl_lstat = correct.gl_lstat;
  pglob->gl_stat = correct.gl_stat;

  return result;
}
symbol_version(__old_glob, glob, GLIBC_2.0);


/* Free storage allocated in PGLOB by a previous `glob' call.  */
void
__old_globfree (old_glob_t *pglob)
{
  glob_t correct;

  /* We only need these two symbols.  */
  correct.gl_pathc = pglob->gl_pathc;
  correct.gl_pathv = pglob->gl_pathv;

  globfree (&correct);
}
symbol_version(__old_globfree, globfree, GLIBC_2.0);

#endif
