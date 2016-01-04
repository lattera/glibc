/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */
int
__gettimeofday (struct timeval *tv, struct timezone *tz)
{
  if (tv == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  tv->tv_sec = (long int) time ((time_t *) NULL);
  tv->tv_usec = 0L;

  if (tz != NULL)
    {
      const time_t timer = tv->tv_sec;
      struct tm tm;
      const struct tm *tmp;

      const long int save_timezone = __timezone;
      const long int save_daylight = __daylight;
      char *save_tzname[2];
      save_tzname[0] = __tzname[0];
      save_tzname[1] = __tzname[1];

      tmp = localtime_r (&timer, &tm);

      tz->tz_minuteswest = __timezone / 60;
      tz->tz_dsttime = __daylight;

      __timezone = save_timezone;
      __daylight = save_daylight;
      __tzname[0] = save_tzname[0];
      __tzname[1] = save_tzname[1];

      if (tmp == NULL)
	return -1;
    }

  return 0;
}
libc_hidden_def (__gettimeofday)
weak_alias (__gettimeofday, gettimeofday)
libc_hidden_weak (gettimeofday)
