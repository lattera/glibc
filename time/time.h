/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.12 DATE and TIME	<time.h>
 */

#ifndef	_TIME_H

#if	!defined(__need_time_t) && !defined(__need_clock_t)
#define	_TIME_H		1
#include <features.h>

__BEGIN_DECLS

#endif

#ifdef	_TIME_H
/* Get size_t and NULL from <stddef.h>.  */
#define	__need_size_t
#define	__need_NULL
#include <stddef.h>
#endif /* <time.h> included.  */



#ifdef	_TIME_H
/* Processor clock ticks per second.  */
#define	CLOCKS_PER_SEC	1	/* ??? */

#ifdef	__USE_POSIX
#define	CLK_TCK		60	/* ??? */
#endif

#endif /* <time.h> included.  */


#if	!defined(__clock_t_defined) &&			\
	(defined(_TIME_H) || defined(__need_clock_t))
#define	__clock_t_defined	1

/* Returned by `clock'.  */
typedef long int clock_t;

#endif /* clock_t not defined and <time.h> or need clock_t.  */
#undef	__need_clock_t

#if	!defined(__time_t_defined) &&			\
	(defined(_TIME_H) || defined(__need_time_t))
#define	__time_t_defined	1

#include <gnu/types.h>

/* Returned by `time'.  */
typedef __time_t time_t;

#endif /* time_t not defined and <time.h> or need time_t.  */
#undef	__need_time_t


#ifdef	_TIME_H
/* Used by other time functions.  */
struct tm
{
  int tm_sec;			/* Seconds.	[0-61] (2 leap seconds) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year	- 1900.  */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/
  long int tm_gmtoff;		/* Seconds west of UTC.  */
  __const char *tm_zone;	/* Timezone abbreviation.  */
};

#endif /* <time.h> included.  */


#ifdef	_TIME_H
/* Time used by the program so far (user time + system time).
   The result / CLOCKS_PER_SECOND is program time in seconds.  */
extern clock_t clock __P ((void));

/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
extern time_t time __P ((time_t *__timer));

/* Return the difference between TIME1 and TIME0.  */
extern double difftime __P ((time_t __time1, time_t __time0))
     __attribute__ ((__const__));

/* Return the `time_t' representation of TP and normalize TP.  */
extern time_t mktime __P ((struct tm *__tp));

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  */
extern time_t __mktime_internal __P ((struct tm *__tp,
				      struct tm *(*__func) (const time_t *,
							    struct tm *)));


/* Format TP into S according to FORMAT.
   Write no more than MAXSIZE characters and return the number
   of characters written, or 0 if it would exceed MAXSIZE.  */
extern size_t strftime __P ((char *__s, size_t __maxsize,
			 __const char *__format, __const struct tm *__tp));


/* Return the `struct tm' representation of *TIMER
   in Universal Coordinated Time (aka Greenwich Mean Time).  */
extern struct tm *gmtime __P ((__const time_t *__timer));

/* Return the `struct tm' representation
   of *TIMER in the local timezone.  */
extern struct tm *localtime __P ((__const time_t *__timer));

#ifdef	__USE_REENTRANT
/* Return the `struct tm' representation of *TIMER in UTC,
   using *TP to store the result.  */
extern struct tm *__gmtime_r __P ((__const time_t *__timer,
				   struct tm *__tp));
extern struct tm *gmtime_r __P ((__const time_t *__timer,
				 struct tm *__tp));

/* Return the `struct tm' representation of *TIMER in local time,
   using *TP to store the result.  */
extern struct tm *__localtime_r __P ((__const time_t *__timer,
				      struct tm *__tp));
extern struct tm *localtime_r __P ((__const time_t *__timer,
				    struct tm *__tp));
#endif

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.  */
extern void __offtime __P ((__const time_t *__timer,
			    long int __offset,
			    struct tm *__TP));

/* Return a string of the form "Day Mon dd hh:mm:ss yyyy\n"
   that is the representation of TP in this format.  */
extern char *asctime __P ((__const struct tm *__tp));

/* Equivalent to `asctime(localtime(timer))'.  */
extern char *ctime __P ((__const time_t *__timer));


/* Defined in localtime.c.  */
extern char *__tzname[2];	/* Current timezone names.  */
extern int __daylight;		/* If it is daylight savings time.  */
extern long int __timezone;	/* Seconds west of UTC.  */

/* Set time conversion information from the TZ environment variable.
   If TZ is not defined, a locale-dependent default is used.  */
extern void __tzset __P ((void));

#ifdef	__USE_POSIX
/* Same as above.  */
extern char *tzname[2];

/* Return the maximum length of a timezone name.
   This is what `sysconf (_SC_TZNAME_MAX)' does.  */
extern long int __tzname_max __P ((void));

extern void tzset __P ((void));
#ifdef	__OPTIMIZE__
#define	tzset()	__tzset()
#endif /* Optimizing.  */
#endif

#ifdef	__USE_SVID
extern int daylight;
extern long int timezone;

/* Set the system time to *WHEN.
   This call is restricted to the superuser.  */
extern int stime __P ((__const time_t *__when));
#endif


/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */
#define	__isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))


#ifdef __USE_MISC
/* Miscellaneous functions many Unices inherited from the public domain
   localtime package.  These are included only for compatibility.  */

/* Like `mktime', but for TP represents Universal Time, not local time.  */
extern time_t timegm __P ((struct tm *__tp));

/* Another name for `mktime'.  */
extern time_t timelocal __P ((struct tm *__tp));

/* Return the number of days in YEAR.  */
extern int dysize __P ((int __year));
#endif


__END_DECLS

#endif /* <time.h> included.  */

#endif /* <time.h> not already included.  */
