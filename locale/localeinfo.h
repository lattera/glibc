/* localeinfo.h -- declarations for internal libc locale interfaces
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

#ifndef _LOCALEINFO_H
#define _LOCALEINFO_H 1

#include <stddef.h>
#include <langinfo.h>
#include <sys/types.h>

/* Magic number at the beginning of a locale data file for CATEGORY.  */
#define	LIMAGIC(category)	(0x051472CA ^ (category))

/* Structure describing locale data in core for a category.  */
struct locale_data
  {
    const char *filedata;	/* Region mapping the file data.  */
    off_t filesize;		/* Size of the file (and the region).  */

    unsigned int nstrings;	/* Number of strings below.  */
    const char *strings[0];	/* Items, usually pointers into `filedata'.  */
  };


/* For each category declare the variable for the current locale data.  */
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
extern const struct locale_data *_nl_current_##category;
#include "categories.def"
#undef	DEFINE_CATEGORY

extern const char *const _nl_category_names[LC_ALL];
extern const struct locale_data * *const _nl_current[LC_ALL];

/* Extract the current CATEGORY locale's string for ITEM.  */
#define _NL_CURRENT(category, item) \
  (_nl_current_##category->strings[_NL_ITEM_INDEX (item)])

/* This is used in lc-CATEGORY.c to define _nl_current_CATEGORY.  */
#define _NL_CURRENT_DEFINE(category) \
  extern const struct locale_data _nl_C_##category; \
  const struct locale_data *_nl_current_##category = &_nl_C_##category

/* Load the locale data for CATEGORY from the file specified by *NAME.
   If *NAME is "", use environment variables as specified by POSIX,
   and fill in *NAME with the actual name used.  */
extern struct locale_data *_nl_load_locale (int category, char **name);

/* Free the locale data read in by a `_nl_load_locale' call.  */
extern void _nl_free_locale (struct locale_data *);


#endif	/* localeinfo.h */
