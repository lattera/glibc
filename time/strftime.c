/* Extensions for GNU date that are still missing here:
   -
   _
*/

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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _LIBC
# define HAVE_LIMITS_H 1
# define HAVE_MBLEN 1
# define HAVE_TM_ZONE 1
# define STDC_HEADERS 1
# include <ansidecl.h>
# include "../locale/localeinfo.h"
#endif

#include <stdio.h>
#include <sys/types.h>		/* Some systems define `time_t' here.  */
#include <time.h>

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

static unsigned int week __P((const struct tm *const, int));


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
#define	cpy(n, s)	add((n), memcpy((PTR) p, (PTR) (s), (n)))

#ifdef _LIBC
#define	fmt(n, args)	add((n), if (sprintf args != (n)) return 0)
#else
#define	fmt(n, args)	add((n), sprintf args; if (strlen (p) != (n)) return 0)
#endif

/* Return the week in the year specified by TP,
   with weeks starting on STARTING_DAY.  */
#ifdef	__GNUC__
inline
#endif
static unsigned int
week (tp, starting_day)
      const struct tm *const tp;
      int starting_day;
{
  int wday, dl;

  wday = tp->tm_wday - starting_day;
  if (wday < 0)
    wday += 7;

  /* Set DL to the day in the year of the last day of the week previous to the
     one containing the day specified in TP.  If DL is negative or zero, the
     day specified in TP is in the first week of the year.  Otherwise,
     calculate the number of complete weeks before our week (DL / 7) and
     add any partial week at the start of the year (DL % 7).  */
  dl = tp->tm_yday - wday;
  return dl <= 0 ? 0 : ((dl / 7) + ((dl % 7) == 0 ? 0 : 1));
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
					hour12 > 12 ? PM_STR : AM_STR);
  size_t aw_len = strlen(a_wkday);
  size_t am_len = strlen(a_month);
  size_t ap_len = strlen (ampm);
#else
  const char *const f_wkday = weekday_name[tp->tm_wday];
  const char *const f_month = month_name[tp->tm_mon];
  const char *const a_wkday = f_wkday;
  const char *const a_month = f_month;
  const char *const ampm = "AMPM" + 2 * (hour12 > 12);
  size_t aw_len = 3;
  size_t am_len = 3;
  size_t ap_len = 2;
#endif
  size_t wkday_len = strlen(f_wkday);
  size_t month_len = strlen(f_month);
  const unsigned int y_week0 = week (tp, 0);
  const unsigned int y_week1 = week (tp, 1);
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
      const char *subfmt;

#if HAVE_MBLEN
      if (!isascii(*f))
	{
	  /* Non-ASCII, may be a multibyte.  */
	  int len = mblen(f, strlen(f));
	  if (len > 0)
	    {
	      cpy(len, f);
	      continue;
	    }
	}
#endif

      if (*f != '%')
	{
	  add(1, *p = *f);
	  continue;
	}

      ++f;
      switch (*f)
	{
	case '\0':
	case '%':
	  add(1, *p = *f);
	  break;

	case 'a':
	  cpy(aw_len, a_wkday);
	  break;

	case 'A':
	  cpy(wkday_len, f_wkday);
	  break;

	case 'b':
	case 'h':		/* GNU extension.  */
	  cpy(am_len, a_month);
	  break;

	case 'B':
	  cpy(month_len, f_month);
	  break;

	case 'c':
#ifdef _NL_CURRENT
	  subfmt = _NL_CURRENT (LC_TIME, D_T_FMT);
#else
	  subfmt = "%a %b %d %H:%M:%S %Z %Y";
#endif
	subformat:
	  {
	    size_t len = strftime (p, maxsize - i, subfmt, tp);
	    if (len == 0 && *subfmt)
	      return 0;
	    add(len, );
	  }
	  break;

	case 'C':
	  fmt (2, (p, "%02d", (1900 + tp->tm_year) / 100));
	  break;

	case 'x':
#ifdef _NL_CURRENT
	  subfmt = _NL_CURRENT (LC_TIME, D_FMT);
	  goto subformat;
#endif
	  /* Fall through.  */
	case 'D':		/* GNU extension.  */
	  subfmt = "%m/%d/%y";
	  goto subformat;

	case 'd':
	  fmt(2, (p, "%02d", tp->tm_mday));
	  break;

	case 'e':		/* GNU extension: %d, but blank-padded.  */
	  fmt(2, (p, "%2d", tp->tm_mday));
	  break;

	case 'H':
	  fmt(2, (p, "%02d", tp->tm_hour));
	  break;

	case 'I':
	  fmt(2, (p, "%02d", hour12));
	  break;

	case 'k':		/* GNU extension.  */
	  fmt(2, (p, "%2d", tp->tm_hour));
	  break;

	case 'l':		/* GNU extension.  */
	  fmt(2, (p, "%2d", hour12));
	  break;

	case 'j':
	  fmt(3, (p, "%03d", 1 + tp->tm_yday));
	  break;

	case 'M':
	  fmt(2, (p, "%02d", tp->tm_min));
	  break;

	case 'm':
	  fmt(2, (p, "%02d", tp->tm_mon + 1));
	  break;

	case 'n':		/* GNU extension.  */
	  add (1, *p = '\n');
	  break;

	case 'p':
	  cpy(ap_len, ampm);
	  break;

	case 'R':		/* GNU extension.  */
	  subfmt = "%H:%M";
	  goto subformat;

	case 'r':		/* GNU extension.  */
	  subfmt = "%I:%M:%S %p";
	  goto subformat;

	case 'S':
	  fmt(2, (p, "%02d", tp->tm_sec));
	  break;

	case 'X':
#ifdef _NL_CURRENT
	  subfmt = _NL_CURRENT (LC_TIME, T_FMT);
	  goto subformat;
#endif
	  /* Fall through.  */
	case 'T':		/* GNU extenstion.  */
	  subfmt = "%H:%M:%S";
	  goto subformat;

	case 't':		/* GNU extenstion.  */
	  add (1, *p = '\t');
	  break;

	case 'U':
	  fmt(2, (p, "%02u", y_week0));
	  break;

	case 'W':
	  fmt(2, (p, "%02u", y_week1));
	  break;

	case 'w':
	  fmt(2, (p, "%02d", tp->tm_wday));
	  break;

	case 'Y':
	  fmt(4, (p, "%04d", 1900 + tp->tm_year));
	  break;

	case 'y':
	  fmt(2, (p, "%02d", tp->tm_year % 100));
	  break;

	case 'Z':
	  cpy(zonelen, zone);
	  break;

	default:
	  /* Bad format.  */
	  break;
	}
    }

  if (p)
    *p = '\0';
  return i;
}
