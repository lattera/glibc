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

#include <alloca.h>
#include <langinfo.h>
#include <string.h>

/* Undefine following line in production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include "locales.h"
#include "localeinfo.h"
#include "stringtrans.h"

void *xmalloc (size_t __n);
void *xrealloc (void *__ptr, size_t __n);


/* The real definition of the struct for the LC_NUMERIC locale.  */
struct locale_numeric_t
{
  const char *decimal_point;
  const char *thousands_sep;
  char *grouping;
  size_t grouping_max;
  size_t grouping_act;
};


void
numeric_startup (struct linereader *lr, struct localedef_t *locale,
		 struct charset_t *charset)
{
  struct locale_numeric_t *numeric;

  /* We have a definition for LC_NUMERIC.  */
  copy_posix.mask &= ~(1 << LC_NUMERIC);

  /* It is important that we always use UCS1 encoding for strings now.  */
  encoding_method = ENC_UCS1;

  locale->categories[LC_NUMERIC].numeric = numeric =
    (struct locale_numeric_t *) xmalloc (sizeof (struct locale_numeric_t));

  memset (numeric, '\0', sizeof (struct locale_numeric_t));

  numeric->grouping_max = 80;
  numeric->grouping = (char *) xmalloc (numeric->grouping_max);
  numeric->grouping_act = 0;
}


void
numeric_finish (struct localedef_t *locale)
{
  struct locale_numeric_t *numeric = locale->categories[LC_NUMERIC].numeric;

#define TEST_ELEM(cat)							      \
  if (numeric->cat == NULL && !be_quiet)				      \
    error (0, 0, _("field `%s' in category `%s' not defined"),		      \
	   #cat, "LC_NUMERIC")

  TEST_ELEM (decimal_point);
  TEST_ELEM (thousands_sep);

  /* The decimal point must not be empty.  This is not said explicitly
     in POSIX but ANSI C (ISO/IEC 9899) says in 4.4.2.1 it has to be
     != "".  */
  if (numeric->decimal_point[0] == '\0' && !be_quiet)
    {
      error (0, 0, _("\
value for field `%s' in category `%s' must not be the empty string"),
	     "decimal_point", "LC_NUMERIC");
    }

  if (numeric->grouping_act == 0 && !be_quiet)
    error (0, 0, _("field `%s' in category `%s' not defined"),
	   "grouping", "LC_NUMERIC");
}


void
numeric_output (struct localedef_t *locale, const char *output_path)
{
  struct locale_numeric_t *numeric = locale->categories[LC_NUMERIC].numeric;
  struct iovec iov[2 + _NL_ITEM_INDEX (_NL_NUM_LC_NUMERIC)];
  struct locale_file data;
  u_int32_t idx[_NL_ITEM_INDEX (_NL_NUM_LC_NUMERIC)];
  size_t cnt = 0;

  if ((locale->binary & (1 << LC_NUMERIC)) != 0)
    {
      iov[0].iov_base = numeric;
      iov[0].iov_len = locale->len[LC_NUMERIC];

      write_locale_data (output_path, "LC_NUMERIC", 1, iov);

      return;
    }

  data.magic = LIMAGIC (LC_NUMERIC);
  data.n = _NL_ITEM_INDEX (_NL_NUM_LC_NUMERIC);
  iov[cnt].iov_base = (void *) &data;
  iov[cnt].iov_len = sizeof (data);
  ++cnt;

  iov[cnt].iov_base = (void *) idx;
  iov[cnt].iov_len = sizeof (idx);
  ++cnt;

  idx[cnt - 2] = iov[0].iov_len + iov[1].iov_len;
  iov[cnt].iov_base = (void *) (numeric->decimal_point ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (numeric->thousands_sep ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = alloca (numeric->grouping_act + 1);
  iov[cnt].iov_len = numeric->grouping_act + 1;
  memcpy (iov[cnt].iov_base, numeric->grouping, numeric->grouping_act);
  ((char *) iov[cnt].iov_base)[numeric->grouping_act] = '\0';

  assert (cnt + 1 == 2 + _NL_ITEM_INDEX (_NL_NUM_LC_NUMERIC));

  write_locale_data (output_path, "LC_NUMERIC",
		     2 + _NL_ITEM_INDEX (_NL_NUM_LC_NUMERIC), iov);
}


void
numeric_add (struct linereader *lr, struct localedef_t *locale,
	     enum token_t tok, struct token *code,
	     struct charset_t *charset)
{
  struct locale_numeric_t *numeric = locale->categories[LC_NUMERIC].numeric;

  switch (tok)
    {
#define STR_ELEM(cat)							      \
    case tok_##cat:							      \
      if (numeric->cat != NULL)						      \
	lr_error (lr, _("\
field `%s' in category `%s' declared more than once"),			      \
		  #cat, "LC_NUMERIC");					      \
      else if (code->val.str.start == NULL)				      \
	{								      \
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),\
		    #cat, "LC_NUMERIC");				      \
	  numeric->cat = "";						      \
	}								      \
      else								      \
	numeric->cat = code->val.str.start;				      \
      break

    STR_ELEM (decimal_point);
    STR_ELEM (thousands_sep);

    case tok_grouping:
      if (numeric->grouping_act == numeric->grouping_max)
	{
	  numeric->grouping_max *= 2;
	  numeric->grouping = (char *) xrealloc (numeric->grouping,
						 numeric->grouping_max);
	}
      if (numeric->grouping_act > 0
	  && (numeric->grouping[numeric->grouping_act - 1] == '\177'))
	{
	  lr_error (lr, _("\
`-1' must be last entry in `%s' field in `%s' category"),
		    "grouping", "LC_NUMERIC");
	  --numeric->grouping_act;
	}

      if (code->tok == tok_minus1)
	numeric->grouping[numeric->grouping_act++] = '\177';
      else if (code->val.num == 0)
	/* A value of 0 disables grouping from here on but we must
	   not store a NUL character since this terminates the string.
	   Use something different which must not be used otherwise.  */
	numeric->grouping[numeric->grouping_act++] = '\377';
      else if (code->val.num > 126)
	lr_error (lr, _("\
values for field `%s' in category `%s' must be smaller than 127"),
		  "grouping", "LC_NUMERIC");
      else
	numeric->grouping[numeric->grouping_act++] = code->val.num;
      break;

    default:
      assert (! "unknown token in category `LC_NUMERIC': should not happen");
    }
}
