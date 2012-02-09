/* Convert `time_t' to `struct tm' in local time zone.
   Copyright (C) 1991,92,93,95,96,97,98,2002 Free Software Foundation, Inc.
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

#include <time.h>

/* The C Standard says that localtime and gmtime return the same pointer.  */
struct tm _tmbuf;


/* Return the `struct tm' representation of *T in local time,
   using *TP to store the result.  */
struct tm *
__localtime_r (t, tp)
     const time_t *t;
     struct tm *tp;
{
  return __tz_convert (t, 1, tp);
}
weak_alias (__localtime_r, localtime_r)


/* Return the `struct tm' representation of *T in local time.  */
struct tm *
localtime (t)
     const time_t *t;
{
  return __tz_convert (t, 1, &_tmbuf);
}
libc_hidden_def (localtime)
