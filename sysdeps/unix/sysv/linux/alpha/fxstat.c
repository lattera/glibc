/* fxstat using old-style Unix stat system call.
   Copyright (C) 2004 Free Software Foundation, Inc.
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

#define __fxstat64 __fxstat64_disable

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <kernel_stat.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <xstatconv.h>

#undef __fxstat64


/* Get information about the file NAME in BUF.  */
int
__fxstat (int vers, int fd, struct stat *buf)
{
  INTERNAL_SYSCALL_DECL (err);
  int result, errno_out;
  struct kernel_stat kbuf;

  if (vers == _STAT_VER_KERNEL64 && !__libc_missing_axp_stat64)
    {
      result = INTERNAL_SYSCALL (fstat64, err, 2, fd, buf);
      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      errno_out = INTERNAL_SYSCALL_ERRNO (result, err);
      if (errno_out != ENOSYS)
	goto fail;
      __libc_missing_axp_stat64 = 1;
    }

  result = INTERNAL_SYSCALL (fstat, err, 2, fd, &kbuf);
  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return __xstat_conv (vers, &kbuf, buf);
  errno_out = INTERNAL_SYSCALL_ERRNO (result, err);
  
 fail:
  __set_errno (errno_out);
  return -1;
}
hidden_def (__fxstat)
weak_alias (__fxstat, _fxstat);
strong_alias (__fxstat, __fxstat64);
hidden_ver (__fxstat, __fxstat64)
