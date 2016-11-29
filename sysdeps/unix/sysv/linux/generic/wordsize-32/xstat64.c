/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

/* Hide the prototype for __xstat so that GCC will not complain about
   the different function signature if it is aliased to  __xstat64.
   If XSTAT_IS_XSTAT64 is set to non-zero then the stat and stat64
   structures have an identical layout but different type names.  */

#define __xstat __xstat_disable

#include <errno.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Get information about the file NAME in BUF.  */
int
__xstat64 (int vers, const char *name, struct stat64 *buf)
{
  if (vers == _STAT_VER_KERNEL)
    return INLINE_SYSCALL (fstatat64, 4, AT_FDCWD, name, buf, 0);

  errno = EINVAL;
  return -1;
}
hidden_def (__xstat64)

#undef __xstat
#if XSTAT_IS_XSTAT64
strong_alias (__xstat64, __xstat)
hidden_ver (__xstat64, __xstat)
#endif
