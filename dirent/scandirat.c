/* Copyright (C) 1992-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* We need to avoid the header declaration of scandir64, because
   the types don't match scandir and then the compiler will
   complain about the mismatch when we do the alias below.  */
#define scandirat64       __renamed_scandirat64

#include <dirent.h>

#undef  scandirat64

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <bits/libc-lock.h>

#ifndef SCANDIRAT
# define SCANDIRAT scandirat
# define READDIR __readdir
# define DIRENT_TYPE struct dirent
#endif

#ifndef SKIP_SCANDIR_CANCEL
void
__scandir_cancel_handler (void *arg)
{
  struct scandir_cancel_struct *cp = arg;
  size_t i;
  void **v = cp->v;

  for (i = 0; i < cp->cnt; ++i)
    free (v[i]);
  free (v);
  (void) __closedir (cp->dp);
}
#endif


int
SCANDIRAT (dfd, dir, namelist, select, cmp)
     int dfd;
     const char *dir;
     DIRENT_TYPE ***namelist;
     int (*select) (const DIRENT_TYPE *);
     int (*cmp) (const DIRENT_TYPE **, const DIRENT_TYPE **);
{
  DIR *dp = __opendirat (dfd, dir);
  DIRENT_TYPE **v = NULL;
  size_t vsize = 0;
  struct scandir_cancel_struct c;
  DIRENT_TYPE *d;
  int save;

  if (dp == NULL)
    return -1;

  save = errno;
  __set_errno (0);

  c.dp = dp;
  c.v = NULL;
  c.cnt = 0;
  __libc_cleanup_push (__scandir_cancel_handler, &c);

  while ((d = READDIR (dp)) != NULL)
    {
      int use_it = select == NULL;

      if (! use_it)
	{
	  use_it = select (d);
	  /* The select function might have changed errno.  It was
	     zero before and it need to be again to make the latter
	     tests work.  */
	  __set_errno (0);
	}

      if (use_it)
	{
	  DIRENT_TYPE *vnew;
	  size_t dsize;

	  /* Ignore errors from select or readdir */
	  __set_errno (0);

	  if (__builtin_expect (c.cnt == vsize, 0))
	    {
	      DIRENT_TYPE **new;
	      if (vsize == 0)
		vsize = 10;
	      else
		vsize *= 2;
	      new = (DIRENT_TYPE **) realloc (v, vsize * sizeof (*v));
	      if (new == NULL)
		break;
	      v = new;
	      c.v = (void *) v;
	    }

	  dsize = &d->d_name[_D_ALLOC_NAMLEN (d)] - (char *) d;
	  vnew = (DIRENT_TYPE *) malloc (dsize);
	  if (vnew == NULL)
	    break;

	  v[c.cnt++] = (DIRENT_TYPE *) memcpy (vnew, d, dsize);
	}
    }

  if (__builtin_expect (errno, 0) != 0)
    {
      save = errno;

      while (c.cnt > 0)
	free (v[--c.cnt]);
      free (v);
      c.cnt = -1;
    }
  else
    {
      /* Sort the list if we have a comparison function to sort with.  */
      if (cmp != NULL)
	qsort (v, c.cnt, sizeof (*v),
	       (int (*) (const void *, const void *)) cmp);

      *namelist = v;
    }

  __libc_cleanup_pop (0);

  (void) __closedir (dp);
  __set_errno (save);

  return c.cnt;
}
libc_hidden_def (SCANDIRAT)

#ifdef _DIRENT_MATCHES_DIRENT64
weak_alias (scandirat, scandirat64)
#endif
