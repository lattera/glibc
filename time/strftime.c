/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _LIBC
# define HAVE_LIMITS_H 1
# define HAVE_MBLEN 1
# define HAVE_TM_GMTOFF 1
# define HAVE_TM_ZONE 1
# define STDC_HEADERS 1
# include <ansidecl.h>
# include "../locale/localeinfo.h"
#endif

#include <sys/types.h>		/* Some systems define `time_t' here.  */

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_MBLEN
# include <ctype.h>
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#endif

#if STDC_HEADERS
# include <stddef.h>
# include <stdlib.h>
# include <string.h>
#else
# define memcpy(d, s, n) bcopy (s, d, n)
#endif

#ifndef __P
#if defined (__GNUC__) || (defined (__STDC__) && __STDC__)
#define __P(args) args
#else
#define __P(args) ()
#endif  /* GCC.  */
#endif  /* Not __P.  */

#ifndef PTR
#ifdef __STDC__
#define PTR void *
#else
#define PTR char *
#endif
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#define TYPE_SIGNED(t) ((t) -1 < 0)

/* Bound on length of the string representing an integer value of type t.
   Subtract one for the sign bit if t is signed;
   302 / 1000 is log10 (2) rounded up;
   add one for integer division truncation;
   add one more for a minus sign if t is signed.  */
#define INT_STRLEN_BOUND(t) \
  ((sizeof (t) * CHAR_BIT - TYPE_SIGNED (t)) * 302 / 100 + 1 + TYPE_SIGNED (t))

#define TM_YEAR_BASE 1900


#ifdef _LIBC
# define gmtime_r __gmtime_r
# define localtime_r __localtime_r
#else
# if ! HAVE_LOCALTIME_R
#  if ! HAVE_TM_GMTOFF
/* Approximate gmtime_r as best we can in its absence.  */
#define gmtime_r my_gmtime_r
static struct tm *gmtime_r __P ((const time_t *, struct tm *));
static struct tm *
gmtime_r (t, tp)
     const time_t *t;
     struct tm *tp;
{
  struct tm *l = gmtime (t);
  if (! l)
    return 0;
  *tp = *l;
  return tp;
}
#  endif /* ! HAVE_TM_GMTOFF */

/* Approximate localtime_r as best we can in its absence.  */
#define localtime_r my_localtime_r
static struct tm *localtime_r __P ((const time_t *, struct tm *));
static struct tm *
localtime_r (t, tp)
     const time_t *t;
     struct tm *tp;
{
  struct tm *l = localtime (t);
  if (! l)
    return 0;
  *tp = *l;
  return tp;
}
# endif /* ! HAVE_LOCALTIME_R */
#endif /* ! defined (_LIBC) */


static unsigned int week __P ((const struct tm *const, int, int));


#define	add(n, f)							      \
  do									      \
    {									      \
      i += (n);								      \
      if (i >= maxsize)							      \
	return 0;							      \
      else								      \
	if (p)								      \
	  {								      \
	    f;								      \
	    p += (n);							      \
	  }								      \
    } while (0)
#define	cpy(n, s)	add ((n), memcpy((PTR) p, (PTR) (s), (n)))

#if ! HAVE_TM_GMTOFF
/* Yield the difference between *A and *B,
   measured in seconds, ignoring leap seconds.  */
static int tm_diff __P ((const struct tm *, const struct tm *));
static int
tm_diff (a, b)
     const struct tm *a;
     const struct tm *b;
{
  int ay = a->tm_year + TM_YEAR_BASE - 1;
  int by = b->tm_year + TM_YEAR_BASE - 1;
  /* Divide years by 100, rounding towards minus infinity.  */
  int ac = ay / 100 - (ay % 100 < 0);
  int bc = by / 100 - (by % 100 < 0);
  int intervening_leap_days =
    ((ay >> 2) - (by >> 2)) - (ac - bc) + ((ac >> 2) - (bc >> 2));
  int years = ay - by;
  int days = (365 * years + intervening_leap_days
	      + (a->tm_yday - b->tm_yday));
  return (60 * (60 * (24 * days + (a->tm_hour - b->tm_hour))
		+ (a->tm_min - b->tm_min))
	  + (a->tm_sec - b->tm_sec));
}
#endif /* ! HAVE_TM_GMTOFF */



/* Return the week in the year specified by TP,
   with weeks starting on STARTING_DAY.  */
#ifdef	__GNUC__
inline
#endif
static unsigned int
week (tp, starting_day, max_preceding)
      const struct tm *const tp;
      int starting_day;
      int max_preceding;
{
  int wday, dl, base;

  wday = tp->tm_wday - starting_day;
  if (wday < 0)
    wday += 7;

  /* Set DL to the day in the year of the first day of the week
     containing the day specified in TP.  */
  dl = tp->tm_yday - wday;

  /* For the computation following ISO 8601:1988 we set the number of
     the week containing January 1st to 1 if this week has more than
     MAX_PRECEDING days in the new year.  For ISO 8601 this number is
     3, for the other representation it is 7 (i.e., not to be
     fulfilled).  */
  base = ((dl + 7) % 7) > max_preceding ? 1 : 0;

  /* If DL is negative we compute the result as 0 unless we have to
     compute it according ISO 8601.  In this case we have to return 53
     or 1 if the week containing January 1st has less than 4 days in
     the new year or not.  If DL is not negative we calculate the
     number of complete weeks for our week (DL / 7) plus 1 (because
     only for DL < 0 we are in week 0/53 and plus the number of the
     first week computed in the last step.  */
  return dl < 0 ? (dl < -max_preceding ? 53 : base)
		: base + 1 + dl / 7;
}

#ifndef _NL_CURRENT
static char const weekday_name[][10] =
  {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
static char const month_name[][10] =
  {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  };
#endif

/* Write information from TP into S according to the format
   string FORMAT, writing no more that MAXSIZE characters
   (including the terminating '\0') and returning number of
   characters written.  If S is NULL, nothing will be written
   anywhere, so to determine how many characters would be
   written, use NULL for S and (size_t) UINT_MAX for MAXSIZE.  */
size_t
strftime (s, maxsize, format, tp)
      char *s;
      size_t maxsize;
      const char *format;
      register const struct tm *tp;
{
  int hour12 = tp->tm_hour;
#ifdef _NL_CURRENT
  const char *const a_wkday = _NL_CURRENT (LC_TIME, ABDAY_1 + tp->tm_wday);
  const char *const f_wkday = _NL_CURRENT (LC_TIME, DAY_1 + tp->tm_wday);
  const char *const a_month = _NL_CURRENT (LC_TIME, ABMON_1 + tp->tm_mon);
  const char *const f_month = _NL_CURRENT (LC_TIME, MON_1 + tp->tm_mon);
  const char *const ampm = _NL_CURRENT (LC_TIME,
					hour12 > 11 ? PM_STR : AM_STR);
  size_t aw_len = strlen(a_wkday);
  size_t am_len = strlen(a_month);
  size_t ap_len = strlen (ampm);

  const char * const*alt_digits = &_NL_CURRENT (LC_TIME, ALT_DIGITS);
  int nr_alt_digits = (_NL_CURRENT (LC_TIME, ALT_DIGITS + 1) - *alt_digits);
#else
  const char *const f_wkday = weekday_name[tp->tm_wday];
  const char *const f_month = month_name[tp->tm_mon];
  const char *const a_wkday = f_wkday;
  const char *const a_month = f_month;
  const char *const ampm = "AMPM" + 2 * (hour12 > 11);
  size_t aw_len = 3;
  size_t am_len = 3;
  size_t ap_len = 2;
#endif
  size_t wkday_len = strlen (f_wkday);
  size_t month_len = strlen (f_month);
  const unsigned int y_week0 = week (tp, 0, 7);
  const unsigned int y_week1 = week (tp, 1, 7);
  const unsigned int y_week2 = week (tp, 1, 3);
  const char *zone;
  size_t zonelen;
  register size_t i = 0;
  register char *p = s;
  register const char *f;

  zone = 0;
#if HAVE_TM_ZONE
  zone = (const char *) tp->tm_zone;
#endif
#if HAVE_TZNAME
  if (!(zone && *zone) && tp->tm_isdst >= 0)
    zone = tzname[tp->tm_isdst];
#endif
  if (!(zone && *zone))
    zone = "???";

  zonelen = strlen (zone);

  if (hour12 > 12)
    hour12 -= 12;
  else
    if (hour12 == 0) hour12 = 12;

  for (f = format; *f != '\0'; ++f)
    {
      enum { pad_zero, pad_space, pad_none } pad; /* Padding for number.  */
      unsigned int digits;	/* Max digits for numeric format.  */
      unsigned int number_value; /* Numeric value to be printed.  */
      int negative_number;	/* 1 if the number is negative.  */
      const char *subfmt = "";
      enum { none, alternate, era } modifier;
      char *bufp;
      char buf[1 + (sizeof (int) < sizeof (time_t)
		    ? INT_STRLEN_BOUND (time_t)
		    : INT_STRLEN_BOUND (int))];

#if HAVE_MBLEN
      if (!isascii (*f))
	{
	  /* Non-ASCII, may be a multibyte.  */
	  int len = mblen (f, strlen (f));
	  if (len > 0)
	    {
	      cpy(len, f);
	      continue;
	    }
	}
#endif

      if (*f != '%')
	{
	  add (1, *p = *f);
	  continue;
	}

      /* Check for flags that can modify a number format.  */
      ++f;
      switch (*f)
	{
	case '_':
	  pad = pad_space;
	  ++f;
	  break;
	case '-':
	  pad = pad_none;
	  ++f;
	  break;
	default:
	  pad = pad_zero;
	  break;
	}

      /* Check for modifiers.  */
      switch (*f)
	{
	case 'E':
	  ++f;
	  modifier = era;
	  break;
	case 'O':
	  ++f;
	  modifier = alternate;
	  break;
	default:
	  modifier = none;
	  break;
	}

      /* Now do the specified format.  */
      switch (*f)
	{
#define DO_NUMBER(d, v) \
	  digits = d; number_value = v; goto do_number
#define DO_NUMBER_SPACEPAD(d, v) \
	  digits = d; number_value = v; goto do_number_spacepad

	case '\0':		/* GNU extension: % at end of format.  */
	    --f;
	    /* Fall through.  */
	case '%':
	  if (modifier != none)
	    goto bad_format;
	  add (1, *p = *f);
	  break;

	case 'a':
	  if (modifier != none)
	    goto bad_format;
	  cpy (aw_len, a_wkday);
	  break;

	case 'A':
	  if (modifier != none)
	    goto bad_format;
	  cpy (wkday_len, f_wkday);
	  break;

	case 'b':
	case 'h':		/* GNU extension.  */
	  if (modifier != none)
	    goto bad_format;
	  cpy (am_len, a_month);
	  break;

	case 'B':
	  if (modifier != none)
	    goto bad_format;
	  cpy (month_len, f_month);
	  break;

	case 'c':
	  if (modifier == alternate)
	    goto bad_format;
#ifdef _NL_CURRENT
	  if (modifier == era)
	    subfmt = _NL_CURRENT (LC_TIME, ERA_D_T_FMT);
	  if (*subfmt == '\0')
	    subfmt = _NL_CURRENT (LC_TIME, D_T_FMT);
#else
	  subfmt = "%a %b %e %H:%M:%S %Z %Y";
#endif

	subformat:
	  {
	    size_t len = strftime (p, maxsize - i, subfmt, tp);
	    if (len == 0 && *subfmt)
	      return 0;
	    add (len, ;);
	  }
	  break;

	case 'C':
	  if (modifier == alternate)
	    goto bad_format;
#ifdef _NL_CURRENT
	  /* XXX I'm not sure about this.  --drepper@gnu */
	  if (modifier == era &&
	      *(subfmt = _NL_CURRENT (LC_TIME, ERA)) != '\0')
	    goto subformat;
#endif
	  DO_NUMBER (2, (1900 + tp->tm_year) / 100);

	case 'x':
	  if (modifier == alternate)
	    goto bad_format;
#ifdef _NL_CURRENT
	  if (modifier == era)
	    subfmt = _NL_CURRENT (LC_TIME, ERA_D_FMT);
	  if (*subfmt == '\0')
	    subfmt = _NL_CURRENT (LC_TIME, D_FMT);
	  goto subformat;
#endif
	  /* Fall through.  */
	case 'D':		/* GNU extension.  */
	  subfmt = "%m/%d/%y";
	  goto subformat;

	case 'd':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, tp->tm_mday);

	case 'e':		/* GNU extension: %d, but blank-padded.  */
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER_SPACEPAD (2, tp->tm_mday);

	  /* All numeric formats set DIGITS and NUMBER_VALUE and then
	     jump to one of these two labels.  */

	do_number_spacepad:
	  /* Force `_' flag.  */
	  pad = pad_space;

	do_number:
	  /* Format the number according to the MODIFIER flag.  */

#ifdef _NL_CURRENT
	  if (modifier == alternate && 0 <= number_value
	      && number_value < (unsigned int) nr_alt_digits)
	    {
	      /* ALT_DIGITS is the first entry in an array with
		 alternative digit symbols.  */
	      size_t digitlen = strlen (*(alt_digits + number_value));
	      if (digitlen == 0)
		break;
	      cpy (digitlen, *(alt_digits + number_value));
	      goto done_with_number;
	    }
#endif
	  {
	    unsigned int u = number_value;

	    bufp = buf + sizeof (buf);
	    negative_number = number_value < 0;

	    if (negative_number)
	      u = -u;

	    do
	      *--bufp = u % 10 + '0';
	    while ((u /= 10) != 0);
  	  }

	do_number_sign_and_padding:
	  if (negative_number)
	    *--bufp = '-';

	  if (pad != pad_none)
	    {
	      int padding = digits - (buf + sizeof (buf) - bufp);

	      if (pad == pad_space)
		{
		  while (0 < padding--)
		    *--bufp = ' ';
		}
	      else
		{
		  bufp += negative_number;
		  while (0 < padding--)
		    *--bufp = '0';
		  if (negative_number)
		    *--bufp = '-';
		}
	    }

	  cpy (buf + sizeof (buf) - bufp, bufp);

#ifdef _NL_CURRENT
	done_with_number:
#endif
	  break;


	case 'H':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, tp->tm_hour);

	case 'I':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, hour12);

	case 'k':		/* GNU extension.  */
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER_SPACEPAD (2, tp->tm_hour);

	case 'l':		/* GNU extension.  */
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER_SPACEPAD (2, hour12);

	case 'j':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (3, 1 + tp->tm_yday);

	case 'M':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, tp->tm_min);

	case 'm':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, tp->tm_mon + 1);

	case 'n':		/* GNU extension.  */
	  add (1, *p = '\n');
	  break;

	case 'p':
	  cpy (ap_len, ampm);
	  break;

	case 'R':		/* GNU extension.  */
	  subfmt = "%H:%M";
	  goto subformat;

	case 'r':		/* GNU extension.  */
	  subfmt = "%I:%M:%S %p";
	  goto subformat;

	case 'S':
	  if (modifier == era)
	    return 0;

	  DO_NUMBER (2, tp->tm_sec);

	case 's':		/* GNU extension.  */
  	  {
	    struct tm ltm = *tp;
	    time_t t = mktime (&ltm);

	    /* Generate string value for T using time_t arithmetic;
	       this works even if sizeof (long) < sizeof (time_t).  */

	    bufp = buf + sizeof (buf);
	    negative_number = t < 0;

	    do
	      {
		int d = t % 10;
		t /= 10;

		if (negative_number)
		  {
		    d = -d;

		    /* Adjust if division truncates to minus infinity.  */
		    if (0 < -1 % 10 && d < 0)
		      {
			t++;
			d += 10;
		      }
		  }

		*--bufp = d + '0';
	      }
	    while (t != 0);

	    digits = 1;
	    goto do_number_sign_and_padding;
	  }

	case 'X':
	  if (modifier == alternate)
	    goto bad_format;
#ifdef _NL_CURRENT
	  if (modifier == era)
	    subfmt = _NL_CURRENT (LC_TIME, ERA_T_FMT);
	  if (*subfmt == '\0')
	    subfmt = _NL_CURRENT (LC_TIME, T_FMT);
	  goto subformat;
#endif
	  /* Fall through.  */
	case 'T':		/* GNU extension.  */
	  subfmt = "%H:%M:%S";
	  goto subformat;

	case 't':		/* GNU extension.  */
	  add (1, *p = '\t');
	  break;

	case 'U':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, y_week0);

	case 'V':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, y_week2);

	case 'W':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, y_week1);

	case 'w':
	  if (modifier == era)
	    goto bad_format;

	  DO_NUMBER (2, tp->tm_wday);

	case 'Y':
#ifdef _NL_CURRENT
	  if (modifier == era
	      && *(subfmt = _NL_CURRENT (LC_TIME, ERA_YEAR)) != '\0')
	    goto subformat;
	  else
#endif
	    if (modifier == alternate)
	      goto bad_format;
	    else
	      DO_NUMBER (4, 1900 + tp->tm_year);

	case 'y':
#ifdef _NL_CURRENT
	  if (modifier == era
	      && *(subfmt = _NL_CURRENT (LC_TIME, ERA_YEAR)) != '\0')
	    goto subformat;
#endif
	  DO_NUMBER (2, tp->tm_year % 100);

	case 'Z':
	  cpy(zonelen, zone);
	  break;

	case 'z':		/* GNU extension.  */
	  if (tp->tm_isdst < 0)
	    break;

	  {
	    int diff;
#if HAVE_TM_GMTOFF
	    diff = tp->tm_gmtoff;
#else
	    struct tm gtm;
	    struct tm ltm = *tp;
	    time_t lt = mktime (&ltm);

	    if (lt == (time_t) -1)
	      {
		/* mktime returns -1 for errors, but -1 is also a
		   valid time_t value.  Check whether an error really
		   occurred.  */
		struct tm tm;
		localtime_r (&lt, &tm);

		if ((ltm.tm_sec ^ tm.tm_sec)
		    | (ltm.tm_min ^ tm.tm_min)
		    | (ltm.tm_hour ^ tm.tm_hour)
		    | (ltm.tm_mday ^ tm.tm_mday)
		    | (ltm.tm_mon ^ tm.tm_mon)
		    | (ltm.tm_year ^ tm.tm_year))
		  break;
	      }

	    if (! gmtime_r (&lt, &gtm))
	      break;

	    diff = tm_diff (&ltm, &gtm);
#endif

	    if (diff < 0)
	      {
		add (1, *p = '-');
		diff = -diff;
	      }
	    else
	      add (1, *p = '+');

	    pad = pad_zero;

	    diff /= 60;
	    DO_NUMBER (4, (diff / 60) * 100 + diff % 60);
	  }

	default:
	  /* Bad format.  */
	bad_format:
	  if (pad == pad_space)
	    add (1, *p = '_');
	  else if (pad == pad_zero)
	    add (1, *p = '0');

	  if (modifier == era)
	    add (1, *p = 'E');
	  else if (modifier == alternate)
	    add (1, *p = 'O');

	  add (1, *p = *f);
	  break;
	}
    }

  if (p)
    *p = '\0';
  return i;
}
