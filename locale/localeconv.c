/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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

#include <locale.h>
#include "localeinfo.h"

/* Return monetary and numeric information about the current locale.  */
struct lconv *
localeconv __P ((void))
{
  static struct lconv result;

  result.decimal_point = (char *) _NL_CURRENT (LC_NUMERIC, DECIMAL_POINT);
  result.thousands_sep = (char *) _NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP);
  result.grouping = (char *) _NL_CURRENT (LC_NUMERIC, GROUPING);

  result.int_curr_symbol = (char *) _NL_CURRENT (LC_MONETARY, INT_CURR_SYMBOL);
  result.currency_symbol = (char *) _NL_CURRENT (LC_MONETARY, CURRENCY_SYMBOL);
  result.mon_decimal_point = (char *) _NL_CURRENT (LC_MONETARY,
						   MON_DECIMAL_POINT);
  result.mon_thousands_sep = (char *) _NL_CURRENT (LC_MONETARY,
						   MON_THOUSANDS_SEP);
  result.mon_grouping = (char *) _NL_CURRENT (LC_MONETARY, MON_GROUPING);
  result.positive_sign = (char *) _NL_CURRENT (LC_MONETARY, POSITIVE_SIGN);
  result.negative_sign = (char *) _NL_CURRENT (LC_MONETARY, NEGATIVE_SIGN);
  result.int_frac_digits = *(char *) _NL_CURRENT (LC_MONETARY,
						  INT_FRAC_DIGITS);
  result.frac_digits = *(char *) _NL_CURRENT (LC_MONETARY, FRAC_DIGITS);
  result.p_cs_precedes = *(char *) _NL_CURRENT (LC_MONETARY, P_CS_PRECEDES);
  result.p_sep_by_space = *(char *) _NL_CURRENT (LC_MONETARY, P_SEP_BY_SPACE);
  result.n_cs_precedes = *(char *) _NL_CURRENT (LC_MONETARY, P_CS_PRECEDES);
  result.n_sep_by_space = *(char *) _NL_CURRENT (LC_MONETARY, N_SEP_BY_SPACE);
  result.p_sign_posn = *(char *) _NL_CURRENT (LC_MONETARY, P_SIGN_POSN);
  result.n_sign_posn = *(char *) _NL_CURRENT (LC_MONETARY, N_SIGN_POSN);

  return &result;
}
