/* Access to locale-dependent parameters.
   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _LANGINFO_H
#define	_LANGINFO_H 1

#include <locale.h>		/* Define the LC_* category names.  */

/* Get the type definition.  */
#include <nl_types.h>


__BEGIN_DECLS

/* Construct an `nl_item' value for `nl_langinfo' from a locale category
   (LC_*) and an item index within the category.  Some code may depend on
   the item values within a category increasing monotonically with the
   indices.  */
#define _NL_ITEM(category, index)	(((category) << 16) | (index))

/* Extract the category and item index from a constructed `nl_item' value.  */
#define _NL_ITEM_CATEGORY(item)		((int) (item) >> 16)
#define _NL_ITEM_INDEX(item)		((int) (item) & 0xffff)


/* Enumeration of locale items that can be queried with `nl_langinfo'.  */
enum
{
  /* LC_TIME category: date and time formatting.  */

  /* Abbreviated days of the week. */
  ABDAY_1 = _NL_ITEM (LC_TIME, 0), /* Sun */
#define ABDAY_1			ABDAY_1
  ABDAY_2,
#define ABDAY_2			ABDAY_2
  ABDAY_3,
#define ABDAY_3			ABDAY_3
  ABDAY_4,
#define ABDAY_4			ABDAY_4
  ABDAY_5,
#define ABDAY_5			ABDAY_5
  ABDAY_6,
#define ABDAY_6			ABDAY_6
  ABDAY_7,
#define ABDAY_7			ABDAY_7

  /* Long-named days of the week. */
  DAY_1,			/* Sunday */
#define DAY_1			DAY_1
  DAY_2,			/* Monday */
#define DAY_2			DAY_2
  DAY_3,			/* Tuesday */
#define DAY_3			DAY_3
  DAY_4,			/* Wednesday */
#define DAY_4			DAY_4
  DAY_5,			/* Thursday */
#define DAY_5			DAY_5
  DAY_6,			/* Friday */
#define DAY_6			DAY_6
  DAY_7,			/* Saturday */
#define DAY_7			DAY_7

  /* Abbreviated month names.  */
  ABMON_1,			/* Jan */
#define ABMON_1			ABMON_1
  ABMON_2,
#define ABMON_2			ABMON_2
  ABMON_3,
#define ABMON_3			ABMON_3
  ABMON_4,
#define ABMON_4			ABMON_4
  ABMON_5,
#define ABMON_5			ABMON_5
  ABMON_6,
#define ABMON_6			ABMON_6
  ABMON_7,
#define ABMON_7			ABMON_7
  ABMON_8,
#define ABMON_8			ABMON_8
  ABMON_9,
#define ABMON_9			ABMON_9
  ABMON_10,
#define ABMON_10		ABMON_10
  ABMON_11,
#define ABMON_11		ABMON_11
  ABMON_12,
#define ABMON_12		ABMON_12

  /* Long month names.  */
  MON_1,			/* January */
#define MON_1			MON_1
  MON_2,
#define MON_2			MON_2
  MON_3,
#define MON_3			MON_3
  MON_4,
#define MON_4			MON_4
  MON_5,
#define MON_5			MON_5
  MON_6,
#define MON_6			MON_6
  MON_7,
#define MON_7			MON_7
  MON_8,
#define MON_8			MON_8
  MON_9,
#define MON_9			MON_9
  MON_10,
#define MON_10			MON_10
  MON_11,
#define MON_11			MON_11
  MON_12,
#define MON_12			MON_12

  AM_STR,			/* Ante meridian string.  */
#define AM_STR			AM_STR
  PM_STR,			/* Post meridian string.  */
#define PM_STR			PM_STR

  D_T_FMT,			/* Date and time format for strftime.  */
#define D_T_FMT			D_T_FMT
  D_FMT,			/* Date format for strftime.  */
#define D_FMT			D_FMT
  T_FMT,			/* Time format for strftime.  */
#define T_FMT			T_FMT
  T_FMT_AMPM,			/* 12-hour time format for strftime.  */
#define T_FMT_AMPM		T_FMT_AMPM

  ERA,				/* Alternate era.  */
#define ERA			ERA
  ERA_YEAR,			/* Year in alternate era format.  */
#define ERA_YEAR		ERA_YEAR
  ERA_D_FMT,			/* Date in alternate era format.  */
#define ERA_D_FMT		ERA_D_FMT
  ALT_DIGITS,			/* Alternate symbols for digits.  */
#define ALT_DIGITS		ALT_DIGITS
  ERA_D_T_FMT,			/* Date and time in alternate era format.  */
#define ERA_D_T_FMT		ERA_D_T_FMT
  ERA_T_FMT,			/* Time in alternate era format.  */
#define ERA_T_FMT		ERA_T_FMT

  _NL_TIME_ERA_NUM_ENTRIES_EB,	/* Number entries in the era arrays.  */
  _NL_TIME_ERA_NUM_ENTRIES_EL,	/* Number entries in the era arrays.  */
  _NL_TIME_ERA_ENTRIES_EB,	/* Structure with era entries in usable form.*/
  _NL_TIME_ERA_ENTRIES_EL,

  _NL_WABDAY_1_EB, /* Sun */
  _NL_WABDAY_2_EB,
  _NL_WABDAY_3_EB,
  _NL_WABDAY_4_EB,
  _NL_WABDAY_5_EB,
  _NL_WABDAY_6_EB,
  _NL_WABDAY_7_EB,
  _NL_WABDAY_1_EL, /* Sun */
  _NL_WABDAY_2_EL,
  _NL_WABDAY_3_EL,
  _NL_WABDAY_4_EL,
  _NL_WABDAY_5_EL,
  _NL_WABDAY_6_EL,
  _NL_WABDAY_7_EL,

  /* Long-named days of the week. */
  _NL_WDAY_1_EB,		/* Sunday */
  _NL_WDAY_2_EB,		/* Monday */
  _NL_WDAY_3_EB,		/* Tuesday */
  _NL_WDAY_4_EB,		/* Wednesday */
  _NL_WDAY_5_EB,		/* Thursday */
  _NL_WDAY_6_EB,		/* Friday */
  _NL_WDAY_7_EB,		/* Saturday */
  _NL_WDAY_1_EL,		/* Sunday */
  _NL_WDAY_2_EL,		/* Monday */
  _NL_WDAY_3_EL,		/* Tuesday */
  _NL_WDAY_4_EL,		/* Wednesday */
  _NL_WDAY_5_EL,		/* Thursday */
  _NL_WDAY_6_EL,		/* Friday */
  _NL_WDAY_7_EL,		/* Saturday */

  /* Abbreviated month names.  */
  _NL_WABMON_1_EB,		/* Jan */
  _NL_WABMON_2_EB,
  _NL_WABMON_3_EB,
  _NL_WABMON_4_EB,
  _NL_WABMON_5_EB,
  _NL_WABMON_6_EB,
  _NL_WABMON_7_EB,
  _NL_WABMON_8_EB,
  _NL_WABMON_9_EB,
  _NL_WABMON_10_EB,
  _NL_WABMON_11_EB,
  _NL_WABMON_12_EB,
  _NL_WABMON_1_EL,		/* Jan */
  _NL_WABMON_2_EL,
  _NL_WABMON_3_EL,
  _NL_WABMON_4_EL,
  _NL_WABMON_5_EL,
  _NL_WABMON_6_EL,
  _NL_WABMON_7_EL,
  _NL_WABMON_8_EL,
  _NL_WABMON_9_EL,
  _NL_WABMON_10_EL,
  _NL_WABMON_11_EL,
  _NL_WABMON_12_EL,

  /* Long month names.  */
  _NL_WMON_1_EB,		/* January */
  _NL_WMON_2_EB,
  _NL_WMON_3_EB,
  _NL_WMON_4_EB,
  _NL_WMON_5_EB,
  _NL_WMON_6_EB,
  _NL_WMON_7_EB,
  _NL_WMON_8_EB,
  _NL_WMON_9_EB,
  _NL_WMON_10_EB,
  _NL_WMON_11_EB,
  _NL_WMON_12_EB,
  _NL_WMON_1_EL,		/* January */
  _NL_WMON_2_EL,
  _NL_WMON_3_EL,
  _NL_WMON_4_EL,
  _NL_WMON_5_EL,
  _NL_WMON_6_EL,
  _NL_WMON_7_EL,
  _NL_WMON_8_EL,
  _NL_WMON_9_EL,
  _NL_WMON_10_EL,
  _NL_WMON_11_EL,
  _NL_WMON_12_EL,

  _NL_WAM_STR_EB,		/* Ante meridian string.  */
  _NL_WPM_STR_EB,		/* Post meridian string.  */
  _NL_WAM_STR_EL,		/* Ante meridian string.  */
  _NL_WPM_STR_EL,		/* Post meridian string.  */

  _NL_WD_T_FMT_EB,		/* Date and time format for strftime.  */
  _NL_WD_FMT_EB,		/* Date format for strftime.  */
  _NL_WT_FMT_EB,		/* Time format for strftime.  */
  _NL_WT_FMT_AMPM_EB,		/* 12-hour time format for strftime.  */
  _NL_WD_T_FMT_EL,		/* Date and time format for strftime.  */
  _NL_WD_FMT_EL,		/* Date format for strftime.  */
  _NL_WT_FMT_EL,		/* Time format for strftime.  */
  _NL_WT_FMT_AMPM_EL,		/* 12-hour time format for strftime.  */

  _NL_WERA_YEAR_EB,		/* Year in alternate era format.  */
  _NL_WERA_D_FMT_EB,		/* Date in alternate era format.  */
  _NL_WALT_DIGITS_EB,		/* Alternate symbols for digits.  */
  _NL_WERA_D_T_FMT_EB,		/* Date and time in alternate era format.  */
  _NL_WERA_T_FMT_EB,		/* Time in alternate era format.  */
  _NL_WERA_YEAR_EL,		/* Year in alternate era format.  */
  _NL_WERA_D_FMT_EL,		/* Date in alternate era format.  */
  _NL_WALT_DIGITS_EL,		/* Alternate symbols for digits.  */
  _NL_WERA_D_T_FMT_EL,		/* Date and time in alternate era format.  */
  _NL_WERA_T_FMT_EL,		/* Time in alternate era format.  */

  _NL_TIME_WEEK_NDAYS,
  _NL_TIME_WEEK_1STDAY,
  _NL_TIME_WEEK_1STWEEK,
  _NL_TIME_FIRST_WEEKDAY,
  _NL_TIME_FIRST_WORKDAY,
  _NL_TIME_CAL_DIRECTION,
  _NL_TIME_TIMEZONE,

  _NL_NUM_LC_TIME,		/* Number of indices in LC_TIME category.  */

  /* LC_COLLATE category: text sorting.
     This information is accessed by the strcoll and strxfrm functions.
     These `nl_langinfo' names are used only internally.  */
  _NL_COLLATE_NRULES = _NL_ITEM (LC_COLLATE, 0),
  _NL_COLLATE_RULES,
  _NL_COLLATE_HASH_SIZE,
  _NL_COLLATE_HASH_LAYERS,
  _NL_COLLATE_TABLEMB_EB,
  _NL_COLLATE_TABLEMB_EL,
  _NL_COLLATE_TABLEWC_EB,
  _NL_COLLATE_TABLEWC_EL,
  _NL_COLLATE_UNDEFINED_MB,
  _NL_COLLATE_UNDEFINED_WC,
  _NL_COLLATE_EXTRAMB_EB,
  _NL_COLLATE_EXTRAMB_EL,
  _NL_COLLATE_EXTRAWC_EB,
  _NL_COLLATE_EXTRAWC_EL,
  _NL_COLLATE_ELEM_HASH_SIZE,
  _NL_COLLATE_ELEM_HASH_EB,
  _NL_COLLATE_ELEM_HASH_EL,
  _NL_COLLATE_ELEM_STR_POOL,
  _NL_COLLATE_ELEM_VAL_EB,
  _NL_COLLATE_ELEM_VAL_EL,
  _NL_COLLATE_ELEM_VALMB_EB,
  _NL_COLLATE_ELEM_VALMB_EL,
  _NL_COLLATE_ELEM_VALWC_EB,
  _NL_COLLATE_ELEM_VALWC_EL,
  _NL_COLLATE_SYMB_HASH_SIZE,
  _NL_COLLATE_SYMB_HASH_EB,
  _NL_COLLATE_SYMB_HASH_EL,
  _NL_COLLATE_SYMB_STR_POOL,
  _NL_COLLATE_SYMB_CLASSMB_EB,
  _NL_COLLATE_SYMB_CLASSMB_EL,
  _NL_COLLATE_SYMB_CLASSWC_EB,
  _NL_COLLATE_SYMB_CLASSWC_EL,
  _NL_NUM_LC_COLLATE,

  /* LC_CTYPE category: character classification.
     This information is accessed by the functions in <ctype.h>.
     These `nl_langinfo' names are used only internally.  */
  _NL_CTYPE_CLASS = _NL_ITEM (LC_CTYPE, 0),
  _NL_CTYPE_TOUPPER_EB,
  _NL_CTYPE_TOLOWER_EB,
  _NL_CTYPE_TOUPPER_EL,
  _NL_CTYPE_TOLOWER_EL,
  _NL_CTYPE_CLASS32,
  _NL_CTYPE_NAMES_EB,
  _NL_CTYPE_NAMES_EL,
  _NL_CTYPE_HASH_SIZE_EB,
  _NL_CTYPE_HASH_LAYERS_EB,
  _NL_CTYPE_CLASS_NAMES,
  _NL_CTYPE_MAP_NAMES,
  _NL_CTYPE_WIDTH,
  _NL_CTYPE_MB_CUR_MAX,
  _NL_CTYPE_CODESET_NAME,
#ifdef __USE_XOPEN
  CODESET = _NL_CTYPE_CODESET_NAME,
#endif
  _NL_CTYPE_HASH_SIZE_EL,
  _NL_CTYPE_HASH_LAYERS_EL,
  _NL_CTYPE_INDIGITS_MB_LEN_EB,
  _NL_CTYPE_INDIGITS_MB_LEN_EL,
  _NL_CTYPE_INDIGITS0_MB,
  _NL_CTYPE_INDIGITS1_MB,
  _NL_CTYPE_INDIGITS2_MB,
  _NL_CTYPE_INDIGITS3_MB,
  _NL_CTYPE_INDIGITS4_MB,
  _NL_CTYPE_INDIGITS5_MB,
  _NL_CTYPE_INDIGITS6_MB,
  _NL_CTYPE_INDIGITS7_MB,
  _NL_CTYPE_INDIGITS8_MB,
  _NL_CTYPE_INDIGITS9_MB,
  _NL_CTYPE_INDIGITS_WC_LEN_EB,
  _NL_CTYPE_INDIGITS_WC_LEN_EL,
  _NL_CTYPE_INDIGITS0_WC_EB,
  _NL_CTYPE_INDIGITS1_WC_EB,
  _NL_CTYPE_INDIGITS2_WC_EB,
  _NL_CTYPE_INDIGITS3_WC_EB,
  _NL_CTYPE_INDIGITS4_WC_EB,
  _NL_CTYPE_INDIGITS5_WC_EB,
  _NL_CTYPE_INDIGITS6_WC_EB,
  _NL_CTYPE_INDIGITS7_WC_EB,
  _NL_CTYPE_INDIGITS8_WC_EB,
  _NL_CTYPE_INDIGITS9_WC_EB,
  _NL_CTYPE_INDIGITS0_WC_EL,
  _NL_CTYPE_INDIGITS1_WC_EL,
  _NL_CTYPE_INDIGITS2_WC_EL,
  _NL_CTYPE_INDIGITS3_WC_EL,
  _NL_CTYPE_INDIGITS4_WC_EL,
  _NL_CTYPE_INDIGITS5_WC_EL,
  _NL_CTYPE_INDIGITS6_WC_EL,
  _NL_CTYPE_INDIGITS7_WC_EL,
  _NL_CTYPE_INDIGITS8_WC_EL,
  _NL_CTYPE_INDIGITS9_WC_EL,
  _NL_CTYPE_OUTDIGIT0_MB,
  _NL_CTYPE_OUTDIGIT1_MB,
  _NL_CTYPE_OUTDIGIT2_MB,
  _NL_CTYPE_OUTDIGIT3_MB,
  _NL_CTYPE_OUTDIGIT4_MB,
  _NL_CTYPE_OUTDIGIT5_MB,
  _NL_CTYPE_OUTDIGIT6_MB,
  _NL_CTYPE_OUTDIGIT7_MB,
  _NL_CTYPE_OUTDIGIT8_MB,
  _NL_CTYPE_OUTDIGIT9_MB,
  _NL_CTYPE_OUTDIGIT0_WC_EB,
  _NL_CTYPE_OUTDIGIT1_WC_EB,
  _NL_CTYPE_OUTDIGIT2_WC_EB,
  _NL_CTYPE_OUTDIGIT3_WC_EB,
  _NL_CTYPE_OUTDIGIT4_WC_EB,
  _NL_CTYPE_OUTDIGIT5_WC_EB,
  _NL_CTYPE_OUTDIGIT6_WC_EB,
  _NL_CTYPE_OUTDIGIT7_WC_EB,
  _NL_CTYPE_OUTDIGIT8_WC_EB,
  _NL_CTYPE_OUTDIGIT9_WC_EB,
  _NL_CTYPE_OUTDIGIT0_WC_EL,
  _NL_CTYPE_OUTDIGIT1_WC_EL,
  _NL_CTYPE_OUTDIGIT2_WC_EL,
  _NL_CTYPE_OUTDIGIT3_WC_EL,
  _NL_CTYPE_OUTDIGIT4_WC_EL,
  _NL_CTYPE_OUTDIGIT5_WC_EL,
  _NL_CTYPE_OUTDIGIT6_WC_EL,
  _NL_CTYPE_OUTDIGIT7_WC_EL,
  _NL_CTYPE_OUTDIGIT8_WC_EL,
  _NL_CTYPE_OUTDIGIT9_WC_EL,
  _NL_CTYPE_TRANSLIT_HASH_SIZE_EB,
  _NL_CTYPE_TRANSLIT_HASH_SIZE_EL,
  _NL_CTYPE_TRANSLIT_HASH_LAYERS_EB,
  _NL_CTYPE_TRANSLIT_HASH_LAYERS_EL,
  _NL_CTYPE_TRANSLIT_FROM_IDX_EB,
  _NL_CTYPE_TRANSLIT_FROM_IDX_EL,
  _NL_CTYPE_TRANSLIT_FROM_TBL_EB,
  _NL_CTYPE_TRANSLIT_FROM_TBL_EL,
  _NL_CTYPE_TRANSLIT_TO_IDX_EB,
  _NL_CTYPE_TRANSLIT_TO_IDX_EL,
  _NL_CTYPE_TRANSLIT_TO_TBL_EB,
  _NL_CTYPE_TRANSLIT_TO_TBL_EL,
  _NL_NUM_LC_CTYPE,

  /* LC_MONETARY category: formatting of monetary quantities.
     These items each correspond to a member of `struct lconv',
     defined in <locale.h>.  */
  INT_CURR_SYMBOL = _NL_ITEM (LC_MONETARY, 0),
#define INT_CURR_SYMBOL		INT_CURR_SYMBOL
  CURRENCY_SYMBOL,
#define CURRENCY_SYMBOL		CURRENCY_SYMBOL
#ifdef __USE_XOPEN
  CRNCYSTR = CURRENCY_SYMBOL,
# define CRNCYSTR		CRNCYSTR
#endif
  MON_DECIMAL_POINT,
#define MON_DECIMAL_POINT	MON_DECIMAL_POINT
  MON_THOUSANDS_SEP,
#define MON_THOUSANDS_SEP	MON_THOUSANDS_SEP
  MON_GROUPING,
#define MON_GROUPING		MON_GROUPING
  POSITIVE_SIGN,
#define POSITIVE_SIGN		POSITIVE_SIGN
  NEGATIVE_SIGN,
#define NEGATIVE_SIGN		NEGATIVE_SIGN
  INT_FRAC_DIGITS,
#define INT_FRAC_DIGITS		INT_FRAC_DIGITS
  FRAC_DIGITS,
#define FRAC_DIGITS		FRAC_DIGITS
  P_CS_PRECEDES,
#define P_CS_PRECEDES		P_CS_PRECEDES
  P_SEP_BY_SPACE,
#define P_SEP_BY_SPACE		P_SEP_BY_SPACE
  N_CS_PRECEDES,
#define N_CS_PRECEDES		N_CS_PRECEDES
  N_SEP_BY_SPACE,
#define N_SEP_BY_SPACE		N_SEP_BY_SPACE
  P_SIGN_POSN,
#define P_SIGN_POSN		P_SIGN_POSN
  N_SIGN_POSN,
#define N_SIGN_POSN		N_SIGN_POSN
  _NL_MONETARY_INT_P_CS_PRECEDES,
  _NL_MONETARY_INT_P_SEP_BY_SPACE,
  _NL_MONETARY_INT_N_CS_PRECEDES,
  _NL_MONETARY_INT_N_SEP_BY_SPACE,
  _NL_MONETARY_INT_P_SIGN_POSN,
  _NL_MONETARY_INT_N_SIGN_POSN,
  _NL_MONETARY_DUO_INT_CURR_SYMBOL,
  _NL_MONETARY_DUO_CURRENCY_SYMBOL,
  _NL_MONETARY_DUO_INT_FRAC_DIGITS,
  _NL_MONETARY_DUO_FRAC_DIGITS,
  _NL_MONETARY_DUO_P_CS_PRECEDES,
  _NL_MONETARY_DUO_P_SEP_BY_SPACE,
  _NL_MONETARY_DUO_N_CS_PRECEDES,
  _NL_MONETARY_DUO_N_SEP_BY_SPACE,
  _NL_MONETARY_DUO_INT_P_CS_PRECEDES,
  _NL_MONETARY_DUO_INT_P_SEP_BY_SPACE,
  _NL_MONETARY_DUO_INT_N_CS_PRECEDES,
  _NL_MONETARY_DUO_INT_N_SEP_BY_SPACE,
  _NL_MONETARY_DUO_P_SIGN_POSN,
  _NL_MONETARY_DUO_N_SIGN_POSN,
  _NL_MONETARY_DUO_INT_P_SIGN_POSN,
  _NL_MONETARY_DUO_INT_N_SIGN_POSN,
  _NL_MONETARY_UNO_VALID_FROM,
  _NL_MONETARY_UNO_VALID_TO,
  _NL_MONETARY_DUO_VALID_FROM,
  _NL_MONETARY_DUO_VALID_TO,
  _NL_MONETARY_CONVERSION_RATE_EB,
  _NL_MONETARY_CONVERSION_RATE_EL,
  _NL_NUM_LC_MONETARY,

  /* LC_NUMERIC category: formatting of numbers.
     These also correspond to members of `struct lconv'; see <locale.h>.  */
  DECIMAL_POINT = _NL_ITEM (LC_NUMERIC, 0),
#define DECIMAL_POINT		DECIMAL_POINT
#ifdef __USE_XOPEN
  RADIXCHAR = DECIMAL_POINT,
# define RADIXCHAR		RADIXCHAR
#endif
  THOUSANDS_SEP,
#define THOUSANDS_SEP		THOUSANDS_SEP
#ifdef __USE_XOPEN
  THOUSEP = THOUSANDS_SEP,
# define THOUSANDS_SEP		THOUSANDS_SEP
#endif
  GROUPING,
#define GROUPING		GROUPING
  _NL_NUM_LC_NUMERIC,

  YESEXPR = _NL_ITEM (LC_MESSAGES, 0), /* Regex matching ``yes'' input.  */
#define YESEXPR			YESEXPR
  NOEXPR,			/* Regex matching ``no'' input.  */
#define NOEXPR			NOEXPR
  YESSTR,			/* Output string for ``yes''.  */
#define YESSTR			YESSTR
  NOSTR,			/* Output string for ``no''.  */
#define	NOSTR			NOSTR
  _NL_NUM_LC_MESSAGES,

  _NL_PAPER_HEIGHT_EB = _NL_ITEM (LC_PAPER, 0),
  _NL_PAPER_HEIGHT_EL,
  _NL_PAPER_WIDTH_EB,
  _NL_PAPER_WIDTH_EL,
  _NL_NUM_LC_PAPER,

  _NL_NAME_NAME_FMT = _NL_ITEM (LC_NAME, 0),
  _NL_NAME_NAME_GEN,
  _NL_NAME_NAME_MR,
  _NL_NAME_NAME_MRS,
  _NL_NAME_NAME_MISS,
  _NL_NAME_NAME_MS,
  _NL_NUM_LC_NAME,

  _NL_ADDRESS_POSTAL_FMT = _NL_ITEM (LC_ADDRESS, 0),
  _NL_ADDRESS_COUNTRY_NAME,
  _NL_ADDRESS_COUNTRY_POST,
  _NL_ADDRESS_COUNTRY_AB2,
  _NL_ADDRESS_COUNTRY_AB3,
  _NL_ADDRESS_COUNTRY_CAR,
  _NL_ADDRESS_COUNTRY_NUM_EB,
  _NL_ADDRESS_COUNTRY_NUM_EL,
  _NL_ADDRESS_COUNTRY_ISBN,
  _NL_ADDRESS_LANG_NAME,
  _NL_ADDRESS_LANG_AB,
  _NL_ADDRESS_LANG_TERM,
  _NL_ADDRESS_LANG_LIB,
  _NL_NUM_LC_ADDRESS,

  _NL_TELEPHONE_TEL_INT_FMT = _NL_ITEM (LC_TELEPHONE, 0),
  _NL_TELEPHONE_TEL_DOM_FMT,
  _NL_TELEPHONE_INT_SELECT,
  _NL_TELEPHONE_INT_PREFIX,
  _NL_NUM_LC_TELEPHONE,

  _NL_MEASUREMENT_MEASUREMENT = _NL_ITEM (LC_MEASUREMENT, 0),
  _NL_NUM_LC_MEASUREMENT,

  _NL_IDENTIFICATION_TITLE = _NL_ITEM (LC_IDENTIFICATION, 0),
  _NL_IDENTIFICATION_SOURCE,
  _NL_IDENTIFICATION_ADDRESS,
  _NL_IDENTIFICATION_CONTACT,
  _NL_IDENTIFICATION_EMAIL,
  _NL_IDENTIFICATION_TEL,
  _NL_IDENTIFICATION_FAX,
  _NL_IDENTIFICATION_LANGUAGE,
  _NL_IDENTIFICATION_TERRITORY,
  _NL_IDENTIFICATION_AUDIENCE,
  _NL_IDENTIFICATION_APPLICATION,
  _NL_IDENTIFICATION_ABBREVIATION,
  _NL_IDENTIFICATION_REVISION,
  _NL_IDENTIFICATION_DATE,
  _NL_IDENTIFICATION_CATEGORY,
  _NL_NUM_LC_IDENTIFICATION,

  /* This marks the highest value used.  */
  _NL_NUM
};


/* Return the current locale's value for ITEM.
   If ITEM is invalid, an empty string is returned.

   The string returned will not change until `setlocale' is called;
   it is usually in read-only memory and cannot be modified.  */

extern char *nl_langinfo __P ((nl_item __item));


__END_DECLS

#endif	/* langinfo.h */
