/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1996.

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

#ifndef _TOKEN_H
#define _TOKEN_H

enum token_t
{
  tok_none = 0,

  tok_eof,
  tok_eol,
  tok_bsymbol,
  tok_ident,
  tok_ellipsis,
  tok_semicolon,
  tok_comma,
  tok_open_brace,
  tok_close_brace,
  tok_charcode,
  tok_ucs2,
  tok_ucs4,
  tok_number,
  tok_minus1,
  tok_string,

  tok_escape_char,
  tok_comment_char,
  tok_charmap,
  tok_end,
  tok_g0esc,
  tok_g1esc,
  tok_g2esc,
  tok_g3esc,

  tok_charids,

  tok_code_set_name,
  tok_mb_cur_max,
  tok_mb_cur_min,
  tok_charconv,
  tok_width,
  tok_width_variable,
  tok_width_default,
  tok_repertoiremap,

  tok_lc_ctype,
  tok_copy,
  tok_upper,
  tok_lower,
  tok_alpha,
  tok_digit,
  tok_xdigit,
  tok_space,
  tok_print,
  tok_graph,
  tok_blank,
  tok_cntrl,
  tok_punct,
  tok_alnum,
  tok_charclass,
  tok_toupper,
  tok_tolower,
  tok_lc_collate,
  tok_collating_element,
  tok_collating_symbol,
  tok_order_start,
  tok_order_end,
  tok_from,
  tok_forward,
  tok_backward,
  tok_position,
  tok_undefined,
  tok_ignore,
  tok_lc_monetary,
  tok_int_curr_symbol,
  tok_currency_symbol,
  tok_mon_decimal_point,
  tok_mon_thousands_sep,
  tok_mon_grouping,
  tok_positive_sign,
  tok_negative_sign,
  tok_int_frac_digits,
  tok_frac_digits,
  tok_p_cs_precedes,
  tok_p_sep_by_space,
  tok_n_cs_precedes,
  tok_n_sep_by_space,
  tok_p_sign_posn,
  tok_n_sign_posn,
  tok_lc_numeric,
  tok_decimal_point,
  tok_thousands_sep,
  tok_grouping,
  tok_lc_time,
  tok_abday,
  tok_day,
  tok_abmon,
  tok_mon,
  tok_d_t_fmt,
  tok_d_fmt,
  tok_t_fmt,
  tok_am_pm,
  tok_t_fmt_ampm,
  tok_era,
  tok_era_year,
  tok_era_d_fmt,
  tok_era_d_t_fmt,
  tok_era_t_fmt,
  tok_alt_digits,
  tok_lc_messages,
  tok_yesexpr,
  tok_noexpr,
  tok_yesstr,
  tok_nostr,

  tok_error
};


struct keyword_t
{
  const char *name;
  enum token_t token;
  int symname_or_ident;

  /* Only for locdef file.  */
  int locale;
  enum token_t base;
  enum token_t group;
  enum token_t list;
};


#endif /* token.h */
