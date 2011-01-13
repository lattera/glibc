/* Copyright (C) 2005, 2006, 2009, 2011 Free Software Foundation, Inc.
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
#include <string.h>
#include <sys/stat.h>
#include <kernel-features.h>
#include <sysdep-cancel.h>


/* Create a new directory with permission bits MODE.  But interpret
   relative PATH names relative to the directory associated with FD.  */
int
mkdirat (fd, file, mode)
     int fd;
     const char *file;
     mode_t mode;
{
  int res;

#ifdef __NR_mkdirat
#  ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      res = INLINE_SYSCALL (mkdirat, 3, fd, file, mode);
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
      if (__builtin_expect (filelen == 0, 0))
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
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      buf = alloca (buflen);

      __snprintf (buf, buflen, procfd, fd, file);
      file = buf;
    }

  INTERNAL_SYSCALL_DECL (err);
  res = INTERNAL_SYSCALL (mkdir, err, 2, file, mode);

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (res, err), 0))
    {
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (res, err), fd, buf);
      res = -1;
    }

  return res;
#endif
}
