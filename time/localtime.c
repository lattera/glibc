/* Convert `time_t' to `struct tm' in local time zone.
   Copyright (C) 1991, 92, 93, 95, 96, 97 Free Software Foundation, Inc.
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
#include <libc-lock.h>

/* The C Standard says that localtime and gmtime return the same pointer.  */
struct tm _tmbuf;

/* Prototype for the internal function to get information based on TZ.  */
extern void __tzset_internal __P ((int always));
extern int __tz_compute __P ((time_t timer, struct tm *tp));
extern int __tzfile_compute __P ((time_t timer,
				  long int *leap_correct, int *leap_hit));

extern int __use_tzfile;

/* This lock is defined in tzset.c and locks all the data defined there
   and in tzfile.c; the internal functions do no locking themselves.
   This lock is only taken here and in `tzset'.  */
__libc_lock_define (extern, __tzset_lock)


/* Return the `struct tm' representation of *TIMER in the local timezone.  */
static struct tm *
localtime_internal (const time_t *timer, struct tm *tp)
{
  long int leap_correction;
  int leap_extra_secs;

  if (timer == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  if (__use_tzfile)
    {
      if (! __tzfile_compute (*timer, &leap_correction, &leap_extra_secs))
	tp = NULL;
    }
  else
    {
      tp = __gmtime_r (timer, tp);
      if (tp && ! __tz_compute (*timer, tp))
	tp = NULL;
      leap_correction = 0L;
      leap_extra_secs = 0;
    }

  if (tp)
    {
      __offtime (timer, __timezone - leap_correction, tp);
      tp->tm_sec += leap_extra_secs;
      tp->tm_isdst = __daylight;
      tp->tm_gmtoff = __timezone;
      tp->tm_zone = __tzname[__daylight];
    }

  return tp;
}


/* POSIX.1 8.3.7.2 says that localtime_r is not required to set
   tzname.  This is a good idea since this allows at least a bit more
   parallelism.  */

struct tm *
localtime (timer)
     const time_t *timer;
{
  struct tm *result;

  __libc_lock_lock (__tzset_lock);

  /* Update internal database according to current TZ setting.  */
  __tzset_internal (1);

  result = localtime_internal (timer, &_tmbuf);

  __libc_lock_unlock (__tzset_lock);

  return result;
}


struct tm *
__localtime_r (timer, tp)
     const time_t *timer;
     struct tm *tp;
{
  struct tm *result;

  __libc_lock_lock (__tzset_lock);

  /* Make sure the database is initialized.  */
  __tzset_internal (0);

  result = localtime_internal (timer, tp);

  __libc_lock_unlock (__tzset_lock);

  return result;
}
weak_alias (__localtime_r, localtime_r)
