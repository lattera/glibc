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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.4 LOCALIZATION	<locale.h>
 */

#ifndef	_LOCALE_H

#define	_LOCALE_H	1
#include <features.h>

__BEGIN_DECLS

/* These are the possibilities for the first argument to setlocale.
   Note that although they are bit masks, they cannot be OR'd together
   to form a new argument to pass.  They must be used one at a time.  */
#define	LC_COLLATE	(1 << 0)
#define	LC_CTYPE	(1 << 1)
#define	LC_MONETARY	(1 << 2)
#define	LC_NUMERIC	(1 << 3)
#define	LC_TIME		(1 << 4)
#define	LC_RESPONSE	(1 << 5)
#define	LC_ALL		(LC_COLLATE|LC_CTYPE|LC_MONETARY|LC_NUMERIC|LC_TIME|\
			 LC_RESPONSE)


/* Structure giving information about numeric and monetary notation.  */
struct lconv
{
  /* Numeric (non-monetary) information.  */

  char *decimal_point;		/* Decimal point character.  */
  char *thousands_sep;		/* Thousands separator.  */
  /* Each element is the number of digits in each group;
     elements with higher indices are farther left.
     An element with value CHAR_MAX means that no further grouping is done.
     An element with value 0 means that the previous element is used
     for all groups farther left.  */
  char *grouping;

  /* Monetary information.  */

  /* First three chars are a currency symbol from ISO 4217.
     Fourth char is the separator.  Fifth char is '\0'.  */
  char *int_curr_symbol;
  char *currency_symbol;	/* Local currency symbol.  */
  char *mon_decimal_point;	/* Decimal point character.  */
  char *mon_thousands_sep;	/* Thousands separator.  */
  char *mon_grouping;		/* Like `grouping' element (above).  */
  char *positive_sign;		/* Sign for positive values.  */
  char *negative_sign;		/* Sign for negative values.  */
  char int_frac_digits;		/* Int'l fractional digits.  */
  char frac_digits;		/* Local fractional digits.  */
  /* 1 if currency_symbol precedes a positive value, 0 if succeeds.  */
  char p_cs_precedes;
  /* 1 iff a space separates currency_symbol from a positive value.  */
  char p_sep_by_space;
  /* 1 if currency_symbol precedes a negative value, 0 if succeeds.  */
  char n_cs_precedes;
  /* 1 iff a space separates currency_symbol from a negative value.  */
  char n_sep_by_space;
  /* Positive and negative sign positions:
     0 Parentheses surround the quantity and currency_symbol.
     1 The sign string precedes the quantity and currency_symbol.
     2 The sign string succedes the quantity and currency_symbol.
     3 The sign string immediately precedes the currency_symbol.
     4 The sign string immediately succedes the currency_symbol.  */
  char p_sign_posn;
  char n_sign_posn;
};


/* Set and/or return the current locale.  */
extern char *setlocale __P ((int __category, __const char *__locale));

/* Return the numeric/monetary information for the current locale.  */
extern struct lconv *localeconv __P ((void));

__END_DECLS

#endif /* locale.h  */
