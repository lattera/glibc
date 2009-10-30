/* Copyright (C) 2005, 2006, 2009 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <kernel-features.h>
#include <sysdep.h>


#ifndef __ASSUME_ATFCTS
void
attribute_hidden
__atfct_seterrno_2 (int errval, int fd1, const char *buf1, int fd2,
		    const char *buf2)
{
  if (buf1 != NULL || buf2 != NULL)
    {
      struct stat64 st;

      if (errval == ENOTDIR)
	{
	  /* This can mean either the file descriptor is invalid or
	     /proc is not mounted.  */
	  if (buf1 != NULL)
	    {
	      if (__fxstat64 (_STAT_VER, fd1, &st) != 0)
		/* errno is already set correctly.  */
		return;

	      /* If /proc is not mounted there is nothing we can do.  */
	      if (S_ISDIR (st.st_mode)
		  && (__xstat64 (_STAT_VER, "/proc/self/fd", &st) != 0
		      || !S_ISDIR (st.st_mode)))
		{
		  errval = ENOSYS;
		  goto out;
		}
	    }

	  if (buf2 != NULL)
	    {
	      if (__fxstat64 (_STAT_VER, fd2, &st) != 0)
		/* errno is already set correctly.  */
		return;

	      /* If /proc is not mounted there is nothing we can do.  */
	      if (S_ISDIR (st.st_mode)
		  && (__xstat64 (_STAT_VER, "/proc/self/fd", &st) != 0
		      || !S_ISDIR (st.st_mode)))
		errval = ENOSYS;
	    }
	}
      else if (errval == ENOENT)
	{
	  /* This could mean the file descriptor is not valid.  We
	     reuse BUF for the stat call.  Find the slash after the
	     file descriptor number.  */
	  if (buf1 != NULL)
	    {
	      *(char *) strchr (buf1 + sizeof "/proc/self/fd", '/') = '\0';

	      int e = __lxstat64 (_STAT_VER, buf1, &st);
	      if ((e == -1 && errno == ENOENT)
		  ||(e == 0 && !S_ISLNK (st.st_mode)))
		{
		  errval = EBADF;
		  goto out;
		}
	    }

	  if (buf2 != NULL)
	    {
	      *(char *) strchr (buf2 + sizeof "/proc/self/fd", '/') = '\0';

	      int e = __lxstat64 (_STAT_VER, buf2, &st);
	      if ((e == -1 && errno == ENOENT)
		  ||(e == 0 && !S_ISLNK (st.st_mode)))
		errval = EBADF;
	    }
	}
    }

 out:
  __set_errno (errval);
}
#endif


/* Rename the file OLD relative to OLDFD to NEW relative to NEWFD.  */
int
renameat (oldfd, old, newfd, new)
     int oldfd;
     const char *old;
     int newfd;
     const char *new;
{
  int result;

#ifdef __NR_renameat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      result = INLINE_SYSCALL (renameat, 4, oldfd, old, newfd, new);
# ifndef __ASSUME_ATFCTS
      if (result == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return result;
    }
#endif

#ifndef __ASSUME_ATFCTS
  static const char procfd[] = "/proc/self/fd/%d/%s";
  char *bufold = NULL;

  if (oldfd != AT_FDCWD && old[0] != '/')
    {
      size_t filelen = strlen (old);
      if (__builtin_expect (filelen == 0, 0))
	{
	  __set_errno (ENOENT);
	  return -1;
	}

      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      bufold = alloca (buflen);

      __snprintf (bufold, buflen, procfd, oldfd, old);
      old = bufold;
    }

  char *bufnew = NULL;

  if (newfd != AT_FDCWD && new[0] != '/')
    {
      size_t filelen = strlen (new);
      if (__builtin_expect (filelen == 0, 0))
	{
	  __set_errno (ENOENT);
	  return -1;
	}

      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      bufnew = alloca (buflen);

      __snprintf (bufnew, buflen, procfd, newfd, new);
      new = bufnew;
    }

  INTERNAL_SYSCALL_DECL (err);

  result = INTERNAL_SYSCALL (rename, err, 2, old,  new);

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 0))
    {
      __atfct_seterrno_2 (INTERNAL_SYSCALL_ERRNO (result, err), newfd, bufnew,
			  oldfd, bufold);
      result = -1;
    }

  return result;
#endif
}
