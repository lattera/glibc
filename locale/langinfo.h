/* Access to locale-dependent parameters.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

  _NL_TIME_NUM_ALT_DIGITS,	/* Number entries in the alt_digits arrays.  */

  _NL_TIME_ERA_NUM_ENTRIES,	/* Number entries in the era arrays.  */
  _NL_TIME_ERA_ENTRIES_EB,	/* Structure with era entries in usable form.*/
  _NL_TIME_ERA_ENTRIES_EL,

  _NL_NUM_LC_TIME,		/* Number of indices in LC_TIME category.  */

  /* LC_COLLATE category: text sorting.
     This information is accessed by the strcoll and strxfrm functions.
     These `nl_langinfo' names are used only internally.  */
  _NL_COLLATE_NRULES = _NL_ITEM (LC_COLLATE, 0),
  _NL_COLLATE_RULES,
  _NL_COLLATE_HASH_SIZE,
  _NL_COLLATE_HASH_LAYERS,
  _NL_COLLATE_TABLE_EB,
  _NL_COLLATE_TABLE_EL,
  _NL_COLLATE_UNDEFINED,
  _NL_COLLATE_EXTRA_EB,
  _NL_COLLATE_EXTRA_EL,
  _NL_COLLATE_ELEM_HASH_SIZE,
  _NL_COLLATE_ELEM_HASH_EB,
  _NL_COLLATE_ELEM_HASH_EL,
  _NL_COLLATE_ELEM_STR_POOL,
  _NL_COLLATE_ELEM_VAL_EB,
  _NL_COLLATE_ELEM_VAL_EL,
  _NL_COLLATE_SYMB_HASH_SIZE,
  _NL_COLLATE_SYMB_HASH_EB,
  _NL_COLLATE_SYMB_HASH_EL,
  _NL_COLLATE_SYMB_STR_POOL,
  _NL_COLLATE_SYMB_CLASS_EB,
  _NL_COLLATE_SYMB_CLASS_EL,
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
  _NL_CTYPE_HASH_SIZE,
  _NL_CTYPE_HASH_LAYERS,
  _NL_CTYPE_CLASS_NAMES,
  _NL_CTYPE_MAP_NAMES,
  _NL_CTYPE_WIDTH,
  _NL_CTYPE_MB_CUR_MAX,
  _NL_CTYPE_CODESET_NAME,
#ifdef __USE_XOPEN
  CODESET = _NL_CTYPE_CODESET_NAME,
#endif
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
# define CRNCYSTR			CRNCYSTR
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
#define THOUSANDS_SEP		THOUSANDS_SEP
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
