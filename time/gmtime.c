/* Copyright (C) 1991, 1993, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stddef.h>
#include <time.h>

/* Defined in localtime.c.  */
extern struct tm _tmbuf;

/* Return the `struct tm' representation of *T in UTC.	*/
struct tm *
DEFUN(gmtime, (t), CONST time_t *t)
{
  return __gmtime_r (t, &_tmbuf);
}

/* Return the `struct tm' representation of *T in UTC,
   using *TP to store the result.  */
struct tm *
DEFUN(__gmtime_r, (t, tp),
      CONST time_t *t AND struct tm *tp)
{
  __offtime (t, 0L, tp);

  tp->tm_isdst = 0;
  tp->tm_gmtoff = 0L;
  tp->tm_zone = "GMT";

  return tp;
}
weak_alias (__gmtime_r, gmtime_r)
