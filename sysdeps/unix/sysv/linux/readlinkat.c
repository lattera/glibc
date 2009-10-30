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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdep.h>
#include <unistd.h>
#include <kernel-features.h>


/* Read the contents of the symbolic link PATH relative to FD into no
   more than LEN bytes of BUF.  */
ssize_t
readlinkat (fd, path, buf, len)
     int fd;
     const char *path;
     char *buf;
     size_t len;
{
  int result;

#ifdef __NR_readlinkat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      result = INLINE_SYSCALL (readlinkat, 4, fd, path, buf, len);
# ifndef __ASSUME_ATFCTS
      if (result == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return result;
    }
#endif

#ifndef __ASSUME_ATFCTS
  char *pathbuf = NULL;

  if (fd != AT_FDCWD && path[0] != '/')
    {
      size_t pathlen = strlen (path);
      if (__builtin_expect (pathlen == 0, 0))
	{
	  __set_errno (ENOENT);
	  return -1;
	}

      static const char procfd[] = "/proc/self/fd/%d/%s";
      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + pathlen;
      pathbuf = __alloca (buflen);

      __snprintf (pathbuf, buflen, procfd, fd, path);
      path = pathbuf;
    }

  INTERNAL_SYSCALL_DECL (err);

  result = INTERNAL_SYSCALL (readlink, err, 3, path, buf, len);

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 0))
    {
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), fd, pathbuf);
      result = -1;
    }

  return result;
#endif
}
libc_hidden_def (readlinkat)
