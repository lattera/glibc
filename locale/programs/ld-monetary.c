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
#include <limits.h>
#include <stdlib.h>
#include <string.h>


/* Undefine following line in production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include "locales.h"
#include "localeinfo.h"
#include "stringtrans.h"

extern void *xmalloc (size_t __n);
extern void *xrealloc (void *__ptr, size_t __n);


/* The real definition of the struct for the LC_NUMERIC locale.  */
struct locale_monetary_t
{
  const char *int_curr_symbol;
  const char *currency_symbol;
  const char *mon_decimal_point;
  const char *mon_thousands_sep;
  char *mon_grouping;
  size_t mon_grouping_max;
  size_t mon_grouping_act;
  const char *positive_sign;
  const char *negative_sign;
  signed char int_frac_digits;
  signed char frac_digits;
  signed char p_cs_precedes;
  signed char p_sep_by_space;
  signed char n_cs_precedes;
  signed char n_sep_by_space;
  signed char p_sign_posn;
  signed char n_sign_posn;
};


/* The content iof the field int_curr_symbol has to be taken from
   ISO-4217.  We test for correct values.  */
#define DEFINE_INT_CURR(str) str,
static const char *const valid_int_curr[] =
  {
#   include "../iso-4217.def"
  };
#define NR_VALID_INT_CURR ((sizeof (valid_int_curr) \
			    / sizeof (valid_int_curr[0])))
#undef DEFINE_INT_CURR


/* Prototypes for local functions.  */
static int curr_strcmp(const char *s1, const char **s2);


void
monetary_startup (struct linereader *lr, struct localedef_t *locale,
		  struct charset_t *charset)
{
  struct locale_monetary_t *monetary;

  /* We have a definition for LC_MONETARY.  */
  copy_posix.mask &= ~(1 << LC_MONETARY);

  /* It is important that we always use UCS1 encoding for strings now.  */
  encoding_method = ENC_UCS1;

  locale->categories[LC_MONETARY].monetary = monetary =
    (struct locale_monetary_t *) xmalloc (sizeof (struct locale_monetary_t));

  memset (monetary, '\0', sizeof (struct locale_monetary_t));

  monetary->mon_grouping_max = 80;
  monetary->mon_grouping =
    (char *) xmalloc (monetary->mon_grouping_max);
  monetary->mon_grouping_act = 0;

  monetary->int_frac_digits = -2;
  monetary->frac_digits = -2;
  monetary->p_cs_precedes = -2;
  monetary->p_sep_by_space = -2;
  monetary->n_cs_precedes = -2;
  monetary->n_sep_by_space = -2;
  monetary->p_sign_posn = -2;
  monetary->n_sign_posn = -2;
}


void
monetary_finish (struct localedef_t *locale)
{
  struct locale_monetary_t *monetary
    = locale->categories[LC_MONETARY].monetary;

#define TEST_ELEM(cat)							      \
  if (monetary->cat == NULL && !be_quiet)				      \
    error (0, 0, _("field `%s' in category `%s' undefined"),		      \
	   #cat, "LC_MONETARY")

  TEST_ELEM (int_curr_symbol);
  TEST_ELEM (currency_symbol);
  TEST_ELEM (mon_decimal_point);
  TEST_ELEM (mon_thousands_sep);
  TEST_ELEM (positive_sign);
  TEST_ELEM (negative_sign);

  /* The international currency symbol must come from ISO 4217.  */
  if (monetary->int_curr_symbol != NULL)
    {
      if (strlen (monetary->int_curr_symbol) != 4
	  && monetary->int_curr_symbol[0] != '\0')
	{
	  if (!be_quiet)
	    error (0, 0, _("\
value of field `int_curr_symbol' in category `LC_MONETARY' has wrong length"));
	}
      else if (monetary->int_curr_symbol[0] != '\0'
	       && bsearch (monetary->int_curr_symbol, valid_int_curr,
			   NR_VALID_INT_CURR, sizeof (const char *),
			   (comparison_fn_t) curr_strcmp) == NULL
	       && !be_quiet)
	error (0, 0, _("\
value of field `int_curr_symbol' in category `LC_MONETARY' does \
not correspond to a valid name in ISO 4217"));
    }

  /* The decimal point must not be empty.  This is not said explicitly
     in POSIX but ANSI C (ISO/IEC 9899) says in 4.4.2.1 it has to be
     != "".  */
  if (monetary->mon_decimal_point[0] == '\0' && !be_quiet)
    {
      error (0, 0, _("\
value for field `%s' in category `%s' must not be the empty string"),
	     "mon_decimal_point", "LC_MONETARY");
    }

  if (monetary->mon_grouping_act == 0 && !be_quiet)
    error (0, 0, _("field `%s' in category `%s' undefined"),
	   "mon_grouping", "LC_MONETARY");

#undef TEST_ELEM
#define TEST_ELEM(cat, min, max)					      \
  if (monetary->cat == -2 && !be_quiet)					      \
    error (0, 0, _("field `%s' in category `%s' undefined"),		      \
	   #cat, "LC_MONETARY");					      \
  else if ((monetary->cat < min || monetary->cat > max) && !be_quiet)	      \
    error (0, 0, _("\
value for field `%s' in category `%s' must be in range %d...%d"),	      \
	   #cat, "LC_MONETARY", min, max)

#if 0
										/* The following two test are not really necessary because all values
    the variable could have are valid.  */
  TEST_ELEM (int_frac_digits, -128, 127);	/* No range check.  */
  TEST_ELEM (frac_digits, -128, 127);		/* No range check.  */
#endif
  TEST_ELEM (p_cs_precedes, -1, 1);
  TEST_ELEM (p_sep_by_space, -1, 2);
  TEST_ELEM (n_cs_precedes, -1, 1);
  TEST_ELEM (n_sep_by_space, -1, 2);
  TEST_ELEM (p_sign_posn, -1, 4);
  TEST_ELEM (n_sign_posn, -1, 4);
}


void
monetary_output (struct localedef_t *locale, const char *output_path)
{
  struct locale_monetary_t *monetary
    = locale->categories[LC_MONETARY].monetary;
  struct iovec iov[2 + _NL_ITEM_INDEX (_NL_NUM_LC_MONETARY)];
  struct locale_file data;
  u_int32_t idx[_NL_ITEM_INDEX (_NL_NUM_LC_MONETARY)];
  size_t cnt = 0;

  if ((locale->binary & (1 << LC_MONETARY)) != 0)
    {
      iov[0].iov_base = monetary;
      iov[0].iov_len = locale->len[LC_MONETARY];

      write_locale_data (output_path, "LC_MONETARY", 1, iov);

      return;
    }

  data.magic = LIMAGIC (LC_MONETARY);
  data.n = _NL_ITEM_INDEX (_NL_NUM_LC_MONETARY);
  iov[cnt].iov_base = (void *) &data;
  iov[cnt].iov_len = sizeof (data);
  ++cnt;

  iov[cnt].iov_base = (void *) idx;
  iov[cnt].iov_len = sizeof (idx);
  ++cnt;

  idx[cnt - 2] = iov[0].iov_len + iov[1].iov_len;
  iov[cnt].iov_base = (void *) (monetary->int_curr_symbol ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (monetary->currency_symbol ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (monetary->mon_decimal_point ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (monetary->mon_thousands_sep ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = alloca (monetary->mon_grouping_act + 1);
  iov[cnt].iov_len = monetary->mon_grouping_act + 1;
  memcpy (iov[cnt].iov_base, monetary->mon_grouping,
	  monetary->mon_grouping_act);
  ((char *) iov[cnt].iov_base)[monetary->mon_grouping_act] = '\0';
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (monetary->positive_sign ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (monetary->negative_sign ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->int_frac_digits;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->frac_digits;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->p_cs_precedes;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->p_sep_by_space;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->n_cs_precedes;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->n_sep_by_space;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->p_sign_posn;
  iov[cnt].iov_len = 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) &monetary->n_sign_posn;
  iov[cnt].iov_len = 1;

  assert (cnt + 1 == 2 + _NL_ITEM_INDEX (_NL_NUM_LC_MONETARY));

  write_locale_data (output_path, "LC_MONETARY",
		     2 + _NL_ITEM_INDEX (_NL_NUM_LC_MONETARY), iov);
}


void
monetary_add (struct linereader *lr, struct localedef_t *locale,
	      enum token_t tok, struct token *code,
	      struct charset_t *charset)
{
  struct locale_monetary_t *monetary
    = locale->categories[LC_MONETARY].monetary;

  switch (tok)
    {
#define STR_ELEM(cat)							      \
    case tok_##cat:							      \
      if (monetary->cat != NULL)					      \
	lr_error (lr, _("\
field `%s' in category `%s' declared more than once"),			      \
		  #cat, "LC_MONETARY");					      \
      else if (code->val.str.start == NULL)				      \
	{								      \
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),\
		    #cat, "LC_MONETARY");				      \
	  monetary->cat = "";						      \
	}								      \
      else								      \
	monetary->cat = code->val.str.start;				      \
      break

    STR_ELEM (int_curr_symbol);
    STR_ELEM (currency_symbol);
    STR_ELEM (mon_decimal_point);
    STR_ELEM (mon_thousands_sep);
    STR_ELEM (positive_sign);
    STR_ELEM (negative_sign);

#define INT_ELEM(cat)							      \
    case tok_##cat:							      \
      if (monetary->cat != -2)						      \
	lr_error (lr, _("\
field `%s' in category `%s' declared more than once"),			      \
		  #cat, "LC_MONETARY");					      \
      else if (code->tok == tok_minus1)					      \
	monetary->cat = -1;						      \
      else								      \
	monetary->cat = code->val.num;					      \
      break

    INT_ELEM (int_frac_digits);
    INT_ELEM (frac_digits);
    INT_ELEM (p_cs_precedes);
    INT_ELEM (p_sep_by_space);
    INT_ELEM (n_cs_precedes);
    INT_ELEM (n_sep_by_space);
    INT_ELEM (p_sign_posn);
    INT_ELEM (n_sign_posn);

    case tok_mon_grouping:
      if (monetary->mon_grouping_act == monetary->mon_grouping_max)
	{
	  monetary->mon_grouping_max *= 2;
	  monetary->mon_grouping =
	    (char *) xrealloc (monetary->mon_grouping,
			       monetary->mon_grouping_max);
	}
      if (monetary->mon_grouping[monetary->mon_grouping_act - 1]
	  == '\177')
	lr_error (lr, _("\
`-1' must be last entry in `%s' field in `%s' category"),
		  "mon_grouping", "LC_MONETARY");
      else
	{
	  if (code->tok == tok_minus1)
	    monetary->mon_grouping[monetary->mon_grouping_act++] = '\177';
	  else if (code->val.num == 0)
	    /* A value of 0 disables grouping from here on but we must
	       not store a NUL character since this terminates the
	       string.  Use something different which must not be used
	       otherwise.  */
	    monetary->mon_grouping[monetary->mon_grouping_act++] = '\377';
	  else if (code->val.num > 126)
	    lr_error (lr, _("\
values for field `%s' in category `%s' must be smaller than 127"),
		      "mon_grouping", "LC_MONETARY");
	  else
	    monetary->mon_grouping[monetary->mon_grouping_act++]
	      = code->val.num;
	}
      break;

    default:
      assert (! "unknown token in category `LC_MONETARY': should not happen");
    }
}


static int
curr_strcmp(const char *s1, const char **s2)
{
  return strcmp (s1, *s2);
}
