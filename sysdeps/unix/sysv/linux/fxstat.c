/* fxstat using old-style Unix fstat system call.
   Copyright (C) 1991,1995-1998,2000,2002,2003 Free Software Foundation, Inc.
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

/* Ho hum, if xstat == xstat64 we must get rid of the prototype or gcc
   will complain since they don't strictly match.  */
#define __fxstat64 __fxstat64_disable

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <xstatconv.h>

/* Get information about the file FD in BUF.  */
int
__fxstat (int vers, int fd, struct stat *buf)
{
  if (vers == _STAT_VER_KERNEL)
    return INLINE_SYSCALL (fstat, 2, fd, CHECK_1 ((struct kernel_stat *) buf));

#ifdef STAT_IS_KERNEL_STAT
  errno = EINVAL;
  return -1;
#else
  struct kernel_stat kbuf;
  int result;

  result = INLINE_SYSCALL (fstat, 2, fd, __ptrvalue (&kbuf));
  if (result == 0)
    result = __xstat_conv (vers, &kbuf, buf);

  return result;
#endif
}

hidden_def (__fxstat)
weak_alias (__fxstat, _fxstat);
#ifdef XSTAT_IS_XSTAT64
#undef __fxstat64
strong_alias (__fxstat, __fxstat64);
hidden_ver (__fxstat, __fxstat64)
#endif
