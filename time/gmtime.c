/* Convert `time_t' to `struct tm' in UTC.
   Copyright (C) 1991, 93, 95, 96, 97, 98, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <time.h>

/* Return the `struct tm' representation of *T in UTC,
   using *TP to store the result.  */
struct tm *
__gmtime_r (t, tp)
     const time_t *t;
     struct tm *tp;
{
  return __tz_convert (t, 0, tp);
}
libc_hidden_def (__gmtime_r)
weak_alias (__gmtime_r, gmtime_r)


/* Return the `struct tm' representation of *T in UTC.	*/
struct tm *
gmtime (t)
     const time_t *t;
{
  return __tz_convert (t, 0, &_tmbuf);
}
