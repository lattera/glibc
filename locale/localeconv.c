/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <localeinfo.h>
#include <locale.h>


/* Return monetary and numeric information about the current locale.  */
struct lconv *
DEFUN_VOID(localeconv)
{
  static struct lconv result;

  result.decimal_point = (char *) _numeric_info->decimal_point;
  result.thousands_sep = (char *) _numeric_info->thousands_sep;
  result.grouping = (char *) _numeric_info->grouping;

  result.int_curr_symbol = (char *) _monetary_info->int_curr_symbol;
  result.currency_symbol = (char *) _monetary_info->currency_symbol;
  result.mon_decimal_point = (char *) _monetary_info->mon_decimal_point;
  result.mon_thousands_sep = (char *) _monetary_info->mon_thousands_sep;
  result.mon_grouping = (char *) _monetary_info->mon_grouping;
  result.positive_sign = (char *) _monetary_info->positive_sign;
  result.negative_sign = (char *) _monetary_info->negative_sign;
  result.int_frac_digits = _monetary_info->int_frac_digits;
  result.frac_digits = _monetary_info->frac_digits;
  result.p_cs_precedes = _monetary_info->p_cs_precedes;
  result.p_sep_by_space = _monetary_info->p_sep_by_space;
  result.n_cs_precedes = _monetary_info->p_cs_precedes;
  result.n_sep_by_space = _monetary_info->n_sep_by_space;
  result.p_sign_posn = _monetary_info->p_sign_posn;
  result.n_sign_posn = _monetary_info->n_sign_posn;

  return &result;
}
