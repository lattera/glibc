/* Copyright (C) 1991-1993,1995-2001,2003,2004 Free Software Foundation, Inc.
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

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#define	NOID
#include <timezone/tzfile.h>

int __use_tzfile;
static dev_t tzfile_dev;
static ino64_t tzfile_ino;
static time_t tzfile_mtime;

struct ttinfo
  {
    long int offset;		/* Seconds east of GMT.  */
    unsigned char isdst;	/* Used to set tm_isdst.  */
    unsigned char idx;		/* Index into `zone_names'.  */
    unsigned char isstd;	/* Transition times are in standard time.  */
    unsigned char isgmt;	/* Transition times are in GMT.  */
  };

struct leap
  {
    time_t transition;		/* Time the transition takes effect.  */
    long int change;		/* Seconds of correction to apply.  */
  };

static struct ttinfo *find_transition (time_t timer) internal_function;
static void compute_tzname_max (size_t) internal_function;

static size_t num_transitions;
libc_freeres_ptr (static time_t *transitions);
static unsigned char *type_idxs;
static size_t num_types;
static struct ttinfo *types;
static char *zone_names;
static long int rule_stdoff;
static long int rule_dstoff;
static size_t num_leaps;
static struct leap *leaps;

#include <endian.h>
#include <byteswap.h>

/* Decode the four bytes at PTR as a signed integer in network byte order.  */
static inline int
__attribute ((always_inline))
decode (const void *ptr)
{
  if ((BYTE_ORDER == BIG_ENDIAN) && sizeof (int) == 4)
    return *(const int *) ptr;
  else if (BYTE_ORDER == LITTLE_ENDIAN && sizeof (int) == 4)
    return bswap_32 (*(const int *) ptr);
  else
    {
      const unsigned char *p = ptr;
      int result = *p & (1 << (CHAR_BIT - 1)) ? ~0 : 0;

      result = (result << 8) | *p++;
      result = (result << 8) | *p++;
      result = (result << 8) | *p++;
      result = (result << 8) | *p++;

      return result;
    }
}

void
__tzfile_read (const char *file, size_t extra, char **extrap)
{
  static const char default_tzdir[] = TZDIR;
  size_t num_isstd, num_isgmt;
  register FILE *f;
  struct tzhead tzhead;
  size_t chars;
  register size_t i;
  size_t total_size;
  size_t types_idx;
  size_t leaps_idx;
  int was_using_tzfile = __use_tzfile;

  __use_tzfile = 0;

  if (file == NULL)
    /* No user specification; use the site-wide default.  */
    file = TZDEFAULT;
  else if (*file == '\0')
    /* User specified the empty string; use UTC with no leap seconds.  */
    goto ret_free_transitions;
  else
    {
      /* We must not allow to read an arbitrary file in a setuid
	 program.  So we fail for any file which is not in the
	 directory hierachy starting at TZDIR
	 and which is not the system wide default TZDEFAULT.  */
      if (__libc_enable_secure
	  && ((*file == '/'
	       && memcmp (file, TZDEFAULT, sizeof TZDEFAULT)
	       && memcmp (file, default_tzdir, sizeof (default_tzdir) - 1))
	      || strstr (file, "../") != NULL))
	/* This test is certainly a bit too restrictive but it should
	   catch all critical cases.  */
	goto ret_free_transitions;
    }

  if (*file != '/')
    {
      const char *tzdir;
      unsigned int len, tzdir_len;
      char *new, *tmp;

      tzdir = getenv ("TZDIR");
      if (tzdir == NULL || *tzdir == '\0')
	{
	  tzdir = default_tzdir;
	  tzdir_len = sizeof (default_tzdir) - 1;
	}
      else
	tzdir_len = strlen (tzdir);
      len = strlen (file) + 1;
      new = (char *) __alloca (tzdir_len + 1 + len);
      tmp = __mempcpy (new, tzdir, tzdir_len);
      *tmp++ = '/';
      memcpy (tmp, file, len);
      file = new;
    }

  /* If we were already using tzfile, check whether the file changed.  */
  struct stat64 st;
  if (was_using_tzfile
      && stat64 (file, &st) == 0
      && tzfile_ino == st.st_ino && tzfile_dev == st.st_dev
      && tzfile_mtime == st.st_mtime)
    {
      /* Nothing to do.  */
      __use_tzfile = 1;
      return;
    }

  /* Note the file is opened with cancellation in the I/O functions
     disabled.  */
  f = fopen (file, "rc");
  if (f == NULL)
    goto ret_free_transitions;

  /* Get information about the file we are actually using.  */
  if (fstat64 (fileno (f), &st) != 0)
    {
      fclose (f);
      goto ret_free_transitions;
    }

  free ((void *) transitions);
  transitions = NULL;

  /* Remember the inode and device number and modification time.  */
  tzfile_dev = st.st_dev;
  tzfile_ino = st.st_ino;
  tzfile_mtime = st.st_mtime;

  /* No threads reading this stream.  */
  __fsetlocking (f, FSETLOCKING_BYCALLER);

  if (__builtin_expect (fread_unlocked ((void *) &tzhead, sizeof (tzhead),
					1, f) != 1, 0))
    goto lose;

  num_transitions = (size_t) decode (tzhead.tzh_timecnt);
  num_types = (size_t) decode (tzhead.tzh_typecnt);
  chars = (size_t) decode (tzhead.tzh_charcnt);
  num_leaps = (size_t) decode (tzhead.tzh_leapcnt);
  num_isstd = (size_t) decode (tzhead.tzh_ttisstdcnt);
  num_isgmt = (size_t) decode (tzhead.tzh_ttisgmtcnt);

  total_size = num_transitions * (sizeof (time_t) + 1);
  total_size = ((total_size + __alignof__ (struct ttinfo) - 1)
		& ~(__alignof__ (struct ttinfo) - 1));
  types_idx = total_size;
  total_size += num_types * sizeof (struct ttinfo) + chars;
  total_size = ((total_size + __alignof__ (struct leap) - 1)
		& ~(__alignof__ (struct leap) - 1));
  leaps_idx = total_size;
  total_size += num_leaps * sizeof (struct leap);
  /* This is for the extra memory required by the caller.  */
  total_size += extra;

  transitions = (time_t *) malloc (total_size);
  if (transitions == NULL)
    goto lose;

  type_idxs = (unsigned char *) transitions + (num_transitions
					       * sizeof (time_t));
  types = (struct ttinfo *) ((char *) transitions + types_idx);
  zone_names = (char *) types + num_types * sizeof (struct ttinfo);
  leaps = (struct leap *) ((char *) transitions + leaps_idx);
  if (extra > 0)
    *extrap = (char *) &leaps[num_leaps];

  if (sizeof (time_t) < 4)
    abort ();

  if (sizeof (time_t) == 4)
    {
      if (__builtin_expect (fread_unlocked (transitions, 1,
					    (4 + 1) * num_transitions, f)
			    != (4 + 1) * num_transitions, 0))
	goto lose;
    }
  else
    {
      if (__builtin_expect (fread_unlocked (transitions, 4, num_transitions, f)
			    != num_transitions, 0)
	  || __builtin_expect (fread_unlocked (type_idxs, 1, num_transitions,
					       f) != num_transitions, 0))
	goto lose;
    }

  /* Check for bogus indices in the data file, so we can hereafter
     safely use type_idxs[T] as indices into `types' and never crash.  */
  for (i = 0; i < num_transitions; ++i)
    if (__builtin_expect (type_idxs[i] >= num_types, 0))
      goto lose;

  if (BYTE_ORDER != BIG_ENDIAN || sizeof (time_t) != 4)
    {
      /* Decode the transition times, stored as 4-byte integers in
	 network (big-endian) byte order.  We work from the end of
	 the array so as not to clobber the next element to be
	 processed when sizeof (time_t) > 4.  */
      i = num_transitions;
      while (i-- > 0)
	transitions[i] = decode ((char *) transitions + i * 4);
    }

  for (i = 0; i < num_types; ++i)
    {
      unsigned char x[4];
      int c;
      if (__builtin_expect (fread_unlocked (x, 1, sizeof (x), f) != sizeof (x),
			    0))
	goto lose;
      c = getc_unlocked (f);
      if (__builtin_expect ((unsigned int) c > 1u, 0))
	goto lose;
      types[i].isdst = c;
      c = getc_unlocked (f);
      if (__builtin_expect ((size_t) c > chars, 0))
	/* Bogus index in data file.  */
	goto lose;
      types[i].idx = c;
      types[i].offset = (long int) decode (x);
    }

  if (__builtin_expect (fread_unlocked (zone_names, 1, chars, f) != chars, 0))
    goto lose;

  for (i = 0; i < num_leaps; ++i)
    {
      unsigned char x[4];
      if (__builtin_expect (fread_unlocked (x, 1, sizeof (x), f) != sizeof (x),
			    0))
	goto lose;
      leaps[i].transition = (time_t) decode (x);
      if (__builtin_expect (fread_unlocked (x, 1, sizeof (x), f) != sizeof (x),
			    0))
	goto lose;
      leaps[i].change = (long int) decode (x);
    }

  for (i = 0; i < num_isstd; ++i)
    {
      int c = getc_unlocked (f);
      if (__builtin_expect (c == EOF, 0))
	goto lose;
      types[i].isstd = c != 0;
    }
  while (i < num_types)
    types[i++].isstd = 0;

  for (i = 0; i < num_isgmt; ++i)
    {
      int c = getc_unlocked (f);
      if (__builtin_expect (c == EOF, 0))
	goto lose;
      types[i].isgmt = c != 0;
    }
  while (i < num_types)
    types[i++].isgmt = 0;

  fclose (f);

  /* First "register" all timezone names.  */
  for (i = 0; i < num_types; ++i)
    (void) __tzstring (&zone_names[types[i].idx]);

  /* Find the standard and daylight time offsets used by the rule file.
     We choose the offsets in the types of each flavor that are
     transitioned to earliest in time.  */
  __tzname[0] = NULL;
  __tzname[1] = NULL;
  for (i = num_transitions; i > 0; )
    {
      int type = type_idxs[--i];
      int dst = types[type].isdst;

      if (__tzname[dst] == NULL)
	{
	  int idx = types[type].idx;

	  __tzname[dst] = __tzstring (&zone_names[idx]);

	  if (__tzname[1 - dst] != NULL)
	    break;
	}
    }
  if (__tzname[0] == NULL)
    {
      /* This should only happen if there are no transition rules.
	 In this case there should be only one single type.  */
      assert (num_types == 1);
      __tzname[0] = __tzstring (zone_names);
    }
  if (__tzname[1] == NULL)
    __tzname[1] = __tzname[0];

  compute_tzname_max (chars);

  if (num_transitions == 0)
    /* Use the first rule (which should also be the only one).  */
    rule_stdoff = rule_dstoff = types[0].offset;
  else
    {
      int stdoff_set = 0, dstoff_set = 0;
      rule_stdoff = rule_dstoff = 0;
      i = num_transitions - 1;
      do
	{
	  if (!stdoff_set && !types[type_idxs[i]].isdst)
	    {
	      stdoff_set = 1;
	      rule_stdoff = types[type_idxs[i]].offset;
	    }
	  else if (!dstoff_set && types[type_idxs[i]].isdst)
	    {
	      dstoff_set = 1;
	      rule_dstoff = types[type_idxs[i]].offset;
	    }
	  if (stdoff_set && dstoff_set)
	    break;
	}
      while (i-- > 0);

      if (!dstoff_set)
	rule_dstoff = rule_stdoff;
    }

  __daylight = rule_stdoff != rule_dstoff;
  __timezone = -rule_stdoff;

  __use_tzfile = 1;
  return;

 lose:
  fclose (f);
 ret_free_transitions:
  free ((void *) transitions);
  transitions = NULL;
}

/* The user specified a hand-made timezone, but not its DST rules.
   We will use the names and offsets from the user, and the rules
   from the TZDEFRULES file.  */

void
__tzfile_default (const char *std, const char *dst,
		  long int stdoff, long int dstoff)
{
  size_t stdlen = strlen (std) + 1;
  size_t dstlen = strlen (dst) + 1;
  size_t i;
  int isdst;
  char *cp;

  __tzfile_read (TZDEFRULES, stdlen + dstlen, &cp);
  if (!__use_tzfile)
    return;

  if (num_types < 2)
    {
      __use_tzfile = 0;
      return;
    }

  /* Ignore the zone names read from the file and use the given ones
     instead.  */
  __mempcpy (__mempcpy (cp, std, stdlen), dst, dstlen);
  zone_names = cp;

  /* Now there are only two zones, regardless of what the file contained.  */
  num_types = 2;

  /* Now correct the transition times for the user-specified standard and
     daylight offsets from GMT.  */
  isdst = 0;
  for (i = 0; i < num_transitions; ++i)
    {
      struct ttinfo *trans_type = &types[type_idxs[i]];

      /* We will use only types 0 (standard) and 1 (daylight).
	 Fix up this transition to point to whichever matches
	 the flavor of its original type.  */
      type_idxs[i] = trans_type->isdst;

      if (trans_type->isgmt)
	/* The transition time is in GMT.  No correction to apply.  */ ;
      else if (isdst && !trans_type->isstd)
	/* The type says this transition is in "local wall clock time", and
	   wall clock time as of the previous transition was DST.  Correct
	   for the difference between the rule's DST offset and the user's
	   DST offset.  */
	transitions[i] += dstoff - rule_dstoff;
      else
	/* This transition is in "local wall clock time", and wall clock
	   time as of this iteration is non-DST.  Correct for the
	   difference between the rule's standard offset and the user's
	   standard offset.  */
	transitions[i] += stdoff - rule_stdoff;

      /* The DST state of "local wall clock time" for the next iteration is
	 as specified by this transition.  */
      isdst = trans_type->isdst;
    }

  /* Now that we adjusted the transitions to the requested offsets,
     reset the rule_stdoff and rule_dstoff values appropriately.  They
     are used elsewhere.  */
  rule_stdoff = stdoff;
  rule_dstoff = dstoff;

  /* Reset types 0 and 1 to describe the user's settings.  */
  types[0].idx = 0;
  types[0].offset = stdoff;
  types[0].isdst = 0;
  types[1].idx = stdlen;
  types[1].offset = dstoff;
  types[1].isdst = 1;

  /* Reset the zone names to point to the user's names.  */
  __tzname[0] = (char *) std;
  __tzname[1] = (char *) dst;

  /* Set the timezone.  */
  __timezone = -types[0].offset;

  compute_tzname_max (stdlen + dstlen);
}

static struct ttinfo *
internal_function
find_transition (time_t timer)
{
  size_t i;

  if (num_transitions == 0 || timer < transitions[0])
    {
      /* TIMER is before any transition (or there are no transitions).
	 Choose the first non-DST type
	 (or the first if they're all DST types).  */
      i = 0;
      while (i < num_types && types[i].isdst)
	++i;
      if (i == num_types)
	i = 0;
    }
  else
    {
      /* Find the first transition after TIMER, and
	 then pick the type of the transition before it.  */
      for (i = 1; i < num_transitions; ++i)
	if (timer < transitions[i])
	  break;
      i = type_idxs[i - 1];
    }

  return &types[i];
}

void
__tzfile_compute (time_t timer, int use_localtime,
		  long int *leap_correct, int *leap_hit,
		  struct tm *tp)
{
  register size_t i;

  if (use_localtime)
    {
      struct ttinfo *info = find_transition (timer);
      __daylight = rule_stdoff != rule_dstoff;
      __timezone = -rule_stdoff;
      __tzname[0] = NULL;
      __tzname[1] = NULL;
      for (i = num_transitions; i > 0; )
	{
	  int type = type_idxs[--i];
	  int dst = types[type].isdst;
	  int idx = types[type].idx;

	  if (__tzname[dst] == NULL)
	    {
	      __tzname[dst] = __tzstring (&zone_names[idx]);

	      if (__tzname[1 - dst] != NULL)
		break;
	    }
	}
      if (__tzname[0] == NULL)
	{
	  /* This should only happen if there are no transition rules.
	     In this case there should be only one single type.  */
	  assert (num_types == 1);
	  __tzname[0] = __tzstring (zone_names);
	}
      if (__tzname[1] == NULL)
	/* There is no daylight saving time.  */
	__tzname[1] = __tzname[0];
      tp->tm_isdst = info->isdst;
      tp->tm_zone = __tzstring (&zone_names[info->idx]);
      tp->tm_gmtoff = info->offset;
    }

  *leap_correct = 0L;
  *leap_hit = 0;

  /* Find the last leap second correction transition time before TIMER.  */
  i = num_leaps;
  do
    if (i-- == 0)
      return;
  while (timer < leaps[i].transition);

  /* Apply its correction.  */
  *leap_correct = leaps[i].change;

  if (timer == leaps[i].transition && /* Exactly at the transition time.  */
      ((i == 0 && leaps[i].change > 0) ||
       leaps[i].change > leaps[i - 1].change))
    {
      *leap_hit = 1;
      while (i > 0
	     && leaps[i].transition == leaps[i - 1].transition + 1
	     && leaps[i].change == leaps[i - 1].change + 1)
	{
	  ++*leap_hit;
	  --i;
	}
    }
}

static void
internal_function
compute_tzname_max (size_t chars)
{
  const char *p;

  p = zone_names;
  do
    {
      const char *start = p;
      while (*p != '\0')
	++p;
      if ((size_t) (p - start) > __tzname_cur_max)
	__tzname_cur_max = p - start;
    }
  while (++p < &zone_names[chars]);
}
