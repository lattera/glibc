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

/* Locale-specific information.  */

#ifndef	_LOCALEINFO_H

#define	_LOCALEINFO_H	1

#define	__need_size_t
#define	__need_wchar_t
#include <stddef.h>
#include <limits.h>


/* Change these if the `wchar_t' type is changed.  */
#define	WCHAR_MAX	((wchar_t) UCHAR_MAX)


/* Used by multibyte char functions.  */
typedef struct
{
  char *string;			/* Bytes.  */
  size_t len;			/* # of bytes.  */
  long int shift;		/* # of mb_char's to shift.  */
} mb_char;

struct ctype_mbchar_info
{
  size_t mb_max;		/* Max MB char length.  */
  mb_char *mb_chars;		/* MB chars.  */
};

struct ctype_ctype_info
{
  unsigned short int *ctype_b;	/* Characteristics.  */
  short int *ctype_tolower;	/* Case mappings.  */
  short int *ctype_toupper;	/* Case mappings.  */
};

struct ctype_info
{
  struct ctype_ctype_info *ctype;
  struct ctype_mbchar_info *mbchar;
};

extern __const struct ctype_info *_ctype_info;

/* These are necessary because they are used in a header file.  */
extern __const unsigned short int *__ctype_b;
extern __const short int *__ctype_tolower;
extern __const short int *__ctype_toupper;


/* Used by strcoll and strxfrm.  */
typedef struct
{
  unsigned char *values;
  size_t nvalues;
} literal_value;

typedef struct
{
  union
  {
    literal_value literal;
    /* %%% This may become a regex_t in the future.  */
    char *regexp;
  } replace, with;
  unsigned int regexp:1;
} subst;

struct collate_info
{
  size_t nsubsts;
  subst *substs;

  unsigned char *values;
  unsigned char *offsets;
};

extern __const struct collate_info *_collate_info;


/* Used by strtod, atof.  */
struct numeric_info
{
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
};

extern __const struct numeric_info *_numeric_info;


/* Used in the return value of localeconv.  */
struct monetary_info
{
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
};

extern __const struct monetary_info *_monetary_info;


/* Used by strftime, asctime.  */
struct time_info
{
  char *abbrev_wkday[7];	/* Short weekday names.  */
  char *full_wkday[7];		/* Full weekday names.  */
  char *abbrev_month[12];	/* Short month names.  */
  char *full_month[12];		/* Full month names.  */
  char *ampm[2];		/* "AM" and "PM" strings.  */

  char *date_time;		/* Appropriate date and time format.  */
  char *date;			/* Appropriate date format.  */
  char *time;			/* Appropriate time format.  */

  char *ut0;			/* Name for GMT.  */
  char *tz;			/* Default TZ value.  */
};

extern __const struct time_info *_time_info;

struct response_info
{
  /* Regexp for affirmative answers.  */
  char *yesexpr;

  /* Regexp for negative answers.  */
  char *noexpr;
};

extern __const struct response_info *_response_info;

/* Locale structure.  */
typedef struct
{
  char *name;
  int categories;

  unsigned int allocated:1;

  int subcategories;
  size_t num_sublocales;
  struct sub_locale *sublocales;

  __ptr_t *info;
} locale;

typedef struct sub_locale
{
  unsigned int pointer:1;

  int categories;
  char *name;

  locale *locale;
} sublocale;


/* This is the magic number that localeinfo object files begin with.
   In case you're wondering why I chose the value 0x051472CA, it's
   because I was born on 05-14-72 in Oakland, CA.  */
#define	LIMAGIC		0x051472CA
/* This is the magic number that precedes each category-specific section
   of a localeinfo object file.  It's the arbitrary magic number above,
   but modified by the category so that it's different from the per-file
   magic number and unique for each category.  */
#define	CATEGORY_MAGIC(x)	(LIMAGIC ^ (x))

extern __const char *__lidir, *__lidefault;

extern locale *__find_locale __P ((int categories, __const char *name));
extern locale *__new_locale __P ((locale *));
extern locale *__localefile __P ((__const char *file));
extern void __free_locale __P ((locale *));


#endif /* localeinfo.h  */
