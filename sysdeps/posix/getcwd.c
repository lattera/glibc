/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* Wants:
   AC_STDC_HEADERS
   AC_DIR_HEADER
   AC_UNISTD_H
   AC_MEMORY_H
   AC_CONST
   AC_ALLOCA
 */

/* AIX requires this to be the first thing in the file.  */
#if defined (_AIX) && !defined (__GNUC__)
 #pragma alloca
#endif

#ifdef	HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef	STDC_HEADERS
#include <stddef.h>
#endif

#if !defined(__GNU_LIBRARY__) && !defined(STDC_HEADERS)
extern int errno;
#endif

#ifndef	NULL
#define	NULL	0
#endif

#if defined (USGr3) && !defined (DIRENT)
#define DIRENT
#endif /* USGr3 */
#if defined (Xenix) && !defined (SYSNDIR)
#define SYSNDIR
#endif /* Xenix */

#if defined (POSIX) || defined (DIRENT) || defined (__GNU_LIBRARY__)
#include <dirent.h>
#ifndef	__GNU_LIBRARY__
#define D_NAMLEN(d) strlen((d)->d_name)
#else
#define	HAVE_D_NAMLEN
#define D_NAMLEN(d) ((d)->d_namlen)
#endif
#else /* not POSIX or DIRENT */
#define	dirent		direct
#define D_NAMLEN(d)	((d)->d_namlen)
#define	HAVE_D_NAMLEN
#if defined (USG) && !defined (sgi)
#if defined (SYSNDIR)
#include <sys/ndir.h>
#else /* Not SYSNDIR */
#include "ndir.h"
#endif /* SYSNDIR */
#else /* not USG */
#include <sys/dir.h>
#endif /* USG */
#endif /* POSIX or DIRENT or __GNU_LIBRARY__ */

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif

#if	(defined (STDC_HEADERS) || defined (__GNU_LIBRARY__) \
	 || defined (POSIX))
#include <stdlib.h>
#include <string.h>
#define	ANSI_STRING
#else	/* No standard headers.  */

#ifdef	USG

#include <string.h>
#ifdef	NEED_MEMORY_H
#include <memory.h>
#endif
#define	ANSI_STRING

#else	/* Not USG.  */

#ifdef	NeXT

#include <string.h>

#else	/* Not NeXT.  */

#include <strings.h>

#ifndef	bcmp
extern int bcmp ();
#endif
#ifndef	bzero
extern void bzero ();
#endif
#ifndef	bcopy
extern void bcopy ();
#endif

#endif	/* NeXT. */

#endif	/* USG.  */

extern char *malloc (), *realloc ();
extern void free ();

#endif /* Standard headers.  */

#ifndef	ANSI_STRING
#define	memcpy(d, s, n)	bcopy((s), (d), (n))
#define	memmove memcpy
#endif	/* Not ANSI_STRING.  */

#if	!defined(__alloca) && !defined(__GNU_LIBRARY__)

#ifdef	__GNUC__
#undef	alloca
#define	alloca(n)	__builtin_alloca (n)
#else	/* Not GCC.  */
#if	defined (sparc) || defined (HAVE_ALLOCA_H)
#include <alloca.h>
#else	/* Not sparc or HAVE_ALLOCA_H.  */
#ifndef	_AIX
extern char *alloca ();
#endif	/* Not _AIX.  */
#endif	/* sparc or HAVE_ALLOCA_H.  */
#endif	/* GCC.  */

#define	__alloca	alloca

#endif

#if (defined (HAVE_LIMITS_H) || defined (STDC_HEADERS) || \
     defined (__GNU_LIBRARY__))
#include <limits.h>
#else
#include <sys/param.h>
#endif

#ifndef PATH_MAX
#ifdef	MAXPATHLEN
#define	PATH_MAX MAXPATHLEN
#else
#define	PATH_MAX 1024
#endif
#endif

#ifndef	STDC_HEADERS
#undef	size_t
#define	size_t	unsigned int
#endif

#if !__STDC__ && !defined (const)
#define const
#endif

#ifndef __GNU_LIBRARY__
#define	__lstat	stat
#endif

/* Get the pathname of the current working directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined or
   SIZE was too small.  If successful, returns BUF.  In GNU, if BUF is
   NULL, an array is allocated with `malloc'; the array is SIZE bytes long,
   unless SIZE <= 0, in which case it is as big as necessary.  */

char *
getcwd (buf, size)
     char *buf;
     size_t size;
{
  static const char dots[]
    = "../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../..";
  const char *dotp, *dotlist;
  size_t dotsize;
  dev_t rootdev, thisdev;
  ino_t rootino, thisino;
  char *path;
  register char *pathp;
  struct stat st;

  if (size == 0)
    {
      if (buf != NULL)
	{
	  errno = EINVAL;
	  return NULL;
	}

      size = PATH_MAX + 1;
    }

  if (buf != NULL)
    path = buf;
  else
    {
      path = malloc (size);
      if (path == NULL)
	return NULL;
    }

  pathp = path + size;
  *--pathp = '\0';

  if (__lstat (".", &st) < 0)
    return NULL;
  thisdev = st.st_dev;
  thisino = st.st_ino;

  if (__lstat ("/", &st) < 0)
    return NULL;
  rootdev = st.st_dev;
  rootino = st.st_ino;

  dotsize = sizeof (dots) - 1;
  dotp = &dots[sizeof (dots)];
  dotlist = dots;
  while (!(thisdev == rootdev && thisino == rootino))
    {
      register DIR *dirstream;
      register struct dirent *d;
      dev_t dotdev;
      ino_t dotino;
      char mount_point;

      /* Look at the parent directory.  */
      if (dotp == dotlist)
	{
	  /* My, what a deep directory tree you have, Grandma.  */
	  char *new;
	  if (dotlist == dots)
	    {
	      new = malloc (dotsize * 2 + 1);
	      if (new == NULL)
		return NULL;
	      memcpy (new, dots, dotsize);
	    }
	  else
	    {
	      new = realloc ((__ptr_t) dotlist, dotsize * 2 + 1);
	      if (new == NULL)
		goto lose;
	    }
	  memcpy (&new[dotsize], new, dotsize);
	  dotp = &new[dotsize];
	  dotsize *= 2;
	  new[dotsize] = '\0';
	  dotlist = new;
	}

      dotp -= 3;

      /* Figure out if this directory is a mount point.  */
      if (__lstat (dotp, &st) < 0)
	goto lose;
      dotdev = st.st_dev;
      dotino = st.st_ino;
      mount_point = dotdev != thisdev;

      /* Search for the last directory.  */
      dirstream = opendir (dotp);
      if (dirstream == NULL)
	goto lose;
      while ((d = readdir (dirstream)) != NULL)
	{
	  if (d->d_name[0] == '.' &&
	      (d->d_namlen == 1 || (d->d_namlen == 2 && d->d_name[1] == '.')))
	    continue;
	  if (mount_point || d->d_ino == thisino)
	    {
	      char *name = __alloca (dotlist + dotsize - dotp +
				     1 + d->d_namlen + 1);
	      memcpy (name, dotp, dotlist + dotsize - dotp);
	      name[dotlist + dotsize - dotp] = '/';
	      memcpy (&name[dotlist + dotsize - dotp + 1],
		      d->d_name, d->d_namlen + 1);
	      if (__lstat (name, &st) < 0)
		{
		  int save = errno;
		  (void) closedir (dirstream);
		  errno = save;
		  goto lose;
		}
	      if (st.st_dev == thisdev && st.st_ino == thisino)
		break;
	    }
	}
      if (d == NULL)
	{
	  int save = errno;
	  (void) closedir (dirstream);
	  errno = save;
	  goto lose;
	}
      else
	{
	  if (pathp - path < d->d_namlen + 1)
	    {
	      if (buf != NULL)
		{
		  errno = ERANGE;
		  return NULL;
		}
	      else
		{
		  size *= 2;
		  buf = realloc (path, size);
		  if (buf == NULL)
		    {
		      (void) closedir (dirstream);
		      free (path);
		      errno = ENOMEM; /* closedir might have changed it.  */
		      return NULL;
		    }
		  pathp = &buf[pathp - path];
		  path = buf;
		}
	    }
	  pathp -= d->d_namlen;
	  (void) memcpy (pathp, d->d_name, d->d_namlen);
	  *--pathp = '/';
	  (void) closedir (dirstream);
	}

      thisdev = dotdev;
      thisino = dotino;
    }

  if (pathp == &path[size - 1])
    *--pathp = '/';

  if (dotlist != dots)
    free ((__ptr_t) dotlist);

  memmove (path, pathp, path + size - pathp);
  return path;

 lose:
  if (dotlist != dots)
    free ((__ptr_t) dotlist);
  return NULL;
}
