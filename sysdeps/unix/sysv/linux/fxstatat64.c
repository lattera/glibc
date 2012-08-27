/* Copyright (C) 2005-2012 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

/* Get information about the file NAME in BUF.  */

int
__fxstatat64 (int vers, int fd, const char *file, struct stat64 *st, int flag)
{
  if (__builtin_expect (vers != _STAT_VER_LINUX, 0))
    {
      __set_errno (EINVAL);
      return -1;
    }

  int result;
  INTERNAL_SYSCALL_DECL (err);

#ifdef __NR_fstatat64
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      result = INTERNAL_SYSCALL (fstatat64, err, 4, fd, file, st, flag);
# ifndef __ASSUME_ATFCTS
      if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 1)
	  && INTERNAL_SYSCALL_ERRNO (result, err) == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	if (!__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	  return 0;
	else
	  {
	    __set_errno (INTERNAL_SYSCALL_ERRNO (result, err));
	    return -1;
	  }
    }
#endif

#ifndef __ASSUME_ATFCTS
  if (flag & ~AT_SYMLINK_NOFOLLOW)
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

  if (flag & AT_SYMLINK_NOFOLLOW)
    result = INTERNAL_SYSCALL (lstat64, err, 2, CHECK_STRING (file),
			       CHECK_1 (st));
  else
    result = INTERNAL_SYSCALL (stat64, err, 2, CHECK_STRING (file),
			       CHECK_1 (st));
  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    {
# if defined _HAVE_STAT64___ST_INO && __ASSUME_ST_INO_64_BIT == 0
      if (st->__st_ino != (__ino_t) st->st_ino)
	st->st_ino = st->__st_ino;
# endif
      return result;
    }
  __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), fd, buf);

  return -1;
#endif
}
libc_hidden_def (__fxstatat64)
