/* Copyright (C) 1991, 92, 94, 95, 96, 97 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <time.h>
#include <sys/time.h>

#ifndef HAVE_GNU_LD
#define __daylight	daylight
#define __timezone	timezone
#define __tzname	tzname
#endif

/* Assembler Routines to access the timer registers */
asm("
.rtc_upper: mfspr   3,4         # copy RTCU to return register
            blr

.rtc_lower: mfspr   3,5         # copy RTCL to return register
            blr
");

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */
int
__gettimeofday (tv, tz)
     struct timeval  *tv;
     struct timezone *tz;
{
  int ts, tl, tu;

  if (tv == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  ts = rtc_upper();      /* seconds                         */
  tl = rtc_lower();      /* nanoseconds                     */
  tu = rtc_upper();      /* Check for a carry from          */
  if (ts != tu)          /* the lower reg to the upper      */
      tl  = rtc_lower(); /* Recover from the race condition */

  tv->tv_sec  = (long int) (tu + (double)tl/1000000000); 
  tv->tv_usec = (long int) ((double)tl/1000);

#if 0
  if (tz != NULL)
    {
      const  time_t timer = tv->tv_sec;
      struct tm tm;
      const  struct tm *tmp;

      const long int save_timezone = __timezone;
      const long int save_daylight = __daylight;
      char *save_tzname[2];
      save_tzname[0] = __tzname[0];
      save_tzname[1] = __tzname[1];

      tmp = localtime_r (&timer, &tm);

      tz->tz_minuteswest = __timezone / 60;
      tz->tz_dsttime     = __daylight;

      __timezone  = save_timezone;
      __daylight  = save_daylight;
      __tzname[0] = save_tzname[0];
      __tzname[1] = save_tzname[1];

      if (tmp == NULL)
	return -1;
    }
#endif

  return 0;
}

weak_alias (__gettimeofday, gettimeofday)
