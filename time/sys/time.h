/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

#ifndef _SYS_TIME_H

#define _SYS_TIME_H	1
#include <features.h>

#include <time.h>

__BEGIN_DECLS

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval
  {
    int tv_sec;			/* Seconds.  */
    int tv_usec;		/* Microseconds.  */
  };

/* POSIX.4 structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec
  {
    long int ts_sec;		/* Seconds.  */
    long int ts_nsec;		/* Nanoseconds.  */
  };

/* Macros for converting between `struct timeval' and `struct timespec'.  */
#define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->ts_sec = (tv)->tv_sec;                                    \
        (ts)->ts_nsec = (tv)->tv_usec * 1000;                           \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->ts_sec;                                    \
        (tv)->tv_usec = (ts)->ts_nsec / 1000;                           \
}


/* Structure crudely representing a timezone.
   This is obsolete and should never be used.  */
struct timezone
  {
    int tz_minuteswest;		/* Minutes west of GMT.  */
    int tz_dsttime;		/* Nonzero if DST is ever in effect.  */
  };

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.
   NOTE: This form of timezone information is obsolete.
   Use the functions and variables declared in <time.h> instead.  */
extern int __gettimeofday __P ((struct timeval *__tv,
				struct timezone *__tz));
extern int gettimeofday __P ((struct timeval *__tv,
			      struct timezone *__tz));

/* Set the current time of day and timezone information.
   This call is restricted to the super-user.  */
extern int __settimeofday __P ((__const struct timeval *__tv,
				__const struct timezone *__tz));
extern int settimeofday __P ((__const struct timeval *__tv,
			      __const struct timezone *__tz));

/* Adjust the current time of day by the amount in DELTA.
   If OLDDELTA is not NULL, it is filled in with the amount
   of time adjustment remaining to be done from the last `adjtime' call.
   This call is restricted to the super-user.  */
extern int __adjtime __P ((__const struct timeval *__delta,
			   struct timeval *__olddelta));
extern int adjtime __P ((__const struct timeval *__delta,
			 struct timeval *__olddelta));


/* Values for the first argument to `getitimer' and `setitimer'.  */
enum __itimer_which
  {
    /* Timers run in real time.  */
    ITIMER_REAL = 0,
    /* Timers run only when the process is executing.  */
    ITIMER_VIRTUAL = 1,
    /* Timers run when the process is executing and when
       the system is executing on behalf of the process.  */
    ITIMER_PROF = 2
  };

/* Type of the second argument to `getitimer' and
   the second and third arguments `setitimer'.  */
struct itimerval
  {
    /* Value to put into `it_value' when the timer expires.  */
    struct timeval it_interval;
    /* Time to the next timer expiration.  */
    struct timeval it_value;
  };

/* Set *VALUE to the current setting of timer WHICH.
   Return 0 on success, -1 on errors.  */
extern int __getitimer __P ((enum __itimer_which __which,
			     struct itimerval *__value));
extern int getitimer __P ((enum __itimer_which __which,
			   struct itimerval *__value));

/* Set the timer WHICH to *NEW.  If OLD is not NULL,
   set *OLD to the old value of timer WHICH.
   Returns 0 on success, -1 on errors.  */
extern int __setitimer __P ((enum __itimer_which __which,
			     struct itimerval *__new,
			     struct itimerval *__old));
extern int setitimer __P ((enum __itimer_which __which,
			   struct itimerval *__new,
			   struct itimerval *__old));

/* Change the access time of FILE to TVP[0] and
   the modification time of FILE to TVP[1].  */
extern int __utimes __P ((__const char *__file, struct timeval __tvp[2]));
extern int utimes __P ((__const char *__file, struct timeval __tvp[2]));


/* Convenience macros for operations on timevals.
   NOTE: `timercmp' does not work for >= or <=.  */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, CMP)	\
  ((tvp)->tv_sec CMP (uvp)->tv_sec || \
   (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec CMP (uvp)->tv_usec)
#define	timerclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)



__END_DECLS

#endif /* sys/time.h */
