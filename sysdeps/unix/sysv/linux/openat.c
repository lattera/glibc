/* Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <kernel-features.h>
#include <sysdep-cancel.h>


#if !defined OPENAT && !defined __ASSUME_ATFCTS
# define OPENAT openat


void
attribute_hidden
__atfct_seterrno (int errval, int fd, const char *buf)
{
  if (buf != NULL)
    {
      struct stat64 st;

      if (errval == ENOTDIR)
	{
	  /* This can mean either the file descriptor is invalid or
	     /proc is not mounted.  */
	  if (__fxstat64 (_STAT_VER, fd, &st) != 0)
	    /* errno is already set correctly.  */
	    return;

	  /* If /proc is not mounted there is nothing we can do.  */
	  if (S_ISDIR (st.st_mode)
	      && (__xstat64 (_STAT_VER, "/proc/self/fd", &st) != 0
		  || !S_ISDIR (st.st_mode)))
	    errval = ENOSYS;
	}
      else if (errval == ENOENT)
	{
	  /* This could mean the file descriptor is not valid.  We
	     reuse BUF for the stat call.  Find the slash after the
	     file descriptor number.  */
	  *(char *) strchr (buf + sizeof "/proc/self/fd", '/') = '\0';

	  int e = __lxstat64 (_STAT_VER, buf, &st);
	  if ((e == -1 && errno == ENOENT)
	      ||(e == 0 && !S_ISLNK (st.st_mode)))
	    errval = EBADF;
	}
    }

  __set_errno (errval);
}

int __have_atfcts;
#endif

/* Open FILE with access OFLAG.  Interpret relative paths relative to
   the directory associated with FD.  If OFLAG includes O_CREAT, a
   third argument is the file protection.  */
int
OPENAT (fd, file, oflag)
     int fd;
     const char *file;
     int oflag;
{
  mode_t mode = 0;
  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
    }

  /* We have to add the O_LARGEFILE flag for openat64.  */
#ifdef MORE_OFLAGS
  oflag |= MORE_OFLAGS;
#endif

  INTERNAL_SYSCALL_DECL (err);
  int res;

#ifdef __NR_openat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      if (SINGLE_THREAD_P)
	res = INLINE_SYSCALL (openat, 4, fd, file, oflag, mode);
      else
	{
	  int oldtype = LIBC_CANCEL_ASYNC ();

	  res = INLINE_SYSCALL (openat, 4, fd, file, oflag, mode);

	  LIBC_CANCEL_RESET (oldtype);
	}

# ifndef __ASSUME_ATFCTS
      if (res == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return res;
    }
#endif

#ifndef __ASSUME_ATFCTS
  char *buf = NULL;

  if (fd != AT_FDCWD && file[0] != '/')
    {
      size_t filelen = strlen (file);
      static const char procfd[] = "/proc/self/fd/%d/%s";
      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      buf = alloca (buflen);

      __snprintf (buf, buflen, procfd, fd, file);
      file = buf;
    }

  if (SINGLE_THREAD_P)
    res = INTERNAL_SYSCALL (open, err, 3, file, oflag, mode);
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      res = INTERNAL_SYSCALL (open, err, 3, file, oflag, mode);

      LIBC_CANCEL_RESET (oldtype);
    }

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (res, err), 0))
    {
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (res, err), fd, buf);
      res = -1;
    }

  return res;
#endif
}
