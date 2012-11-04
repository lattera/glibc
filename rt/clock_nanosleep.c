/* High-resolution sleep with the specified clock.  Stub version.
   Copyright (C) 2000-2012 Free Software Foundation, Inc.
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
#include <time.h>

int
clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
		 struct timespec *rem)
{
  if (__builtin_expect (req->tv_nsec, 0) < 0
      || __builtin_expect (req->tv_nsec, 0) >= 1000000000)
    return EINVAL;

  if (flags != TIMER_ABSTIME && flags != 0)
    return EINVAL;

  /* Not implemented.  */
  return ENOSYS;
}
strong_alias (clock_nanosleep, __clock_nanosleep)
stub_warning (clock_nanosleep)
