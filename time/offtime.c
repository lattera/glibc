/* Copyright (C) 1991, 1993 Free Software Foundation, Inc.
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
#include <time.h>


/* Defined in mktime.c.  */
extern CONST unsigned short int __mon_lengths[2][12];

#define	SECS_PER_HOUR	(60 * 60)
#define	SECS_PER_DAY	(SECS_PER_HOUR * 24)

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.  */
void
DEFUN(__offtime, (t, offset, tp),
      CONST time_t *t AND long int offset AND struct tm *tp)
{
  register long int days, rem;
  register int y;
  register CONST unsigned short int *ip;

  days = *t / SECS_PER_DAY;
  rem = *t % SECS_PER_DAY;
  rem += offset;
  while (rem < 0)
    {
      rem += SECS_PER_DAY;
      --days;
    }
  while (rem >= SECS_PER_DAY)
    {
      rem -= SECS_PER_DAY;
      ++days;
    }
  tp->tm_hour = rem / SECS_PER_HOUR;
  rem %= SECS_PER_HOUR;
  tp->tm_min = rem / 60;
  tp->tm_sec = rem % 60;
  /* January 1, 1970 was a Thursday.  */
  tp->tm_wday = (4 + days) % 7;
  if (tp->tm_wday < 0)
    tp->tm_wday += 7;
  y = 1970;
  while (days >= (rem = __isleap(y) ? 366 : 365))
    {
      ++y;
      days -= rem;
    }
  while (days < 0)
    {
      --y;
      days += __isleap(y) ? 366 : 365;
    }
  tp->tm_year = y - 1900;
  tp->tm_yday = days;
  ip = __mon_lengths[__isleap(y)];
  for (y = 0; days >= ip[y]; ++y)
    days -= ip[y];
  tp->tm_mon = y;
  tp->tm_mday = days + 1;
  tp->tm_isdst = -1;
}
