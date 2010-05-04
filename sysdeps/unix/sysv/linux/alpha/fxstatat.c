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


/* Get information about the file NAME in BUF.  */
int
__fxstatat (int vers, int fd, const char *file, struct stat *st, int flag)
{
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

  INTERNAL_SYSCALL_DECL (err);
  int result, errno_out;
  struct kernel_stat kst;

#if __ASSUME_STAT64_SYSCALL > 0
  if (vers == _STAT_VER_KERNEL64)
    {
      if (flag & AT_SYMLINK_NOFOLLOW)
	result = INTERNAL_SYSCALL (lstat64, err, 2, file, st);
      else
	result = INTERNAL_SYSCALL (stat64, err, 2, file, st);

      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      errno_out = INTERNAL_SYSCALL_ERRNO (result, err);
      goto fail;
    }
#elif defined __NR_stat64
  if (vers == _STAT_VER_KERNEL64 && !__libc_missing_axp_stat64)
    {
      if (flag & AT_SYMLINK_NOFOLLOW)
	result = INTERNAL_SYSCALL (lstat64, err, 2, file, st);
      else
	result = INTERNAL_SYSCALL (stat64, err, 2, file, st);

      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      errno_out = INTERNAL_SYSCALL_ERRNO (result, err);
      if (errno_out != ENOSYS)
	goto fail;
      __libc_missing_axp_stat64 = 1;
    }
#endif

  if (flag & AT_SYMLINK_NOFOLLOW)
    result = INTERNAL_SYSCALL (lstat, err, 2, file, &kst);
  else
    result = INTERNAL_SYSCALL (stat, err, 2, file, &kst);

  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return __xstat_conv (vers, &kst, st);
  errno_out = INTERNAL_SYSCALL_ERRNO (result, err);

 fail:
  __atfct_seterrno (errno_out, fd, buf);

  return -1;
}
libc_hidden_def (__fxstatat)
strong_alias (__fxstatat, __fxstatat64);
libc_hidden_ver(__fxstatat, __fxstatat64);
