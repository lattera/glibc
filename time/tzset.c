/* Copyright (C) 1991-2002,2003,2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <ctype.h>
#include <errno.h>
#include <bits/libc-lock.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define NOID
#include <timezone/tzfile.h>

char *__tzname[2] = { (char *) "GMT", (char *) "GMT" };
int __daylight = 0;
long int __timezone = 0L;

weak_alias (__tzname, tzname)
weak_alias (__daylight, daylight)
weak_alias (__timezone, timezone)

/* This locks all the state variables in tzfile.c and this file.  */
__libc_lock_define_initialized (static, tzset_lock)


#define	min(a, b)	((a) < (b) ? (a) : (b))
#define	max(a, b)	((a) > (b) ? (a) : (b))
#define	sign(x)		((x) < 0 ? -1 : 1)


/* This structure contains all the information about a
   timezone given in the POSIX standard TZ envariable.  */
typedef struct
  {
    const char *name;

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


static void compute_change (tz_rule *rule, int year) __THROW internal_function;
static void tz_compute (const struct tm *tm) __THROW internal_function;
static void tzset_internal (int always, int explicit)
     __THROW internal_function;

/* List of buffers containing time zone strings. */
struct tzstring_l
{
  struct tzstring_l *next;
  size_t len;  /* strlen(data) - doesn't count terminating NUL! */
  char data[0];
};

static struct tzstring_l *tzstring_list;

/* Allocate a permanent home for S.  It will never be moved or deallocated,
   but may share space with other strings.
   Don't modify the returned string. */
char *
__tzstring (const char *s)
{
  char *p;
  struct tzstring_l *t, *u, *new;
  size_t len = strlen(s);

  /* Walk the list and look for a match.  If this string is the same
     as the end of an already-allocated string, it can share space. */
  for (u = t = tzstring_list; t; u = t, t = t->next)
    if (len <= t->len)
      {
	p = &t->data[t->len - len];
	if (strcmp (s, p) == 0)
	  return p;
      }

  /* Not found; allocate a new buffer. */
  new = malloc (sizeof (struct tzstring_l) + len + 1);
  if (!new)
    return NULL;

  new->next = NULL;
  new->len = len;
  strcpy (new->data, s);

  if (u)
    u->next = new;
  else
    tzstring_list = new;

  return new->data;
}

/* Maximum length of a timezone name.  tzset_internal keeps this up to date
   (never decreasing it) when ! __use_tzfile.
   tzfile.c keeps it up to date when __use_tzfile.  */
size_t __tzname_cur_max;

long int
__tzname_max ()
{
  __libc_lock_lock (tzset_lock);

  tzset_internal (0, 0);

  __libc_lock_unlock (tzset_lock);

  return __tzname_cur_max;
}

static char *old_tz;

/* Interpret the TZ envariable.  */
static void
internal_function
tzset_internal (always, explicit)
     int always;
     int explicit;
{
  static int is_initialized;
  register const char *tz;
  register size_t l;
  char *tzbuf;
  unsigned short int hh, mm, ss;
  unsigned short int whichrule;

  if (is_initialized && !always)
    return;
  is_initialized = 1;

  /* Examine the TZ environment variable.  */
  tz = getenv ("TZ");
  if (tz == NULL && !explicit)
    /* Use the site-wide default.  This is a file name which means we
       would not see changes to the file if we compare only the file
       name for change.  We want to notice file changes if tzset() has
       been called explicitly.  Leave TZ as NULL in this case.  */
    tz = TZDEFAULT;
  if (tz && *tz == '\0')
    /* User specified the empty string; use UTC explicitly.  */
    tz = "Universal";

  /* A leading colon means "implementation defined syntax".
     We ignore the colon and always use the same algorithm:
     try a data file, and if none exists parse the 1003.1 syntax.  */
  if (tz && *tz == ':')
    ++tz;

  /* Check whether the value changes since the last run.  */
  if (old_tz != NULL && tz != NULL && strcmp (tz, old_tz) == 0)
    /* No change, simply return.  */
    return;

  if (tz == NULL)
    /* No user specification; use the site-wide default.  */
    tz = TZDEFAULT;

  tz_rules[0].name = NULL;
  tz_rules[1].name = NULL;

  /* Save the value of `tz'.  */
  if (old_tz != NULL)
    free (old_tz);
  old_tz = tz ? __strdup (tz) : NULL;

  /* Try to read a data file.  */
  __tzfile_read (tz, 0, NULL);
  if (__use_tzfile)
    return;

  /* No data file found.  Default to UTC if nothing specified.  */

  if (tz == NULL || *tz == '\0'
      || (TZDEFAULT != NULL && strcmp (tz, TZDEFAULT) == 0))
    {
      tz_rules[0].name = tz_rules[1].name = "UTC";
      tz_rules[0].type = tz_rules[1].type = J0;
      tz_rules[0].m = tz_rules[0].n = tz_rules[0].d = 0;
      tz_rules[1].m = tz_rules[1].n = tz_rules[1].d = 0;
      tz_rules[0].secs = tz_rules[1].secs = 0;
      tz_rules[0].offset = tz_rules[1].offset = 0L;
      tz_rules[0].change = tz_rules[1].change = (time_t) -1;
      tz_rules[0].computed_for = tz_rules[1].computed_for = 0;
      goto out;
    }

  /* Clear out old state and reset to unnamed UTC.  */
  memset (tz_rules, 0, sizeof tz_rules);
  tz_rules[0].name = tz_rules[1].name = "";

  /* Get the standard timezone name.  */
  tzbuf = strdupa (tz);

  if (sscanf (tz, "%[^0-9,+-]", tzbuf) != 1 ||
      (l = strlen (tzbuf)) < 3)
    goto out;

  tz_rules[0].name = __tzstring (tzbuf);

  tz += l;

  /* Figure out the standard offset from UTC.  */
  if (*tz == '\0' || (*tz != '+' && *tz != '-' && !isdigit (*tz)))
    goto out;

  if (*tz == '-' || *tz == '+')
    tz_rules[0].offset = *tz++ == '-' ? 1L : -1L;
  else
    tz_rules[0].offset = -1L;
  switch (sscanf (tz, "%hu:%hu:%hu", &hh, &mm, &ss))
    {
    default:
      tz_rules[0].offset = 0;
      goto out;
    case 1:
      mm = 0;
    case 2:
      ss = 0;
    case 3:
      break;
    }
  tz_rules[0].offset *= (min (ss, 59) + (min (mm, 59) * 60) +
			 (min (hh, 24) * 60 * 60));

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
      char *n = tzbuf + strlen (tzbuf) + 1;
      if (sscanf (tz, "%[^0-9,+-]", n) != 1 ||
	  (l = strlen (n)) < 3)
	goto done_names;	/* Punt on name, set up the offsets.  */

      tz_rules[1].name = __tzstring (n);

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
	  tz_rules[1].offset *= (min (ss, 59) + (min (mm, 59) * 60) +
				 (min (hh, 23) * (60 * 60)));
	  break;
	}
      for (l = 0; l < 3; ++l)
	{
	  while (isdigit (*tz))
	    ++tz;
	  if (l < 2 && *tz == ':')
	    ++tz;
	}
      if (*tz == '\0' || (tz[0] == ',' && tz[1] == '\0'))
	{
	  /* There is no rule.  See if there is a default rule file.  */
	  __tzfile_default (tz_rules[0].name, tz_rules[1].name,
			    tz_rules[0].offset, tz_rules[1].offset);
	  if (__use_tzfile)
	    {
	      free (old_tz);
	      old_tz = NULL;
	      return;
	    }
	}
    }
  else
    {
      /* There is no DST.  */
      tz_rules[1].name = tz_rules[0].name;
      tz_rules[1].offset = tz_rules[0].offset;
      goto out;
    }

 done_names:
  /* Figure out the standard <-> DST rules.  */
  for (whichrule = 0; whichrule < 2; ++whichrule)
    {
      register tz_rule *tzr = &tz_rules[whichrule];

      /* Ignore comma to support string following the incorrect
	 specification in early POSIX.1 printings.  */
      tz += *tz == ',';

      /* Get the date of the change.  */
      if (*tz == 'J' || isdigit (*tz))
	{
	  char *end;
	  tzr->type = *tz == 'J' ? J1 : J0;
	  if (tzr->type == J1 && !isdigit (*++tz))
	    goto out;
	  tzr->d = (unsigned short int) strtoul (tz, &end, 10);
	  if (end == tz || tzr->d > 365)
	    goto out;
	  else if (tzr->type == J1 && tzr->d == 0)
	    goto out;
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
	    goto out;
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
	goto out;

      if (*tz != '\0' && *tz != '/' && *tz != ',')
	goto out;
      else if (*tz == '/')
	{
	  /* Get the time of day of the change.  */
	  ++tz;
	  if (*tz == '\0')
	    goto out;
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
	      while (isdigit (*tz))
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

 out:
  __daylight = tz_rules[0].offset != tz_rules[1].offset;
  __timezone = -tz_rules[0].offset;
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
}

/* Figure out the exact time (as a time_t) in YEAR
   when the change described by RULE will occur and
   put it in RULE->change, saving YEAR in RULE->computed_for.  */
static void
internal_function
compute_change (rule, year)
     tz_rule *rule;
     int year;
{
  register time_t t;

  if (year != -1 && rule->computed_for == year)
    /* Operations on times in 2 BC will be slower.  Oh well.  */
    return;

  /* First set T to January 1st, 0:00:00 GMT in YEAR.  */
  if (year > 1970)
    t = ((year - 1970) * 365
	 + /* Compute the number of leapdays between 1970 and YEAR
	      (exclusive).  There is a leapday every 4th year ...  */
	 + ((year - 1) / 4 - 1970 / 4)
	 /* ... except every 100th year ... */
	 - ((year - 1) / 100 - 1970 / 100)
	 /* ... but still every 400th year.  */
	 + ((year - 1) / 400 - 1970 / 400)) * SECSPERDAY;
  else
    t = 0;

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
	unsigned int i;
	int d, m1, yy0, yy1, yy2, dow;
	const unsigned short int *myday =
	  &__mon_yday[__isleap (year)][rule->m];

	/* First add SECSPERDAY for each day in months before M.  */
	t += myday[-1] * SECSPERDAY;

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
	    if (d + 7 >= (int) myday[0] - myday[-1])
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

  rule->change = t - rule->offset + rule->secs;
  rule->computed_for = year;
}


/* Figure out the correct timezone for TM and set `__tzname',
   `__timezone', and `__daylight' accordingly.  */
static void
internal_function
tz_compute (tm)
     const struct tm *tm;
{
  compute_change (&tz_rules[0], 1900 + tm->tm_year);
  compute_change (&tz_rules[1], 1900 + tm->tm_year);
}

/* Reinterpret the TZ environment variable and set `tzname'.  */
#undef tzset

void
__tzset (void)
{
  __libc_lock_lock (tzset_lock);

  tzset_internal (1, 1);

  if (!__use_tzfile)
    {
      /* Set `tzname'.  */
      __tzname[0] = (char *) tz_rules[0].name;
      __tzname[1] = (char *) tz_rules[1].name;
    }

  __libc_lock_unlock (tzset_lock);
}
weak_alias (__tzset, tzset)

/* Return the `struct tm' representation of *TIMER in the local timezone.
   Use local time if USE_LOCALTIME is nonzero, UTC otherwise.  */
struct tm *
__tz_convert (const time_t *timer, int use_localtime, struct tm *tp)
{
  long int leap_correction;
  int leap_extra_secs;

  if (timer == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  __libc_lock_lock (tzset_lock);

  /* Update internal database according to current TZ setting.
     POSIX.1 8.3.7.2 says that localtime_r is not required to set tzname.
     This is a good idea since this allows at least a bit more parallelism.
     By analogy we apply the same rule to gmtime_r.  */
  tzset_internal (tp == &_tmbuf, 0);

  if (__use_tzfile)
    __tzfile_compute (*timer, use_localtime, &leap_correction,
		      &leap_extra_secs, tp);
  else
    {
      if (! __offtime (timer, 0, tp))
	tp = NULL;
      else
	tz_compute (tp);
      leap_correction = 0L;
      leap_extra_secs = 0;
    }

  if (tp)
    {
      if (use_localtime)
	{
	  if (!__use_tzfile)
	    {
	      int isdst;

	      /* We have to distinguish between northern and southern
		 hemisphere.  For the latter the daylight saving time
		 ends in the next year.  */
	      if (__builtin_expect (tz_rules[0].change
				    > tz_rules[1].change, 0))
		isdst = (*timer < tz_rules[1].change
			 || *timer >= tz_rules[0].change);
	      else
		isdst = (*timer >= tz_rules[0].change
			 && *timer < tz_rules[1].change);
	      tp->tm_isdst = isdst;
	      tp->tm_zone = __tzname[isdst];
	      tp->tm_gmtoff = tz_rules[isdst].offset;
	    }
	}
      else
	{
	  tp->tm_isdst = 0;
	  tp->tm_zone = "GMT";
	  tp->tm_gmtoff = 0L;
	}

      if (__offtime (timer, tp->tm_gmtoff - leap_correction, tp))
        tp->tm_sec += leap_extra_secs;
      else
	tp = NULL;
    }

  __libc_lock_unlock (tzset_lock);

  return tp;
}


libc_freeres_fn (free_mem)
{
  while (tzstring_list != NULL)
    {
      struct tzstring_l *old = tzstring_list;

      tzstring_list = tzstring_list->next;
      free (old);
    }
  free (old_tz);
  old_tz = NULL;
}
