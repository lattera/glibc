/* Get the current wall clock time.  NaCl version.
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
#include <sys/time.h>
#include <nacl-interfaces.h>


/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */
int
__gettimeofday (struct timeval *tv, struct timezone *tz)
{
  if (__glibc_unlikely (tz != NULL))
    {
      tz->tz_minuteswest = 0;
      tz->tz_dsttime = 0;
    }

  return NACL_CALL (__nacl_irt_basic.gettod (tv), 0);
}
libc_hidden_def (__gettimeofday)
weak_alias (__gettimeofday, gettimeofday)
libc_hidden_weak (gettimeofday)
