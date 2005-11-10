/* Copyright (C) 2005 Free Software Foundation, Inc.
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
#include <sysdep-cancel.h>


#ifndef OPENAT
# define OPENAT openat
# define MORE_OFLAGS 0
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
  char *buf = NULL;

  if (file[0] != '/')
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

  mode_t mode = 0;
  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
    }

  INTERNAL_SYSCALL_DECL (err);
  int res;

  if (SINGLE_THREAD_P)
    res = INTERNAL_SYSCALL (open, err, 3, file, oflag | MORE_OFLAGS, mode);
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      res = INTERNAL_SYSCALL (open, err, 3, file, oflag | MORE_OFLAGS, mode);

      LIBC_CANCEL_RESET (oldtype);
    }

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (res, err), 0))
    {
      int errval = INTERNAL_SYSCALL_ERRNO (res, err);
      if (buf != NULL && errval == ENOTDIR)
	{
	  /* This can mean either the file desriptor is invalid or
	     /proc is not mounted.  */
	  struct stat64 st;
	  if (__fxstat64 (_STAT_VER, fd, &st) != 0)
	    /* errno is already set correctly.  */
	    goto out;

	  /* If /proc is not mounted there is nothing we can do.  */
	  if (S_ISDIR (st.st_mode)
	      && (__xstat64 (_STAT_VER, "/proc/self/fd", &st) != 0
		  || !S_ISDIR (st.st_mode)))
	    errval = ENOSYS;
	}

      __set_errno (errval);
    }

 out:
  return res;
}
