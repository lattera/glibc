/* nl_langinfo -- Access to locale-dependent parameters.
Copyright (C) 1995 Free Software Foundation, Inc.
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

#ifndef _LANGINFO_H
#define	_LANGINFO_H 1

#include <locale.h>		/* Define the LC_* category names.  */

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
typedef enum
{
  /* LC_TIME category: date and time formatting.  */

  /* Abbreviated days of the week. */
  ABDAY_1 = _NL_ITEM (LC_TIME, 0), /* Sun */
  ABDAY_2,
  ABDAY_3,
  ABDAY_4,
  ABDAY_5,
  ABDAY_6,
  ABDAY_7,

  /* Long-named days of the week. */
  DAY_1,			/* Sunday */
  DAY_2,			/* Monday */
  DAY_3,			/* Tuesday */
  DAY_4,			/* Wednesday */
  DAY_5,			/* Thursday */
  DAY_6,			/* Friday */
  DAY_7,			/* Saturday */

  /* Abbreviated month names.  */
  ABMON_1,			/* Jan */
  ABMON_2,
  ABMON_3,
  ABMON_4,
  ABMON_5,
  ABMON_6,
  ABMON_7,
  ABMON_8,
  ABMON_9,
  ABMON_10,
  ABMON_11,
  ABMON_12,

  /* Long month names.  */
  MON_1,			/* January */
  MON_2,
  MON_3,
  MON_4,
  MON_5,
  MON_6,
  MON_7,
  MON_8,
  MON_9,
  MON_10,
  MON_11,
  MON_12,

  AM_STR,			/* Ante meridian string.  */
  PM_STR,			/* Post meridian string.  */

  D_T_FMT,			/* Date and time format for strftime.  */
  D_FMT,			/* Date format for strftime.  */
  T_FMT,			/* Time format for strftime.  */
  T_FMT_AMPM,			/* 12-hour time format for strftime.  */

  ERA,				/* Alternate era.  */
  ERA_YEAR,			/* Year in alternate era format.  */
  ERA_D_FMT,			/* Date in alternate ear format.  */
  ALT_DIGITS,			/* Alternate symbols for digits.  */

  _NL_NUM_LC_TIME,		/* Number of indices in LC_TIME category.  */

  /* LC_CTYPE category: character classification.
     This information is accessed by the functions in <ctype.h>.
     These `nl_langinfo' names are used internally.  */
  _NL_CTYPE_CLASS_EB = _NL_ITEM (LC_CTYPE, 0),
  _NL_CTYPE_TOUPPER_EB,
  _NL_CTYPE_TOLOWER_EB,
  _NL_CTYPE_CLASS_EL,
  _NL_CTYPE_TOUPPER_EL,
  _NL_CTYPE_TOLOWER_EL,
  _NL_NUM_LC_CTYPE,

  /* LC_MONETARY category: formatting of monetary quantities.
     These items each correspond to a member of `struct lconv',
     defined in <locale.h>.  */
  INT_CURR_SYMBOL = _NL_ITEM (LC_MONETARY, 0),
  CURRENCY_SYMBOL,
  MON_DECIMAL_POINT,
  MON_THOUSANDS_SEP,
  MON_GROUPING,
  POSITIVE_SIGN,
  NEGATIVE_SIGN,
  INT_FRAC_DIGITS,
  FRAC_DIGITS,
  P_CS_PRECEDES,
  P_SEP_BY_SPACE,
  N_CS_PRECEDES,
  N_SEP_BY_SPACE,
  P_SIGN_POSN,
  N_SIGN_POSN,
  _NL_NUM_LC_MONETARY,

  /* LC_NUMERIC category: formatting of numbers.
     These also correspond to members of `struct lconv'; see <locale.h>.  */
  DECIMAL_POINT = _NL_ITEM (LC_NUMERIC, 0),
  THOUSANDS_SEP,
  GROUPING,
  _NL_NUM_LC_NUMERIC,

  YESEXPR = _NL_ITEM (LC_MESSAGES, 0), /* Regex matching ``yes'' input.  */
  NOEXPR,			/* Regex matching ``no'' input.  */
  YESSTR,			/* Output string for ``yes''.  */
  NOSTR,			/* Output string for ``no''.  */
  _NL_NUM_LC_MESSAGES,

  /* Stubs for unfinished categories.  */
  _NL_NUM_LC_COLLATE = 0,

} nl_item;


/* Return the current locale's value for ITEM.
   If ITEM is invalid, an empty string is returned.

   The string returned will not change until `setlocale' is called;
   it is usually in read-only memory and cannot be modified.  */

extern char *nl_langinfo __P ((nl_item item));


__END_DECLS

#endif	/* langinfo.h */
