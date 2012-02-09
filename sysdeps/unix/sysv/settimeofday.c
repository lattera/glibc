/* Copyright (C) 1992, 1995, 1996, 1997, 2001 Free Software Foundation, Inc.
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
#include <sys/time.h>

/* Set the current time of day and timezone information.
   This call is restricted to the super-user.  */
int
__settimeofday (tv, tz)
     const struct timeval *tv;
     const struct timezone *tz;
{
  time_t when;

  if (tv == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (tz != NULL || tv->tv_usec % 1000000 != 0)
    {
      __set_errno (ENOSYS);
      return -1;
    }

  when = tv->tv_sec + (tv->tv_usec / 1000000);
  return stime (&when);
}

weak_alias (__settimeofday, settimeofday)
