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

/* Ho hum, since fxstatat == fxstatat64 we must get rid of the
   prototype or gcc will complain since they don't strictly match.  */
#define __fxstatat64 __fxstatat64_disable

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <sysdep.h>
#include <kernel-features.h>
#include <sys/syscall.h>
#include <bp-checks.h>


/* Get information about the file NAME relative to FD in ST.  */
int
__fxstatat (int vers, int fd, const char *file, struct stat *st, int flag)
{
  if (vers != _STAT_VER_KERNEL && vers != _STAT_VER_LINUX)
    {
      __set_errno (EINVAL);
      return -1;
    }

  int res;

#ifdef __NR_newfstatat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      res = INLINE_SYSCALL (newfstatat, 4, fd, file, st, flag);
# ifndef __ASSUME_ATFCTS
      if (res == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return res;
    }
#endif

#ifndef __ASSUME_ATFCTS
  if ((flag & ~AT_SYMLINK_NOFOLLOW) != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

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

  if (flag & AT_SYMLINK_NOFOLLOW)
    res = INTERNAL_SYSCALL (lstat, err, 2, file, CHECK_1 (st));
  else
    res = INTERNAL_SYSCALL (stat, err, 2, file, CHECK_1 (st));

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (res, err), 0))
    {
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (res, err), fd, buf);
      res = -1;
    }

  return res;
#endif
}
libc_hidden_def (__fxstatat)
#undef __fxstatat64
strong_alias (__fxstatat, __fxstatat64);
strong_alias (__fxstatat64, __GI___fxstatat64)
