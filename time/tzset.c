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

#include <ansidecl.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Defined in mktime.c.  */
extern CONST unsigned short int __mon_lengths[2][12];

#define NOID
#include "tzfile.h"

extern int __use_tzfile;
extern void EXFUN(__tzfile_read, (CONST char *file));
extern void EXFUN(__tzfile_default, (char *std AND char *dst AND
				     long int stdoff AND long int dstoff));
extern int EXFUN(__tzfile_compute, (time_t, struct tm));

#ifndef	HAVE_WEAK_SYMBOLS
#define	__tzname	tzname
#define	__daylight	daylight
#define	__timezone	timezone
#else
weak_alias (__tzname, tzname)
weak_alias (__daylight, daylight)
weak_alias (__timezone, timezone)
#endif

char *__tzname[2] = { (char *) "GMT", (char *) "GMT" };
int __daylight = 0;
long int __timezone = 0L;


#define	min(a, b)	((a) < (b) ? (a) : (b))
#define	max(a, b)	((a) > (b) ? (a) : (b))
#define	sign(x)		((x) < 0 ? -1 : 1)


/* This structure contains all the information about a
   timezone given in the POSIX standard TZ envariable.  */
typedef struct
  {
    char *name;

    /* When to change.  */
    enum { J0, J1, M } type;	/* Interpretation of:  */
    unsigned short int m, n, d;	/* Month, week, day.  */
    unsigned int secs;		/* Time of day.  */

    long int offset;		/* Seconds east of GMT (west if < 0).  */

    /* We cache the computed time of change for a
       given year so we don't have to recompute it.  */
    time_t change;	/* When to change to this zone.  */
    int computed_for;	/* Year above is computed for.  */
  } tz_rule;

/* tz_rules[0] is standard, tz_rules[1] is daylight.  */
static tz_rule tz_rules[2];

int __tzset_run = 0;

/* Interpret the TZ envariable.  */
void
DEFUN_VOID(__tzset)
{
  register CONST char *tz;
  register size_t l;
  unsigned short int hh, mm, ss;
  unsigned short int whichrule;

  /* Free old storage.  */
  if (tz_rules[0].name != NULL && *tz_rules[0].name != '\0')
    free((PTR) tz_rules[0].name);
  if (tz_rules[1].name != NULL && *tz_rules[1].name != '\0' &&
      tz_rules[1].name != tz_rules[0].name)
    free((PTR) tz_rules[1].name);

  tz = getenv("TZ");

  if (tz != NULL && *tz == ':')
    {
      __tzfile_read(tz + 1);
      if (__use_tzfile)
	{
	  __tzset_run = 1;
	  return;
	}
      else
	tz = NULL;
    }

  if (tz == NULL || *tz == '\0')
    {
      __tzfile_read((char *) NULL);
      if (!__use_tzfile)
	{
	  const char UTC[] = "UTC";
	  size_t len = sizeof UTC;
	  tz_rules[0].name = (char *) malloc(len);
	  if (tz_rules[0].name == NULL)
	    return;
	  tz_rules[1].name = (char *) malloc(len);
	  if (tz_rules[1].name == NULL)
	    return;
	  memcpy ((PTR) tz_rules[0].name, UTC, len);
	  memcpy ((PTR) tz_rules[1].name, UTC, len);
	  tz_rules[0].type = tz_rules[1].type = J0;
	  tz_rules[0].m = tz_rules[0].n = tz_rules[0].d = 0;
	  tz_rules[1].m = tz_rules[1].n = tz_rules[1].d = 0;
	  tz_rules[0].secs = tz_rules[1].secs = 0;
	  tz_rules[0].offset = tz_rules[1].offset = 0L;
	  tz_rules[0].change = tz_rules[1].change = (time_t) -1;
	  tz_rules[0].computed_for = tz_rules[1].computed_for = 0;
	}
      __tzset_run = 1;
      return;
    }

  /* Clear out old state and reset to unnamed UTC.  */
  memset (tz_rules, 0, sizeof tz_rules);
  tz_rules[0].name = tz_rules[1].name = (char *) "";

  /* Get the standard timezone name.  */
  tz_rules[0].name = (char *) malloc (strlen (tz) + 1);
  if (tz_rules[0].name == NULL)
    /* Don't set __tzset_run so we will try again.  */
    return;

  if (sscanf(tz, "%[^0-9,+-]", tz_rules[0].name) != 1 ||
      (l = strlen(tz_rules[0].name)) < 3)
    {
      free (tz_rules[0].name);
      tz_rules[0].name = (char *) "";
      return;
    }

  {
    char *n = realloc ((PTR) tz_rules[0].name, l + 1);
    if (n != NULL)
      tz_rules[0].name = n;
  }

  tz += l;

  /* Figure out the standard offset from UTC.  */
  if (*tz == '\0' || (*tz != '+' && *tz != '-' && !isdigit(*tz)))
    return;

  if (*tz == '-' || *tz == '+')
    tz_rules[0].offset = *tz++ == '-' ? 1L : -1L;
  else
    tz_rules[0].offset = -1L;
  switch (sscanf (tz, "%hu:%hu:%hu", &hh, &mm, &ss))
    {
    default:
      return;
    case 1:
      mm = 0;
    case 2:
      ss = 0;
    case 3:
      break;
    }
  tz_rules[0].offset *= (min(ss, 59) + (min(mm, 59) * 60) +
			 (min(hh, 12) * 60 * 60));

  for (l = 0; l < 3; ++l)
    {
      while (isdigit(*tz))
	++tz;
      if (l < 2 && *tz == ':')
	++tz;
    }

  /* Get the DST timezone name (if any).  */
  if (*tz != '\0')
    {
      char *n = malloc (strlen(tz) + 1);
      if (n != NULL)
	{
	  tz_rules[1].name = n;
	  if (sscanf(tz, "%[^0-9,+-]", tz_rules[1].name) != 1 ||
	      (l = strlen(tz_rules[1].name)) < 3)
	    {
	      free (n);
	      tz_rules[1].name = (char *) "";
	      goto done_names;	/* Punt on name, set up the offsets.  */
	    }
	  n = realloc ((PTR) tz_rules[1].name, l + 1);
	  if (n != NULL)
	    tz_rules[1].name = n;
	}
    }

  tz += l;

  /* Figure out the DST offset from GMT.  */
  if (*tz == '-' || *tz == '+')
    tz_rules[1].offset = *tz++ == '-' ? 1L : -1L;
  else
    tz_rules[1].offset = -1L;

  switch (sscanf (tz, "%hu:%hu:%hu", &hh, &mm, &ss))
    {
    default:
      /* Default to one hour later than standard time.  */
      tz_rules[1].offset = tz_rules[0].offset + (60 * 60);
      break;

    case 1:
      mm = 0;
    case 2:
      ss = 0;
    case 3:
      tz_rules[1].offset *= (min(ss, 59) + (min(mm, 59) * 60) +
			     (min(hh, 12) * (60 * 60)));
      break;
    }
  for (l = 0; l < 3; ++l)
    {
      while (isdigit (*tz))
	++tz;
      if (l < 2 && *tz == ':')
	++tz;
    }

 done_names:

  if (*tz == '\0' || (tz[0] == ',' && tz[1] == '\0'))
    {
      /* There is no rule.  See if there is a default rule file.  */
      __tzfile_default (tz_rules[0].name, tz_rules[1].name,
			tz_rules[0].offset, tz_rules[1].offset);
      if (__use_tzfile)
	{
	  __tzset_run = 1;
	  return;
	}
    }

  /* Figure out the standard <-> DST rules.  */
  for (whichrule = 0; whichrule < 2; ++whichrule)
    {
      register tz_rule *tzr = &tz_rules[whichrule];
      
      if (*tz == ',')
	{
	  ++tz;
	  if (*tz == '\0')
	    return;
	}
      
      /* Get the date of the change.  */
      if (*tz == 'J' || isdigit (*tz))
	{
	  char *end;
	  tzr->type = *tz == 'J' ? J1 : J0;
	  if (tzr->type == J1 && !isdigit (*++tz))
	    return;
	  tzr->d = (unsigned short int) strtoul (tz, &end, 10);
	  if (end == tz || tzr->d > 365)
	    return;
	  else if (tzr->type == J1 && tzr->d == 0)
	    return;
	  tz = end;
	}
      else if (*tz == 'M')
	{
	  int n;
	  tzr->type = M;
	  if (sscanf (tz, "M%hu.%hu.%hu%n",
		      &tzr->m, &tzr->n, &tzr->d, &n) != 3 ||
	      tzr->m < 1 || tzr->m > 12 ||
	      tzr->n < 1 || tzr->n > 5 || tzr->d > 6)
	    return;
	  tz += n;
	}
      else if (*tz == '\0')
	{
	  /* United States Federal Law, the equivalent of "M4.1.0,M10.5.0".  */
	  tzr->type = M;
	  if (tzr == &tz_rules[0])
	    {
	      tzr->m = 4;
	      tzr->n = 1;
	      tzr->d = 0;
	    }
	  else
	    {
	      tzr->m = 10;
	      tzr->n = 5;
	      tzr->d = 0;
	    }
	}
      else
	return;
      
      if (*tz != '\0' && *tz != '/' && *tz != ',')
	return;
      else if (*tz == '/')
	{
	  /* Get the time of day of the change.  */
	  ++tz;
	  if (*tz == '\0')
	    return;
	  switch (sscanf (tz, "%hu:%hu:%hu", &hh, &mm, &ss))
	    {
	    default:
	      hh = 2;		/* Default to 2:00 AM.  */
	    case 1:
	      mm = 0;
	    case 2:
	      ss = 0;
	    case 3:
	      break;
	    }
	  for (l = 0; l < 3; ++l)
	    {
	      while (isdigit(*tz))
		++tz;
	      if (l < 2 && *tz == ':')
		++tz;
	    }
	  tzr->secs = (hh * 60 * 60) + (mm * 60) + ss;
	}
      else
	/* Default to 2:00 AM.  */
	tzr->secs = 2 * 60 * 60;

      tzr->computed_for = -1;
    }

  __tzset_run = 1;
}

/* Maximum length of a timezone name.  __tz_compute keeps this up to date
   (never decreasing it) when ! __use_tzfile.
   tzfile.c keeps it up to date when __use_tzfile.  */
long int __tzname_cur_max;

long int
DEFUN_VOID(__tzname_max)
{
  if (! __tzset_run)
    __tzset ();

  return __tzname_cur_max;
}

/* Figure out the exact time (as a time_t) in YEAR
   when the change described by RULE will occur and
   put it in RULE->change, saving YEAR in RULE->computed_for.
   Return nonzero if successful, zero on failure.  */
static int
DEFUN(compute_change, (rule, year), tz_rule *rule AND int year)
{
  register time_t t;
  int y;

  if (year != -1 && rule->computed_for == year)
    /* Operations on times in 1969 will be slower.  Oh well.  */
    return 1;
     
  /* First set T to January 1st, 0:00:00 GMT in YEAR.  */
  t = 0;
  for (y = 1970; y < year; ++y)
    t += SECSPERDAY * (__isleap (y) ? 366 : 365);

  switch (rule->type)
    {
    case J1:
      /* Jn - Julian day, 1 == January 1, 60 == March 1 even in leap years.
	 In non-leap years, or if the day number is 59 or less, just
	 add SECSPERDAY times the day number-1 to the time of
	 January 1, midnight, to get the day.  */
      t += (rule->d - 1) * SECSPERDAY;
      if (rule->d >= 60 && __isleap (year))
	t += SECSPERDAY;
      break;

    case J0:
      /* n - Day of year.
	 Just add SECSPERDAY times the day number to the time of Jan 1st.  */
      t += rule->d * SECSPERDAY;
      break;

    case M:
      /* Mm.n.d - Nth "Dth day" of month M.  */
      {
	register int i, d, m1, yy0, yy1, yy2, dow;

	/* First add SECSPERDAY for each day in months before M.  */
	for (i = 0; i < rule->m - 1; ++i)
	  t += __mon_lengths[__isleap (year)][i] * SECSPERDAY;

	/* Use Zeller's Congruence to get day-of-week of first day of month. */
	m1 = (rule->m + 9) % 12 + 1;
	yy0 = (rule->m <= 2) ? (year - 1) : year;
	yy1 = yy0 / 100;
	yy2 = yy0 % 100;
	dow = ((26 * m1 - 2) / 10 + 1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
	if (dow < 0)
	  dow += 7;

	/* DOW is the day-of-week of the first day of the month.  Get the
	   day-of-month (zero-origin) of the first DOW day of the month.  */
	d = rule->d - dow;
	if (d < 0)
	  d += 7;
	for (i = 1; i < rule->n; ++i)
	  {
	    if (d + 7 >= __mon_lengths[__isleap (year)][rule->m - 1])
	      break;
	    d += 7;
	  }

	/* D is the day-of-month (zero-origin) of the day we want.  */
	t += d * SECSPERDAY;
      }
      break;
    }

  /* T is now the Epoch-relative time of 0:00:00 GMT on the day we want.
     Just add the time of day and local offset from GMT, and we're done.  */

  rule->change = t + rule->offset + rule->secs;
  rule->computed_for = year;
  return 1;
}


/* Figure out the correct timezone for *TIMER and TM (which must be the same)
   and set `__tzname', `__timezone', and `__daylight' accordingly.
   Return nonzero on success, zero on failure.  */
int
DEFUN(__tz_compute, (timer, tm),
      time_t timer AND const struct tm *tm)
{
  if (! __tzset_run)
    __tzset ();

  if (! compute_change (&tz_rules[0], 1900 + tm->tm_year) ||
      ! compute_change (&tz_rules[1], 1900 + tm->tm_year))
    return 0;

  __daylight = timer >= tz_rules[0].change && timer < tz_rules[1].change;
  __timezone = tz_rules[__daylight ? 1 : 0].offset;
  __tzname[0] = (char *) tz_rules[0].name;
  __tzname[1] = (char *) tz_rules[1].name;

  {
    /* Keep __tzname_cur_max up to date.  */
    size_t len0 = strlen (__tzname[0]);
    size_t len1 = strlen (__tzname[1]);
    if (len0 > __tzname_cur_max)
      __tzname_cur_max = len0;
    if (len1 > __tzname_cur_max)
      __tzname_cur_max = len1;
  }

  return 1;
}

weak_alias (__tzset, tzset)
