/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <langinfo.h>
#include <string.h>

/* Undefine following line in production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include "locales.h"
#include "localeinfo.h"
#include "stringtrans.h"


void *xmalloc (size_t __n);
void *xrealloc (void *__p, size_t __n);


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
  size_t era_num;
  const char *era_year;
  const char *era_d_t_fmt;
  const char *era_t_fmt;
  const char *era_d_fmt;
  const char *alt_digits[100];
  size_t cur_num_alt_digits;
};


void
time_startup (struct linereader *lr, struct localedef_t *locale,
	      struct charset_t *charset)
{
  struct locale_time_t *time;

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
  if (time->cur_num_##cat == 0)						      \
    error (0, 0, _("field `%s' in category `%s' not defined"),		      \
	   #cat, "LC_TIME");						      \
  else if (time->cur_num_##cat != max)					      \
    error (0, 0, _("field `%s' in category `%s' has not enough values"),      \
	   #cat, "LC_TIME")

  TESTARR_ELEM (abday, 7);
  TESTARR_ELEM (day, 7);
  TESTARR_ELEM (abmon, 12);
  TESTARR_ELEM (mon, 12);
  TESTARR_ELEM (am_pm, 2);

#define TEST_ELEM(cat)							      \
  if (time->cat == NULL)						      \
    error (0, 0, _("field `%s' in category `%s' not defined"),		      \
	   #cat, "LC_TIME")

  TEST_ELEM (d_t_fmt);
  TEST_ELEM (d_fmt);
  TEST_ELEM (t_fmt);
  TEST_ELEM (t_fmt_ampm);
}


void
time_output (struct localedef_t *locale, const char *output_path)
{
  struct locale_time_t *time = locale->categories[LC_TIME].time;
  struct iovec iov[2 + _NL_ITEM_INDEX (_NL_NUM_LC_TIME)
		  + (time->era_num > 0 ? time->era_num - 1 : 0)
		  + time->cur_num_alt_digits];
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
  for (num = 0; num < time->era_num; ++num, ++cnt)
    {
      iov[2 + cnt].iov_base = (void *) time->era[num];
      iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
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

  iov[2 + cnt].iov_base = (void *) (time->era_d_fmt ?: "");
  iov[2 + cnt].iov_len = strlen (iov[2 + cnt].iov_base) + 1;
  ++cnt;

  assert (cnt == (_NL_ITEM_INDEX (_NL_NUM_LC_TIME) - 1
		  + time->cur_num_alt_digits));

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
too many values for field `%s' in category `LC_TIME'"),			      \
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
	  ++time->era_num;
	  time->era = xrealloc (time->era, time->era_num * sizeof (char *));
	  time->era[time->era_num - 1] = code->val.str.start;
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
