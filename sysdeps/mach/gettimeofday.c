/* Copyright (C) 1991,1992,1995-1997,2001,2002 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <sys/time.h>
#include <mach.h>

#undef __gettimeofday

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */
int
__gettimeofday (tv, tz)
     struct timeval *tv;
     struct timezone *tz;
{
  kern_return_t err;

  if (tz != NULL)
    *tz = (struct timezone){0, 0}; /* XXX */

  if (err = __host_get_time (__mach_host_self (), (time_value_t *) tv))
    {
      errno = err;
      return -1;
    }
  return 0;
}

INTDEF(__gettimeofday)
weak_alias (__gettimeofday, gettimeofday)
