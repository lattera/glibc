/* xstat using old-style Unix stat system call.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

#define __xstat64 __xstat64_disable

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <kernel_stat.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <xstatconv.h>

#undef __xstat64


/* Get information about the file NAME in BUF.  */
int
__xstat (int vers, const char *name, struct stat *buf)
{
  INTERNAL_SYSCALL_DECL (err);
  int result;
  struct kernel_stat kbuf;

  if (vers == _STAT_VER_KERNEL64)
    {
      result = INTERNAL_SYSCALL (stat64, err, 2, name, buf);
      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      __set_errno (INTERNAL_SYSCALL_ERRNO (result, err));
      return -1;
    }

  result = INTERNAL_SYSCALL (stat, err, 2, name, &kbuf);
  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return __xstat_conv (vers, &kbuf, buf);
  __set_errno (INTERNAL_SYSCALL_ERRNO (result, err));
  return -1;
}
hidden_def (__xstat)
weak_alias (__xstat, _xstat);
strong_alias (__xstat, __xstat64);
hidden_ver (__xstat, __xstat64)
