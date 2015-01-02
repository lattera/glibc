/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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
    int secs;			/* Time of day.  */

    long int offset;		/* Seconds east of GMT (west if < 0).  */

    /* We cache the computed time of change for a
       given year so we don't have to recompute it.  */
    time_t change;	/* When to change to this zone.  */
    int computed_for;	/* Year above is computed for.  */
  } tz_rule;

/* tz_rules[0] is standard, tz_rules[1] is daylight.  */
static tz_rule tz_rules[2];


static void compute_change (tz_rule *rule, int year) __THROW internal_function;
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
  size_t len = strlen (s);

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
__tzname_max (void)
{
  __libc_lock_lock (tzset_lock);

  tzset_internal (0, 0);

  __libc_lock_unlock (tzset_lock);

  return __tzname_cur_max;
}

static char *old_tz;

static void
internal_function
update_vars (void)
{
  __daylight = tz_rules[0].offset != tz_rules[1].offset;
  __timezone = -tz_rules[0].offset;
  __tzname[0] = (char *) tz_rules[0].name;
  __tzname[1] = (char *) tz_rules[1].name;

  /* Keep __tzname_cur_max up to date.  */
  size_t len0 = strlen (__tzname[0]);
  size_t len1 = strlen (__tzname[1]);
  if (len0 > __tzname_cur_max)
    __tzname_cur_max = len0;
  if (len1 > __tzname_cur_max)
    __tzname_cur_max = len1;
}


static unsigned int
__attribute_noinline__
compute_offset (unsigned int ss, unsigned int mm, unsigned int hh)
{
  return min (ss, 59) + min (mm, 59) * 60 + min (hh, 24) * 60 * 60;
}


/* Parse the POSIX TZ-style string.  */
void
__tzset_parse_tz (tz)
     const char *tz;
{
  unsigned short int hh, mm, ss;

  /* Clear out old state and reset to unnamed UTC.  */
  memset (tz_rules, '\0', sizeof tz_rules);
  tz_rules[0].name = tz_rules[1].name = "";

  /* Get the standard timezone name.  */
  char *tzbuf = strdupa (tz);

  int consumed;
  if (sscanf (tz, "%[A-Za-z]%n", tzbuf, &consumed) != 1)
    {
      /* Check for the quoted version.  */
      char *wp = tzbuf;
      if (__glibc_unlikely (*tz++ != '<'))
	goto out;

      while (isalnum (*tz) || *tz == '+' || *tz == '-')
	*wp++ = *tz++;
      if (__glibc_unlikely (*tz++ != '>' || wp - tzbuf < 3))
	goto out;
      *wp = '\0';
    }
  else if (__glibc_unlikely (consumed < 3))
    goto out;
  else
    tz += consumed;

  tz_rules[0].name = __tzstring (tzbuf);

  /* Figure out the standard offset from UTC.  */
  if (*tz == '\0' || (*tz != '+' && *tz != '-' && !isdigit (*tz)))
    goto out;

  if (*tz == '-' || *tz == '+')
    tz_rules[0].offset = *tz++ == '-' ? 1L : -1L;
  else
    tz_rules[0].offset = -1L;
  switch (sscanf (tz, "%hu%n:%hu%n:%hu%n",
		  &hh, &consumed, &mm, &consumed, &ss, &consumed))
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
  tz_rules[0].offset *= compute_offset (ss, mm, hh);
  tz += consumed;

  /* Get the DST timezone name (if any).  */
  if (*tz != '\0')
    {
      if (sscanf (tz, "%[A-Za-z]%n", tzbuf, &consumed) != 1)
	{
	  /* Check for the quoted version.  */
	  char *wp = tzbuf;
	  const char *rp = tz;
	  if (__glibc_unlikely (*rp++ != '<'))
	    /* Punt on name, set up the offsets.  */
	    goto done_names;

	  while (isalnum (*rp) || *rp == '+' || *rp == '-')
	    *wp++ = *rp++;
	  if (__glibc_unlikely (*rp++ != '>' || wp - tzbuf < 3))
	    /* Punt on name, set up the offsets.  */
	    goto done_names;
	  *wp = '\0';
	  tz = rp;
	}
      else if (__glibc_unlikely (consumed < 3))
	/* Punt on name, set up the offsets.  */
	goto done_names;
      else
	tz += consumed;

      tz_rules[1].name = __tzstring (tzbuf);

      /* Figure out the DST offset from GMT.  */
      if (*tz == '-' || *tz == '+')
	tz_rules[1].offset = *tz++ == '-' ? 1L : -1L;
      else
	tz_rules[1].offset = -1L;

      switch (sscanf (tz, "%hu%n:%hu%n:%hu%n",
		      &hh, &consumed, &mm, &consumed, &ss, &consumed))
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
	  tz_rules[1].offset *= compute_offset (ss, mm, hh);
	  tz += consumed;
	  break;
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
  for (unsigned int whichrule = 0; whichrule < 2; ++whichrule)
    {
      tz_rule *tzr = &tz_rules[whichrule];

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
	  unsigned long int d = strtoul (tz, &end, 10);
	  if (end == tz || d > 365)
	    goto out;
	  if (tzr->type == J1 && d == 0)
	    goto out;
	  tzr->d = d;
	  tz = end;
	}
      else if (*tz == 'M')
	{
	  tzr->type = M;
	  if (sscanf (tz, "M%hu.%hu.%hu%n",
		      &tzr->m, &tzr->n, &tzr->d, &consumed) != 3
	      || tzr->m < 1 || tzr->m > 12
	      || tzr->n < 1 || tzr->n > 5 || tzr->d > 6)
	    goto out;
	  tz += consumed;
	}
      else if (*tz == '\0')
	{
         /* Daylight time rules in the U.S. are defined in the
            U.S. Code, Title 15, Chapter 6, Subchapter IX - Standard
            Time.  These dates were established by Congress in the
            Energy Policy Act of 2005 [Pub. L. no. 109-58, 119 Stat 594
            (2005)].
	    Below is the equivalent of "M3.2.0,M11.1.0" [/2 not needed
	    since 2:00AM is the default].  */
	  tzr->type = M;
	  if (tzr == &tz_rules[0])
	    {
	      tzr->m = 3;
	      tzr->n = 2;
	      tzr->d = 0;
	    }
	  else
	    {
	      tzr->m = 11;
	      tzr->n = 1;
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
	  int negative;
	  ++tz;
	  if (*tz == '\0')
	    goto out;
	  negative = *tz == '-';
	  tz += negative;
	  consumed = 0;
	  switch (sscanf (tz, "%hu%n:%hu%n:%hu%n",
			  &hh, &consumed, &mm, &consumed, &ss, &consumed))
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
	  tz += consumed;
	  tzr->secs = (negative ? -1 : 1) * ((hh * 60 * 60) + (mm * 60) + ss);
	}
      else
	/* Default to 2:00 AM.  */
	tzr->secs = 2 * 60 * 60;

      tzr->computed_for = -1;
    }

 out:
  update_vars ();
}

/* Interpret the TZ envariable.  */
static void
internal_function
tzset_internal (always, explicit)
     int always;
     int explicit;
{
  static int is_initialized;
  const char *tz;

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

  /* Check whether the value changed since the last run.  */
  if (old_tz != NULL && tz != NULL && strcmp (tz, old_tz) == 0)
    /* No change, simply return.  */
    return;

  if (tz == NULL)
    /* No user specification; use the site-wide default.  */
    tz = TZDEFAULT;

  tz_rules[0].name = NULL;
  tz_rules[1].name = NULL;

  /* Save the value of `tz'.  */
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
      memset (tz_rules, '\0', sizeof tz_rules);
      tz_rules[0].name = tz_rules[1].name = "UTC";
      if (J0 != 0)
	tz_rules[0].type = tz_rules[1].type = J0;
      tz_rules[0].change = tz_rules[1].change = (time_t) -1;
      update_vars ();
      return;
    }

  __tzset_parse_tz (tz);
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
  time_t t;

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
void
internal_function
__tz_compute (timer, tm, use_localtime)
     time_t timer;
     struct tm *tm;
     int use_localtime;
{
  compute_change (&tz_rules[0], 1900 + tm->tm_year);
  compute_change (&tz_rules[1], 1900 + tm->tm_year);

  if (use_localtime)
    {
      int isdst;

      /* We have to distinguish between northern and southern
	 hemisphere.  For the latter the daylight saving time
	 ends in the next year.  */
      if (__builtin_expect (tz_rules[0].change
			    > tz_rules[1].change, 0))
	isdst = (timer < tz_rules[1].change
		 || timer >= tz_rules[0].change);
      else
	isdst = (timer >= tz_rules[0].change
		 && timer < tz_rules[1].change);
      tm->tm_isdst = isdst;
      tm->tm_zone = __tzname[isdst];
      tm->tm_gmtoff = tz_rules[isdst].offset;
    }
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
     This is a good idea since this allows at least a bit more parallelism.  */
  tzset_internal (tp == &_tmbuf && use_localtime, 1);

  if (__use_tzfile)
    __tzfile_compute (*timer, use_localtime, &leap_correction,
		      &leap_extra_secs, tp);
  else
    {
      if (! __offtime (timer, 0, tp))
	tp = NULL;
      else
	__tz_compute (*timer, tp, use_localtime);
      leap_correction = 0L;
      leap_extra_secs = 0;
    }

  if (tp)
    {
      if (! use_localtime)
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
