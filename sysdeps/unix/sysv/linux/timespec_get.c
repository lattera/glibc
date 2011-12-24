/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <time.h>
#include <sysdep.h>
#include <kernel-features.h>

#ifndef HAVE_CLOCK_GETTIME_VSYSCALL
# undef INTERNAL_VSYSCALL
# define INTERNAL_VSYSCALL INTERNAL_SYSCALL
#else
# include <bits/libc-vdso.h>
#endif

#ifndef INTERNAL_GETTIME
# define INTERNAL_GETTIME(id, tp) \
  INTERNAL_VSYSCALL (clock_gettime, err, 2, id, tp)
#endif


/* Set TS to calendar time based in time base BASE.  */
int
timespec_get (ts, base)
     struct timespec *ts;
     int base;
{
  switch (base)
    {
      int res;
      INTERNAL_SYSCALL_DECL (err);
    case TIME_UTC:
      res = INTERNAL_GETTIME (CLOCK_REALTIME, ts);
      if (INTERNAL_SYSCALL_ERROR_P (res, err))
	return 0;
      break;

    default:
      return 0;
    }

  return base;
}
