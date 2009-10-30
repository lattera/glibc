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


/* Make a symbolic link to FROM named TO relative to TOFD.  */
int
symlinkat (from, tofd, to)
     const char *from;
     int tofd;
     const char *to;
{
  int result;

#ifdef __NR_symlinkat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      result = INLINE_SYSCALL (symlinkat, 3, from, tofd, to);
# ifndef __ASSUME_ATFCTS
      if (result == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return result;
    }
#endif

#ifndef __ASSUME_ATFCTS
  char *buf = NULL;

  if (tofd != AT_FDCWD && to[0] != '/')
    {
      size_t tolen = strlen (to);
      if (__builtin_expect (tolen == 0, 0))
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
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + tolen;
      buf = __alloca (buflen);

      __snprintf (buf, buflen, procfd, tofd, to);
      to = buf;
    }

  INTERNAL_SYSCALL_DECL (err);

  result = INTERNAL_SYSCALL (symlink, err, 2, from, to);

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 0))
    {
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), tofd, buf);
      result = -1;
    }

  return result;
#endif
}
