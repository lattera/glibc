/* Definition of all available locale categories and their items.  -*- C -*-
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.

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

/* These definitions are used by the locale-related files in the C library
   and the programs `localedef' and `locale'.

   The general format of the descriptions is like this:

     DEFINE_CATEGORY (ID, name, ( items ), setlocale-postload,
		      locale-input, locale-check, locale-output)

   where items itself is an array of entries in the form

     { ID, name, standard, value-type, min, max }

   The usage of the load, check, output functions depends on the individual
   program code which loads this file.

   The various value types for the items are `string', `stringarray', `byte'
   `bytearray', and `word'.  These cover all possible values in the current
   locale definitions.  `min' and `max' can be individually used again.  */

#ifndef NO_POSTLOAD
#define NO_POSTLOAD NULL
#endif

DEFINE_CATEGORY
(
 LC_COLLATE, "LC_COLLATE",
 (
  DEFINE_ELEMENT (_NL_COLLATE_NRULES,           "collate-nrules",           std, word)
  DEFINE_ELEMENT (_NL_COLLATE_RULES,            "collate-rules",            std, string)
  DEFINE_ELEMENT (_NL_COLLATE_HASH_SIZE,        "collate-hash-size",        std, word)
  DEFINE_ELEMENT (_NL_COLLATE_HASH_LAYERS,    "collate-hash-layers",      std, word)
  DEFINE_ELEMENT (_NL_COLLATE_TABLE_EB,       "collate-table-eb",         std, string)
  DEFINE_ELEMENT (_NL_COLLATE_TABLE_EL,       "collate-table-el",         std, string)
  DEFINE_ELEMENT (_NL_COLLATE_UNDEFINED,      "collate-undefined",        std, word)
  DEFINE_ELEMENT (_NL_COLLATE_EXTRA_EB,       "collate-extra-eb",         std, string)
  DEFINE_ELEMENT (_NL_COLLATE_EXTRA_EL,       "collate-extra-el",         std, string)
  DEFINE_ELEMENT (_NL_COLLATE_ELEM_HASH_SIZE, "collate-elem-hash-size",   std, word)
  DEFINE_ELEMENT (_NL_COLLATE_ELEM_HASH_EB,   "collate-elem-hash-eb",     std, string)
  DEFINE_ELEMENT (_NL_COLLATE_ELEM_HASH_EL,   "collate-elem-hash-el",     std, string)
  DEFINE_ELEMENT (_NL_COLLATE_ELEM_STR_POOL,  "collate-elem-str-pool",    std, string)
  DEFINE_ELEMENT (_NL_COLLATE_ELEM_VAL_EB,    "collate-elem-val-eb", std, string)
  DEFINE_ELEMENT (_NL_COLLATE_ELEM_VAL_EL,    "collate-elem-val-el", std, string)
  DEFINE_ELEMENT (_NL_COLLATE_SYMB_HASH_SIZE, "collate-symb-hash-size",   std, word)
  DEFINE_ELEMENT (_NL_COLLATE_SYMB_HASH_EB,   "collate-symb-hash-eb",     std, string)
  DEFINE_ELEMENT (_NL_COLLATE_SYMB_HASH_EL,   "collate-symb-hash-el",     std, string)
  DEFINE_ELEMENT (_NL_COLLATE_SYMB_STR_POOL,  "collate-symb-str-pool",    std, string)
  DEFINE_ELEMENT (_NL_COLLATE_SYMB_CLASS_EB,  "collate-symb-class-eb",    std, string)
  DEFINE_ELEMENT (_NL_COLLATE_SYMB_CLASS_EL,  "collate-symb-class-el",    std, string)
  ), _nl_postload_collate, collate_input, NULL, NULL)


/* The actual definition of ctype is meaningless here.  It is hard coded in
   the code because it has to be handled very specially.  Only the names of
   the functions and the value types are important.  */
DEFINE_CATEGORY
(
 LC_CTYPE, "LC_CTYPE",
 (
  DEFINE_ELEMENT (_NL_CTYPE_CLASS,	  "ctype-class",        std, string)
  DEFINE_ELEMENT (_NL_CTYPE_TOUPPER_EB,   "ctype-toupper-eb",   std, string)
  DEFINE_ELEMENT (_NL_CTYPE_TOLOWER_EB,   "ctype-tolower-eb",   std, string)
  DEFINE_ELEMENT (_NL_CTYPE_TOUPPER_EL,   "ctype-toupper-el",   std, string)
  DEFINE_ELEMENT (_NL_CTYPE_TOLOWER_EL,   "ctype-tolower-el",   std, string)
  DEFINE_ELEMENT (_NL_CTYPE_CLASS32,      "ctype-class32",      std, string)
  DEFINE_ELEMENT (_NL_CTYPE_NAMES_EB,	  "ctype-names-eb",     std, string)
  DEFINE_ELEMENT (_NL_CTYPE_NAMES_EL,	  "ctype-names-el",     std, string)
  DEFINE_ELEMENT (_NL_CTYPE_HASH_SIZE,	  "ctype-hash-size",    std, word)
  DEFINE_ELEMENT (_NL_CTYPE_HASH_LAYERS,  "ctype-hash-layers",  std, word)
  DEFINE_ELEMENT (_NL_CTYPE_CLASS_NAMES,  "ctype-class-names",  std, stringlist)
  DEFINE_ELEMENT (_NL_CTYPE_MAP_NAMES,	  "ctype-map-names",    std, stringlist)
  DEFINE_ELEMENT (_NL_CTYPE_WIDTH,	  "ctype-width",        std, bytearray)
  DEFINE_ELEMENT (_NL_CTYPE_MB_CUR_MAX,	  "ctype-mb-cur-max",   std, word)
  DEFINE_ELEMENT (_NL_CTYPE_CODESET_NAME, "charmap",		std, string)
  ), _nl_postload_ctype, ctype_input, ctype_check, ctype_output)


DEFINE_CATEGORY
(
 LC_MONETARY, "LC_MONETARY",
 (
  DEFINE_ELEMENT (INT_CURR_SYMBOL,   "int_curr_symbol",   std, string)
  DEFINE_ELEMENT (CURRENCY_SYMBOL,   "currency_symbol",   std, string)
  DEFINE_ELEMENT (MON_DECIMAL_POINT, "mon_decimal_point", std, string)
  DEFINE_ELEMENT (MON_THOUSANDS_SEP, "mon_thousands_sep", std, string)
  DEFINE_ELEMENT (MON_GROUPING,      "mon_grouping",      std, bytearray)
  DEFINE_ELEMENT (POSITIVE_SIGN,     "positive_sign",     std, string)
  DEFINE_ELEMENT (NEGATIVE_SIGN,     "negative_sign",     std, string)
  DEFINE_ELEMENT (INT_FRAC_DIGITS,   "int_frac_digits",   std, byte)
  DEFINE_ELEMENT (FRAC_DIGITS,       "frac_digits",       std, byte)
  DEFINE_ELEMENT (P_CS_PRECEDES,     "p_cs_precedes",     std, byte, 0, 1)
  DEFINE_ELEMENT (P_SEP_BY_SPACE,    "p_sep_by_space",    std, byte, 0, 2)
  DEFINE_ELEMENT (N_CS_PRECEDES,     "n_cs_precedes",     std, byte, 0, 1)
  DEFINE_ELEMENT (N_SEP_BY_SPACE,    "n_sep_by_space",    std, byte, 0, 2)
  DEFINE_ELEMENT (P_SIGN_POSN,       "p_sign_posn",       std, byte, 0, 4)
  DEFINE_ELEMENT (N_SIGN_POSN,       "n_sign_posn",       std, byte, 0, 4)
  ), NO_POSTLOAD, NULL, monetary_check, NULL)


DEFINE_CATEGORY
(
 LC_NUMERIC, "LC_NUMERIC",
 (
  DEFINE_ELEMENT (DECIMAL_POINT, "decimal_point", std, string)
  DEFINE_ELEMENT (THOUSANDS_SEP, "thousands_sep", std, string)
  DEFINE_ELEMENT (GROUPING,      "grouping",      std, bytearray)
  ), NO_POSTLOAD, NULL, numeric_check, NULL)


DEFINE_CATEGORY
(
 LC_TIME, "LC_TIME",
 (
  DEFINE_ELEMENT (ABDAY_1,     "abday",       std, stringarray,  7,  7)
  DEFINE_ELEMENT (DAY_1,       "day",         std, stringarray,  7,  7)
  DEFINE_ELEMENT (ABMON_1,     "abmon",       std, stringarray, 12, 12)
  DEFINE_ELEMENT (MON_1,       "mon",         std, stringarray, 12, 12)
  DEFINE_ELEMENT (AM_STR,      "am_pm",       std, stringarray,  2,  2)
  DEFINE_ELEMENT (D_T_FMT,     "d_t_fmt",     std, string)
  DEFINE_ELEMENT (D_FMT,       "d_fmt",       std, string)
  DEFINE_ELEMENT (T_FMT,       "t_fmt",       std, string)
  DEFINE_ELEMENT (T_FMT_AMPM,  "t_fmt_ampm",  std, string)
  DEFINE_ELEMENT (ERA,         "era",         opt, stringarray)
  DEFINE_ELEMENT (ERA_YEAR,    "era_year",    opt, string)
  DEFINE_ELEMENT (ERA_D_FMT,   "era_d_fmt",   opt, string)
  DEFINE_ELEMENT (ALT_DIGITS,  "alt_digits",  opt, stringarray,  0, 100)
  DEFINE_ELEMENT (ERA_D_T_FMT, "era_d_t_fmt", opt, string)
  DEFINE_ELEMENT (ERA_T_FMT,   "era_t_fmt",   opt, string)
  DEFINE_ELEMENT (_NL_TIME_NUM_ALT_DIGITS,  "time-num-alt-digits", opt, word)
  DEFINE_ELEMENT (_NL_TIME_ERA_NUM_ENTRIES, "time-era-num-entries", opt, word)
  DEFINE_ELEMENT (_NL_TIME_ERA_ENTRIES_EB,  "time-era-entries-eb", opt, string)
  DEFINE_ELEMENT (_NL_TIME_ERA_ENTRIES_EL,  "time-era-entries-el", opt, string)
  ), _nl_postload_time, NULL, NULL, NULL)


DEFINE_CATEGORY
(
 LC_MESSAGES, "LC_MESSAGES",
 (
  DEFINE_ELEMENT (YESEXPR, "yesexpr", std, string)
  DEFINE_ELEMENT (NOEXPR,  "noexpr",  std, string)
  DEFINE_ELEMENT (YESSTR,  "yesstr",  opt, string)
  DEFINE_ELEMENT (NOSTR,   "nostr",   opt, string)
  ), NO_POSTLOAD, NULL, messages_check, NULL)
