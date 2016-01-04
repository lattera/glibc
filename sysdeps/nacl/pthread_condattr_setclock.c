/* Set the clock for timeouts on a condition variable.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
#include "pthreadP.h"


int
pthread_condattr_setclock (pthread_condattr_t *attr, clockid_t clock_id)
{
  switch (clock_id)
    {
    case CLOCK_REALTIME:
      /* This is the default state and the only one actually supported.  */
      return 0;

    case CLOCK_MONOTONIC:
      /* NaCl recognizes CLOCK_MONOTONIC for other purposes, so it is a
         "known clock".  But NaCl doesn't support it for this purpose.  */
      return ENOTSUP;

    default:
      /* The only other recognized clocks are CPU-time clocks,
         which POSIX says should get EINVAL.  */
      return EINVAL;
    }
}
