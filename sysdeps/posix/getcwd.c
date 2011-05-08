/* Copyright (C) 1991,92,93,94,95,96,97,98,99,11 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Wants:
   AC_STDC_HEADERS
   AC_DIR_HEADER
   AC_UNISTD_H
   AC_MEMORY_H
   AC_CONST
   AC_ALLOCA
 */

/* AIX requires this to be the first thing in the file.  */
#if defined _AIX && !defined __GNUC__
 #pragma alloca
#endif

#ifdef	HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef	STDC_HEADERS
# include <stddef.h>
#endif

#if !defined __GNU_LIBRARY__ && !defined STDC_HEADERS
extern int errno;
#endif
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#ifndef	NULL
# define NULL	0
#endif

#if defined USGr3 && !defined DIRENT
# define DIRENT
#endif /* USGr3 */
#if defined Xenix && !defined SYSNDIR
# define SYSNDIR
#endif /* Xenix */

#if defined POSIX || defined DIRENT || defined __GNU_LIBRARY__
# include <dirent.h>
# ifndef __GNU_LIBRARY__
#  define D_NAMLEN(d) strlen((d)->d_name)
# else
#  define HAVE_D_NAMLEN
#  define D_NAMLEN(d) ((d)->d_namlen)
# endif
#else /* not POSIX or DIRENT */
# define dirent		direct
# define D_NAMLEN(d)	((d)->d_namlen)
# define HAVE_D_NAMLEN
# if defined USG && !defined sgi
#  if defined SYSNDIR
#   include <sys/ndir.h>
#  else /* Not SYSNDIR */
#   include "ndir.h"
#  endif /* SYSNDIR */
# else /* not USG */
#  include <sys/dir.h>
# endif /* USG */
#endif /* POSIX or DIRENT or __GNU_LIBRARY__ */

#if defined HAVE_UNISTD_H || defined __GNU_LIBRARY__
# include <unistd.h>
#endif

#if defined STDC_HEADERS || defined __GNU_LIBRARY__ || defined POSIX
# include <stdlib.h>
# include <string.h>
# define ANSI_STRING
#else	/* No standard headers.  */

# ifdef	USG

#  include <string.h>
#  ifdef NEED_MEMORY_H
#   include <memory.h>
#  endif
#  define	ANSI_STRING

# else	/* Not USG.  */

#  ifdef NeXT

#   include <string.h>

#  else	/* Not NeXT.  */

#   include <strings.h>

#   ifndef bcmp
extern int bcmp ();
#   endif
#   ifndef bzero
extern void bzero ();
#   endif
#   ifndef bcopy
extern void bcopy ();
#   endif

#  endif /* NeXT. */

# endif	/* USG.  */

extern char *malloc (), *realloc ();
extern void free ();

#endif /* Standard headers.  */

#ifndef	ANSI_STRING
# define memcpy(d, s, n)	bcopy((s), (d), (n))
# define memmove memcpy
#endif	/* Not ANSI_STRING.  */

#ifndef MAX
# define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifdef _LIBC
# ifndef mempcpy
#  define mempcpy __mempcpy
# endif
# define HAVE_MEMPCPY	1
#endif

#if !defined __alloca && !defined __GNU_LIBRARY__

# ifdef	__GNUC__
#  undef alloca
#  define alloca(n)	__builtin_alloca (n)
# else	/* Not GCC.  */
#  if	defined sparc || defined HAVE_ALLOCA_H
#   include <alloca.h>
#  else	/* Not sparc or HAVE_ALLOCA_H.  */
#   ifndef _AIX
extern char *alloca ();
#   endif /* Not _AIX.  */
#  endif /* sparc or HAVE_ALLOCA_H.  */
# endif	/* GCC.  */

# define __alloca	alloca

#endif

#if defined HAVE_LIMITS_H || defined STDC_HEADERS || defined __GNU_LIBRARY__
# include <limits.h>
#else
# include <sys/param.h>
#endif

#if defined _LIBC
# include <not-cancel.h>
#else
# define openat64_not_cancel_3(dfd, name, mode) openat64 (dfd, name, mode)
# define close_not_cancel_no_status(fd) close (fd)
#endif

#ifndef PATH_MAX
# ifdef	MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else
#  define PATH_MAX 1024
# endif
#endif

#if !defined STDC_HEADERS && !defined __GNU_LIBRARY__
# undef	size_t
# define size_t	unsigned int
#endif

#if !__STDC__ && !defined const
# define const
#endif

#ifndef __GNU_LIBRARY__
# define __lstat64	stat64
#endif

#ifndef _LIBC
# define __getcwd getcwd
#endif

#ifndef GETCWD_RETURN_TYPE
# define GETCWD_RETURN_TYPE char *
#endif

#ifdef __ASSUME_ATFCTS
# define __have_atfcts 1
#elif defined NOT_IN_libc && defined IS_IN_rtld
static int __rtld_have_atfcts;
# define __have_atfcts __rtld_have_atfcts
#endif

/* Get the pathname of the current working directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined or
   SIZE was too small.  If successful, returns BUF.  In GNU, if BUF is
   NULL, an array is allocated with `malloc'; the array is SIZE bytes long,
   unless SIZE == 0, in which case it is as big as necessary.  */

GETCWD_RETURN_TYPE
__getcwd (buf, size)
     char *buf;
     size_t size;
{
#ifndef __ASSUME_ATFCTS
  static const char dots[]
    = "../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../..";
  const char *dotp = &dots[sizeof (dots)];
  const char *dotlist = dots;
  size_t dotsize = sizeof (dots) - 1;
#endif
  int prev_errno = errno;
  DIR *dirstream = NULL;
  bool fd_needs_closing = false;
  int fd = AT_FDCWD;

  char *path;
#ifndef NO_ALLOCATION
  size_t allocated = size;
  if (size == 0)
    {
      if (buf != NULL)
	{
	  __set_errno (EINVAL);
	  return NULL;
	}

      allocated = PATH_MAX + 1;
    }

  if (buf == NULL)
    {
      path = malloc (allocated);
      if (path == NULL)
	return NULL;
    }
  else
#else
# define allocated size
#endif
    path = buf;

  char *pathp = path + allocated;
  *--pathp = '\0';

  struct stat64 st;
  if (__lstat64 (".", &st) < 0)
    goto lose;
  dev_t thisdev = st.st_dev;
  ino_t thisino = st.st_ino;

  if (__lstat64 ("/", &st) < 0)
    goto lose;
  dev_t rootdev = st.st_dev;
  ino_t rootino = st.st_ino;

  while (!(thisdev == rootdev && thisino == rootino))
    {
      if (__have_atfcts >= 0)
	{
	  int mode = O_RDONLY;
#ifdef O_CLOEXEC
	  mode |= O_CLOEXEC;
#endif
	  fd = openat64_not_cancel_3 (fd, "..", mode);
	}
      else
	fd = -1;
      if (fd >= 0)
	{
	  fd_needs_closing = true;
	  if (__fstat64 (fd, &st) < 0)
	    goto lose;
	}
#ifndef __ASSUME_ATFCTS
      else if (errno == ENOSYS)
	{
	  __have_atfcts = -1;

	  /* Look at the parent directory.  */
	  if (dotp == dotlist)
	    {
# ifdef NO_ALLOCATION
	      __set_errno (ENOMEM);
	      goto lose;
# else
	      /* My, what a deep directory tree you have, Grandma.  */
	      char *new;
	      if (dotlist == dots)
		{
		  new = malloc (dotsize * 2 + 1);
		  if (new == NULL)
		    goto lose;
#  ifdef HAVE_MEMPCPY
		  dotp = mempcpy (new, dots, dotsize);
#  else
		  memcpy (new, dots, dotsize);
		  dotp = &new[dotsize];
#  endif
		}
	      else
		{
		  new = realloc ((__ptr_t) dotlist, dotsize * 2 + 1);
		  if (new == NULL)
		    goto lose;
		  dotp = &new[dotsize];
		}
#  ifdef HAVE_MEMPCPY
	      *((char *) mempcpy ((char *) dotp, new, dotsize)) = '\0';
	      dotsize *= 2;
#  else
	      memcpy ((char *) dotp, new, dotsize);
	      dotsize *= 2;
	      new[dotsize] = '\0';
#  endif
	      dotlist = new;
# endif
	    }

	  dotp -= 3;

	  /* Figure out if this directory is a mount point.  */
	  if (__lstat64 (dotp, &st) < 0)
	    goto lose;
	}
#endif
      else
	goto lose;

      if (dirstream && __closedir (dirstream) != 0)
	{
	  dirstream = NULL;
	  goto lose;
       }

      dev_t dotdev = st.st_dev;
      ino_t dotino = st.st_ino;
      bool mount_point = dotdev != thisdev;

      /* Search for the last directory.  */
      if (__have_atfcts >= 0)
	dirstream = __fdopendir (fd);
#ifndef __ASSUME_ATFCTS
      else
	dirstream = __opendir (dotp);
#endif
      if (dirstream == NULL)
	goto lose;
      fd_needs_closing = false;

      struct dirent *d;
      bool use_d_ino = true;
      while (1)
	{
	  /* Clear errno to distinguish EOF from error if readdir returns
	     NULL.  */
	  __set_errno (0);
	  d = __readdir (dirstream);
	  if (d == NULL)
	    {
	      if (errno == 0)
		{
		  /* When we've iterated through all directory entries
		     without finding one with a matching d_ino, rewind the
		     stream and consider each name again, but this time, using
		     lstat64.  This is necessary in a chroot on at least one
		     system.  */
		  if (use_d_ino)
		    {
		      use_d_ino = false;
		      rewinddir (dirstream);
		      continue;
		    }

		  /* EOF on dirstream, which means that the current directory
		     has been removed.  */
		  __set_errno (ENOENT);
		}
	      goto lose;
	    }

	  if (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
	    continue;
	  if (d->d_name[0] == '.'
	      && (d->d_name[1] == '\0'
		  || (d->d_name[1] == '.' && d->d_name[2] == '\0')))
	    continue;
	  if (use_d_ino && !mount_point && (ino_t) d->d_ino != thisino)
	    continue;

	  if (__have_atfcts >= 0)
	    {
	      /* We don't fail here if we cannot stat64() a directory entry.
		 This can happen when (network) filesystems fail.  If this
		 entry is in fact the one we are looking for we will find
		 out soon as we reach the end of the directory without
		 having found anything.  */
	      if (__fstatat64 (fd, d->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0)
		continue;
	    }
#ifndef __ASSUME_ATFCTS
	  else
	    {
	      char name[dotlist + dotsize - dotp + 1 + _D_ALLOC_NAMLEN (d)];
# ifdef HAVE_MEMPCPY
	      char *tmp = mempcpy (name, dotp, dotlist + dotsize - dotp);
	      *tmp++ = '/';
	      strcpy (tmp, d->d_name);
# else
	      memcpy (name, dotp, dotlist + dotsize - dotp);
	      name[dotlist + dotsize - dotp] = '/';
	      strcpy (&name[dotlist + dotsize - dotp + 1], d->d_name);
# endif
	      /* We don't fail here if we cannot stat64() a directory entry.
		 This can happen when (network) filesystems fail.  If this
		 entry is in fact the one we are looking for we will find
		 out soon as we reach the end of the directory without
		 having found anything.  */
	      if (__lstat64 (name, &st) < 0)
		continue;
	    }
#endif
	  if (S_ISDIR (st.st_mode)
	      && st.st_dev == thisdev && st.st_ino == thisino)
	    break;
	}

      size_t namlen = _D_EXACT_NAMLEN (d);

      if ((size_t) (pathp - path) <= namlen)
	{
#ifndef NO_ALLOCATION
	  if (size == 0)
	    {
	      size_t oldsize = allocated;

	      allocated = 2 * MAX (allocated, namlen);
	      char *tmp = realloc (path, allocated);
	      if (tmp == NULL)
		goto lose;

	      /* Move current contents up to the end of the buffer.
		 This is guaranteed to be non-overlapping.  */
	      pathp = memcpy (tmp + allocated - (path + oldsize - pathp),
			      tmp + (pathp - path),
			      path + oldsize - pathp);
	      path = tmp;
	    }
	  else
#endif
	    {
	      __set_errno (ERANGE);
	      goto lose;
	    }
	}
      pathp -= namlen;
      (void) memcpy (pathp, d->d_name, namlen);
      *--pathp = '/';

      thisdev = dotdev;
      thisino = dotino;
    }

  if (dirstream != NULL && __closedir (dirstream) != 0)
    {
      dirstream = NULL;
      goto lose;
    }

  if (pathp == &path[allocated - 1])
    *--pathp = '/';

#ifndef __ASSUME_ATFCTS
  if (dotlist != dots)
    free ((__ptr_t) dotlist);
#endif

  size_t used = path + allocated - pathp;
  memmove (path, pathp, used);

  if (size == 0)
    /* Ensure that the buffer is only as large as necessary.  */
    buf = realloc (path, used);

  if (buf == NULL)
    /* Either buf was NULL all along, or `realloc' failed but
       we still have the original string.  */
    buf = path;

  /* Restore errno on successful return.  */
  __set_errno (prev_errno);

  return buf;

 lose:;
  int save_errno = errno;
#ifndef __ASSUME_ATFCTS
  if (dotlist != dots)
    free ((__ptr_t) dotlist);
#endif
  if (dirstream != NULL)
    __closedir (dirstream);
  if (fd_needs_closing)
    close_not_cancel_no_status (fd);
#ifndef NO_ALLOCATION
  if (buf == NULL)
    free (path);
#endif
  __set_errno (save_errno);
  return NULL;
}

#if defined _LIBC && !defined __getcwd
weak_alias (__getcwd, getcwd)
#endif
