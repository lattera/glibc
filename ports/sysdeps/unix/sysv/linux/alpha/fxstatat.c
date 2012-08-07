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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define __fxstatat64 __fxstatat64_disable

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <kernel_stat.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <xstatconv.h>

#undef __fxstatat64

#ifdef __ASSUME_ATFCTS
# define __have_atfcts 1
#endif

/* Get information about the file NAME in BUF.  */
int
__fxstatat (int vers, int fd, const char *file, struct stat *st, int flag)
{
  INTERNAL_SYSCALL_DECL (err);
  int result, errno_out;

  /* ??? The __fxstatat entry point is new enough that it must be using
     vers == _STAT_VER_KERNEL64.  For the benefit of dl-fxstatat64.c, we
     cannot actually check this, lest the compiler not optimize the rest
     of the function away.  */

#ifdef __NR_fstatat64
  if (__have_atfcts >= 0)
    {
      result = INTERNAL_SYSCALL (fstatat64, err, 4, fd, file, st, flag);
      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      errno_out = INTERNAL_SYSCALL_ERRNO (result, err);
#ifndef __ASSUME_ATFCTS
      if (errno_out == ENOSYS)
	__have_atfcts = -1;
      else
#endif
	{
	  __set_errno (errno_out);
	  return -1;
	}
    }
#endif /* __NR_fstatat64 */

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
    result = INTERNAL_SYSCALL (lstat64, err, 2, file, st);
  else
    result = INTERNAL_SYSCALL (stat64, err, 2, file, st);
  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return result;

  errno_out = INTERNAL_SYSCALL_ERRNO (result, err);
  __atfct_seterrno (errno_out, fd, buf);

  return -1;
}
libc_hidden_def (__fxstatat)
strong_alias (__fxstatat, __fxstatat64);
libc_hidden_ver(__fxstatat, __fxstatat64);
