/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <langinfo.h>
#include <string.h>

/* Undefine following line in production version.  */
/* #define NDEBUG 1 */
#include <assert.h>
#include <stdlib.h>

#include "locales.h"
#include "localeinfo.h"
#include "stringtrans.h"

#define SWAPU32(w) \
  (((w) << 24) | (((w) & 0xff00) << 8) | (((w) >> 8) & 0xff00) | ((w) >> 24))


extern void *xmalloc (size_t __n);
extern void *xrealloc (void *__p, size_t __n);


/* Entry describing an entry of the era specification.  */
struct era_data
{
  int32_t direction;
  int32_t offset;
  int32_t start_date[3];
  int32_t stop_date[3];
  const char *name;
  const char *format;
};


/* The real definition of the struct for the LC_TIME locale.  */
struct locale_time_t
{
  const char *abday[7];
  size_t cur_num_abday;
  const char *day[7];
  size_t cur_num_day;
  const char *abmon[12];
  size_t cur_num_abmon;
  const char *mon[12];
  size_t cur_num_mon;
  const char *am_pm[2];
  size_t cur_num_am_pm;
  const char *d_t_fmt;
  const char *d_fmt;
  const char *t_fmt;
  const char *t_fmt_ampm;
  const char **era;
  u_int32_t cur_num_era;
  const char *era_year;
  const char *era_d_t_fmt;
  const char *era_t_fmt;
  const char *era_d_fmt;
  const char *alt_digits[100];
  u_int32_t cur_num_alt_digits;

  struct era_data *era_entries;
  struct era_data *era_entries_ob;
};


void
time_startup (struct linereader *lr, struct localedef_t *locale,
	      struct charset_t *charset)
{
  struct locale_time_t *time;

  /* We have a definition for LC_TIME.  */
  copy_posix.mask &= ~(1 << LC_TIME);

  /* It is important that we always use UCS1 encoding for strings now.  */
  encoding_method = ENC_UCS1;

  locale->categories[LC_TIME].time = time =
    (struct locale_time_t *) xmalloc (sizeof (struct locale_time_t));

  memset (time, '\0', sizeof (struct locale_time_t));
}


void
time_finish (struct localedef_t *locale)
{
  struct locale_time_t *time = locale->categories[LC_TIME].time;

#define TESTARR_ELEM(cat, max)						      \
  if (time->cur_num_##cat == 0 && !be_quiet)				      \
    error (0, 0, _("field `%s' in category `%s' undefined"),		      \
	   #cat, "LC_TIME");						      \
  else if (time->cur_num_##cat != max && !be_quiet)			      \
    error (0, 0, _("field `%s' in category `%s' has not enough values"),      \
	   #cat, "LC_TIME")

  TESTARR_ELEM (abday, 7);
  TESTARR_ELEM (day, 7);
  TESTARR_ELEM (abmon, 12);
  TESTARR_ELEM (mon, 12);
  TESTARR_ELEM (am_pm, 2);

#define TEST_ELEM(cat)							      \
  if (time->cat == NULL && !be_quiet)					      \
    error (0, 0, _("field `%s' in category `%s' undefined"),		      \
	   #cat, "LC_TIME")

  TEST_ELEM (d_t_fmt);
  TEST_ELEM (d_fmt);
  TEST_ELEM (t_fmt);

  /* According to C.Y.Alexis Cheng <alexis@vnet.ibm.com> the T_FMT_AMPM
     field is optional.  */
  if (time->t_fmt_ampm == NULL)
    /* Use the 24h format as default.  */
    time->t_fmt_ampm = time->t_fmt;

  /* Now process the era entries.  */
  if (time->cur_num_era != 0)
    {
      const int days_per_month[12] = { 31, 29, 31, 30, 31, 30,
				       31, 31, 30, 31 ,30, 31 };
      size_t idx;

      time->era_entries =
	(struct era_data *) xmalloc (time->cur_num_era
				     * sizeof (struct era_data));

      for (idx = 0; idx < time->cur_num_era; ++idx)
	{
	  size_t era_len = strlen (time->era[idx]);
	  char *str = xmalloc ((era_len + 1 + 3) & ~3);
	  char *endp;

	  memcpy (str, time->era[idx], era_len + 1);

	  /* First character must be + or - for the direction.  */
	  if (*str != '+' && *str != '-')
	    {
	      if (!be_quiet)
		error (0, 0, _("direction flag in string %d in `era' field"
			       " in category `%s' is not '+' nor '-'"),
		       idx + 1, "LC_TIME");
	      /* Default arbitrarily to '+'.  */
	      time->era_entries[idx].direction = '+';
	    }
	  else
	    time->era_entries[idx].direction = *str;
	  if (*++str != ':')
	    {
	      if (!be_quiet)
		error (0, 0, _("direction flag in string %d in `era' field"
			       " in category `%s' is not a single character"),
		       idx + 1, "LC_TIME");
	      (void) strsep (&str, ":");
	    }
	  else
	    ++str;

	  /* Now the offset year.  */
	  time->era_entries[idx].offset = strtol (str, &endp, 10);
	  if (endp == str)
	    {
	      if (!be_quiet)
		error (0, 0, _("illegal number for offset in string %d in"
			       " `era' field in category `%s'"),
		       idx + 1, "LC_TIME");
	      (void) strsep (&str, ":");
	    }
	  else if (*endp != ':')
	    {
	      if (!be_quiet)
		error (0, 0, _("garbage at end of offset value in string %d in"
			       " `era' field in category `%s'"),
		       idx + 1, "LC_TIME");
	      (void) strsep (&str, ":");
	    }
	  else
	    str = endp + 1;

	  /* Next is the starting date in ISO format.  */
	  if (strncmp (str, "-*", 2) == 0)
	    {
	      time->era_entries[idx].start_date[0] =
		time->era_entries[idx].start_date[1] =
		time->era_entries[idx].start_date[2] = 0x80000000;
	      if (str[2] != ':')
		goto garbage_start_date;
	      str += 3;
	    }
	  else if (strncmp (str, "+*", 2) == 0)
	    {
	      time->era_entries[idx].start_date[0] =
		time->era_entries[idx].start_date[1] =
		time->era_entries[idx].start_date[2] = 0x7fffffff;
	      if (str[2] != ':')
		goto garbage_start_date;
	      str += 3;
	    }
	  else
	    {
	      time->era_entries[idx].start_date[0] = strtol (str, &endp, 10);
	      if (endp == str || *endp != '/')
		goto invalid_start_date;
	      else
		str = endp + 1;
	      time->era_entries[idx].start_date[0] -= 1900;

	      time->era_entries[idx].start_date[1] = strtol (str, &endp, 10);
	      if (endp == str || *endp != '/')
		goto invalid_start_date;
	      else
		str = endp + 1;
	      time->era_entries[idx].start_date[1] -= 1;

	      time->era_entries[idx].start_date[2] = strtol (str, &endp, 10);
	      if (endp == str)
		{
		invalid_start_date:
		  if (!be_quiet)
		    error (0, 0, _("illegal starting date in string %d in"
				   " `era' field in category `%s'"),
			   idx + 1, "LC_TIME");
		  (void) strsep (&str, ":");
		}
	      else if (*endp != ':')
		{
		garbage_start_date:
		  if (!be_quiet)
		    error (0, 0, _("garbage at end of starting date "
				   "in string %d in `era' field "
				   "in category `%s'"),
			   idx + 1, "LC_TIME");
		  (void) strsep (&str, ":");
		}
	      else
		{
		  str = endp + 1;

		  /* Check for valid value.  */
		  if ((time->era_entries[idx].start_date[1] < 0
		       || time->era_entries[idx].start_date[1] >= 12
		       || time->era_entries[idx].start_date[2] < 0
		       || (time->era_entries[idx].start_date[2]
			   > days_per_month[time->era_entries[idx].start_date[1]])
		       || (time->era_entries[idx].start_date[1] == 2
			   && time->era_entries[idx].start_date[2] == 29
			   && !__isleap (time->era_entries[idx].start_date[0])))
		      && !be_quiet)
			  error (0, 0, _("starting date is illegal in"
					 " string %d in `era' field in"
					 " category `%s'"),
				 idx + 1, "LC_TIME");
		}
	    }

	  /* Next is the stopping date in ISO format.  */
	  if (strncmp (str, "-*", 2) == 0)
	    {
	      time->era_entries[idx].stop_date[0] =
		time->era_entries[idx].stop_date[1] =
		time->era_entries[idx].stop_date[2] = 0x80000000;
	      if (str[2] != ':')
		goto garbage_stop_date;
	      str += 3;
	    }
	  else if (strncmp (str, "+*", 2) == 0)
	    {
	      time->era_entries[idx].stop_date[0] =
		time->era_entries[idx].stop_date[1] =
		time->era_entries[idx].stop_date[2] = 0x7fffffff;
	      if (str[2] != ':')
		goto garbage_stop_date;
	      str += 3;
	    }
	  else
	    {
	      time->era_entries[idx].stop_date[0] = strtol (str, &endp, 10);
	      if (endp == str || *endp != '/')
		goto invalid_stop_date;
	      else
		str = endp + 1;
	      time->era_entries[idx].stop_date[0] -= 1900;

	      time->era_entries[idx].stop_date[1] = strtol (str, &endp, 10);
	      if (endp == str || *endp != '/')
		goto invalid_stop_date;
	      else
		str = endp + 1;
	      time->era_entries[idx].stop_date[1] -= 1;

	      time->era_entries[idx].stop_date[2] = strtol (str, &endp, 10);
	      if (endp == str)
		{
		invalid_stop_date:
		  if (!be_quiet)
		    error (0, 0, _("illegal stopping date in string %d in"
				   " `era' field in category `%s'"),
			   idx + 1, "LC_TIME");
		  (void) strsep (&str, ":");
		}
	      else if (*endp != ':')
		{
		garbage_stop_date:
		  if (!be_quiet)
		    error (0, 0, _("garbage at end of stopping date "
				   "in string %d in `era' field "
				   "in category `%s'"),
			   idx + 1, "LC_TIME");
		  (void) strsep (&str, ":");
		}
	      else
		{
		  str = endp + 1;

		  /* Check for valid value.  */
		  if ((time->era_entries[idx].stop_date[1] < 0
		       || time->era_entries[idx].stop_date[1] >= 12
		       || time->era_entries[idx].stop_date[2] < 0
		       || (time->era_entries[idx].stop_date[2]
			   > days_per_month[time->era_entries[idx].stop_date[1]])
		       || (time->era_entries[idx].stop_date[1] == 2
			   && time->era_entries[idx].stop_date[2] == 29
			   && !__isleap (time->era_entries[idx].stop_date[0])))
		      && !be_quiet)
			  error (0, 0, _("stopping date is illegal in"
					 " string %d in `era' field in"
					 " category `%s'"),
				 idx + 1, "LC_TIME");
		}
	    }

	  if (str == NULL || *str == '\0')
	    {
	      if (!be_quiet)
		error (0, 0, _("missing era name in string %d in `era' field"
			       " in category `%s'"), idx + 1, "LC_TIME");
	      time->era_entries[idx].name =
		time->era_entries[idx].format = "";
	    }
	  else
	    {
	      time->era_entries[idx].name = strsep (&str, ":");

	      if (str == NULL || *str == '\0')
		{
		  if (!be_quiet)
		    error (0, 0, _("missing era format in string %d in `era'"
				   " field in category `%s'"),
			   idx + 1, "LC_TIME");
		  time->era_entries[idx].name =
		    time->era_entries[idx].format = "";
		}
	      else
		time->era_entries[idx].format = str;
	    }
	}

      /* Construct the array for the other byte order.  */
      time->era_entries_ob =
	(struct era_data *) xmalloc (time->cur_num_era
				      * sizeof (struct era_data));

      for (idx = 0; idx < time->cur_num_era; ++idx)
	{
	  time->era_entries_ob[idx].direction =
	    SWAPU32 (time->era_entries[idx].direction);
	  time->era_entries_ob[idx].offset =
	    SWAPU32 (time->era_entries[idx].offset);
	  time->era_entries_ob[idx].start_date[0] =
	    SWAPU32 (time->era_entries[idx].start_date[0]);
	  time->era_entries_ob[idx].start_date[1] =
	    SWAPU32 (time->era_entries[idx].start_date[1]);
	  time->era_entries_ob[idx].start_date[2] =
	    SWAPU32 (time->era_entries[idx].stop_date[2]);
	  time->era_entries_ob[idx].stop_date[0] =
	    SWAPU32 (time->era_entries[idx].stop_date[0]);
	  time->era_entries_ob[idx].stop_date[1] =
	    SWAPU32 (time->era_entries[idx].stop_date[1]);
	  time->era_entries_ob[idx].stop_date[2] =
	    SWAPU32 (time->era_entries[idx].stop_date[2]);
	  time->era_entries_ob[idx].name =
	    time->era_entries[idx].name;
	  time->era_entries_ob[idx].format =
	    time->era_entries[idx].format;
	}
    }
}


void
time_output (struct localedef_t *locale, const char *output_path)
{
  struct locale_time_t *time = locale->categories[LC_TIME].time;
  struct iovec iov[2 + _NL_ITEM_INDEX (_NL_NUM_LC_TIME)
		  + time->cur_num_era - 1
		  + time->cur_num_alt_digits - 1
		  + 1 + (time->cur_num_era * 9 - 1) * 2
		  + (time->cur_num_era == 0)];
  struct locale_file data;
  u_int32_t idx[_NL_ITEM_INDEX (_NL_NUM_LC_TIME)];
  size_t cnt, last_idx, num;

  if ((locale->binary & (1 << LC_TIME)) != 0)
    {
      iov[0].iov_base = time;
      iov[0].iov_len = locale->len[LC_TIME];

      write_locale_data (output_path, "LC_TIME", 1, iov);

      return;
    }

  data.magic = LIMAGIC (LC_TIME);
  data.n = _NL_ITEM_INDEX (_NL_NUM_LC_TIME);
  iov[0].iov_base = (void *) &data;
  iov[0].iov_len = sizeof (data);

  iov[1].iov_base = (void *) idx;
  iov[1].iov_len = sizeof (idx);

  idx[0] = iov[0].iov_len + iov[1].iov_len;

  /* The ab'days.  */
  for (cnt = 0; cnt <= _NL_ITEM_INDEX (ABDAY_7); ++cnt)
    {
      iov[2 + cnt].iov_base =
	(void *) (time->abday[cnt - _NL_ITEM_INDEX (ABDAY_1)] ?: "");
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
    }

  /* The days.  */
  for (; cnt <= _NL_ITEM_INDEX (DAY_7); ++cnt)
    {
      iov[2 + cnt].iov_base =
	(void *) (time->day[cnt - _NL_ITEM_INDEX (DAY_1)] ?: "");
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
    }

  /* The ab'mons.  */
  for (; cnt <= _NL_ITEM_INDEX (ABMON_12); ++cnt)
    {
      iov[2 + cnt].iov_base =
	(void *) (time->abmon[cnt - _NL_ITEM_INDEX (ABMON_1)] ?: "");
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
    }

  /* The mons.  */
  for (; cnt <= _NL_ITEM_INDEX (MON_12); ++cnt)
    {
      iov[2 + cnt].iov_base =
	(void *) (time->mon[cnt - _NL_ITEM_INDEX (MON_1)] ?: "");
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
    }

  /* AM/PM.  */
  for (; cnt <= _NL_ITEM_INDEX (PM_STR); ++cnt)
    {
      iov[2 + cnt].iov_base =
	(void *) (time->am_pm[cnt - _NL_ITEM_INDEX (AM_STR)] ?: "");
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
    }

  iov[2 + cnt].iov_base = (void *) (time->d_t_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
  ++cnt;

  iov[2 + cnt].iov_base = (void *) (time->d_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
  ++cnt;

  iov[2 + cnt].iov_base = (void *) (time->t_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
  ++cnt;

  iov[2 + cnt].iov_base = (void *) (time->t_fmt_ampm ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + cnt] = idx[cnt] + iov[2 + cnt].iov_len;
  last_idx = ++cnt;

  idx[1 + last_idx] = idx[last_idx];
  for (num = 0; num < time->cur_num_era; ++num, ++cnt)
    {
      iov[2 + cnt].iov_base = (void *) time->era[num];
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + last_idx] += iov[2 + cnt].iov_len;
    }
  ++last_idx;

  iov[2 + cnt].iov_base = (void *) (time->era_year ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + last_idx] = idx[last_idx] + iov[2 + cnt].iov_len;
  ++cnt;
  ++last_idx;

  iov[2 + cnt].iov_base = (void *) (time->era_d_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + last_idx] = idx[last_idx] + iov[2 + cnt].iov_len;
  ++cnt;
  ++last_idx;

  idx[1 + last_idx] = idx[last_idx];
  for (num = 0; num < time->cur_num_alt_digits; ++num, ++cnt)
    {
      iov[2 + cnt].iov_base = (void *) (time->alt_digits[num] ?: "");
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
      idx[1 + last_idx] += iov[2 + cnt].iov_len;
    }
  ++last_idx;

  iov[2 + cnt].iov_base = (void *) (time->era_d_t_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + last_idx] = idx[last_idx] + iov[2 + cnt].iov_len;
  ++cnt;
  ++last_idx;

  iov[2 + cnt].iov_base = (void *) (time->era_t_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  idx[1 + last_idx] = idx[last_idx] + iov[2 + cnt].iov_len;
  ++cnt;
  ++last_idx;


  /* We must align the following data.  */
  iov[2 + cnt].iov_base = (void *) "\0\0";
  iov[2 + cnt].iov_len = ((idx[last_idx] + 3) & ~3) - idx[last_idx];
  idx[last_idx] = (idx[last_idx] + 3) & ~3;
  ++cnt;

  iov[2 + cnt].iov_base = (void *) &time->cur_num_alt_digits;
  iov[2 + cnt].iov_len = sizeof (u_int32_t);
  idx[1 + last_idx] = idx[last_idx] + iov[2 + cnt].iov_len;
  ++cnt;
  ++last_idx;

  /* The `era' data in usable form.  */
  iov[2 + cnt].iov_base = (void *) &time->cur_num_era;
  iov[2 + cnt].iov_len = sizeof (u_int32_t);
  idx[1 + last_idx] = idx[last_idx] + iov[2 + cnt].iov_len;
  ++cnt;
  ++last_idx;

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define ERA_B1 time->era_entries_ob
# define ERA_B2 time->era_entries
#else
# define ERA_B1 time->era_entries
# define ERA_B2 time->era_entries_ob
#endif
  idx[1 + last_idx] = idx[last_idx];
  for (num = 0; num < time->cur_num_era; ++num)
    {
      size_t l;

      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].direction;
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].offset;
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].start_date[0];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].start_date[1];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].start_date[2];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].stop_date[0];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].stop_date[1];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B1[num].stop_date[2];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;

      l = (strchr (ERA_B1[num].format, '\0') - ERA_B1[num].name) + 1;
      l = (l + 3) & ~3;
      iov[2 + cnt].iov_base = (void *) ERA_B1[num].name;
      iov[2 + cnt].iov_len = l;
      ++cnt;

      idx[1 + last_idx] += 8 * sizeof (int32_t) + l;

      assert (idx[1 + last_idx] % 4 == 0);
    }
  ++last_idx;

  /* idx[1 + last_idx] = idx[last_idx]; */
  for (num = 0; num < time->cur_num_era; ++num)
    {
      size_t l;

      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].direction;
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].offset;
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].start_date[0];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].start_date[1];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].start_date[2];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].stop_date[0];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].stop_date[1];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;
      iov[2 + cnt].iov_base = (void *) &ERA_B2[num].stop_date[2];
      iov[2 + cnt].iov_len = sizeof (int32_t);
      ++cnt;

      l = (strchr (ERA_B2[num].format, '\0') - ERA_B2[num].name) + 1;
      l = (l + 3) & ~3;
      iov[2 + cnt].iov_base = (void *) ERA_B2[num].name;
      iov[2 + cnt].iov_len = l;
      ++cnt;

      /* idx[1 + last_idx] += 8 * sizeof (int32_t) + l; */
    }

  /* We have a problem when no era data is present.  In this case the
     data pointer for _NL_TIME_ERA_ENTRIES_EB and
     _NL_TIME_ERA_ENTRIES_EL point after the end of the file.  So we
     introduce some dummy data here.  */
  if (time->cur_num_era == 0)
    {
      static u_int32_t dummy = 0;
      iov[2 + cnt].iov_base = (void *) &dummy;
      iov[2 + cnt].iov_len = 4;
      ++cnt;
    }

  assert (cnt == (_NL_ITEM_INDEX (_NL_NUM_LC_TIME)
		  + time->cur_num_era - 1
		  + time->cur_num_alt_digits - 1
		  + 1 + (time->cur_num_era * 9 - 1) * 2
		  + (time->cur_num_era == 0))
	  && last_idx + 1 == _NL_ITEM_INDEX (_NL_NUM_LC_TIME));

  write_locale_data (output_path, "LC_TIME", 2 + cnt, iov);
}


void
time_add (struct linereader *lr, struct localedef_t *locale,
	  enum token_t tok, struct token *code,
	  struct charset_t *charset)
{
  struct locale_time_t *time = locale->categories[LC_TIME].time;

  switch (tok)
    {
#define STRARR_ELEM(cat, max)						      \
    case tok_##cat:							      \
      if (time->cur_num_##cat >= max)					      \
	lr_error (lr, _("\
too many values for field `%s' in category `%s'"),			      \
		  #cat, "LC_TIME");					      \
      else if (code->val.str.start == NULL)				      \
	{								      \
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),\
		    #cat, "LC_TIME");					      \
	  time->cat[time->cur_num_##cat++] = "";			      \
	}								      \
      else								      \
	time->cat[time->cur_num_##cat++] = code->val.str.start;		      \
      break

    STRARR_ELEM (abday, 7);
    STRARR_ELEM (day, 7);
    STRARR_ELEM (abmon, 12);
    STRARR_ELEM (mon, 12);
    STRARR_ELEM (am_pm, 2);
    STRARR_ELEM (alt_digits, 100);

    case tok_era:
      if (code->val.str.start == NULL)
	lr_error (lr, _("unknown character in field `%s' of category `%s'"),
		  "era", "LC_TIME");
      else
	{
	  ++time->cur_num_era;
	  time->era = xrealloc (time->era,
				time->cur_num_era * sizeof (char *));
	  time->era[time->cur_num_era - 1] = code->val.str.start;
	}
      break;

#define STR_ELEM(cat)							      \
    case tok_##cat:							      \
      if (time->cat != NULL)						      \
	lr_error (lr, _("\
field `%s' in category `%s' declared more than once"),			      \
		  #cat, "LC_TIME");					      \
      else if (code->val.str.start == NULL)				      \
	{								      \
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),\
		    #cat, "LC_TIME");					      \
	  time->cat = "";						      \
	}								      \
      else								      \
	time->cat = code->val.str.start;				      \
      break

    STR_ELEM (d_t_fmt);
    STR_ELEM (d_fmt);
    STR_ELEM (t_fmt);
    STR_ELEM (t_fmt_ampm);
    STR_ELEM (era_year);
    STR_ELEM (era_d_t_fmt);
    STR_ELEM (era_d_fmt);
    STR_ELEM (era_t_fmt);

    default:
      assert (! "unknown token in category `LC_TIME': should not happen");
    }
}
