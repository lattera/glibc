/* Copyright (C) 2012 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <stddef.h>
#include <sys/time.h>
#include <time.h>
#include <bits/libc-vdso.h>

int
__gettimeofday (struct timeval *tv, struct timezone *tz)
{
#ifdef SHARED
  /* If the vDSO is available we use it. */
  if (__vdso_gettimeofday != NULL)
    return __vdso_gettimeofday (tv, tz);
#endif
  return INLINE_SYSCALL (gettimeofday, 2, tv, tz);
}

libc_hidden_def (__gettimeofday)
weak_alias (__gettimeofday, gettimeofday)
libc_hidden_weak (gettimeofday)
