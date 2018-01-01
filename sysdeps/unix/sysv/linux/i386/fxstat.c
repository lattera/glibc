/* fxstat using old-style Unix fstat system call.
   Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

/* Ho hum, if xstat == xstat64 we must get rid of the prototype or gcc
   will complain since they don't strictly match.  */
#define __fxstat64 __fxstat64_disable

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <xstatconv.h>

/* Get information about the file FD in BUF.  */
int
__fxstat (int vers, int fd, struct stat *buf)
{
  int result;

  if (vers == _STAT_VER_KERNEL)
    return INLINE_SYSCALL (fstat, 2, fd, buf);

  {
    struct stat64 buf64;

    INTERNAL_SYSCALL_DECL (err);
    result = INTERNAL_SYSCALL (fstat64, err, 2, fd, &buf64);
    if (__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (result, err)))
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (INTERNAL_SYSCALL_ERRNO (result,
									err));
    else
      return __xstat32_conv (vers, &buf64, buf);
  }
}

hidden_def (__fxstat)
weak_alias (__fxstat, _fxstat);
#if XSTAT_IS_XSTAT64
#undef __fxstat64
strong_alias (__fxstat, __fxstat64);
hidden_ver (__fxstat, __fxstat64)
#endif
