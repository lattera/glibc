/* localeinfo.h -- declarations for internal libc locale interfaces
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef _LOCALEINFO_H
#define _LOCALEINFO_H 1

#include <stddef.h>
#include <langinfo.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>

#include "../intl/loadinfo.h"	/* For loaded_l10nfile definition.  */

/* Magic number at the beginning of a locale data file for CATEGORY.  */
#define	LIMAGIC(category)	(0x960617de ^ (category))

/* Two special weight constants for the collation data.  */
#define FORWARD_CHAR ((wchar_t) 0xfffffffd)
#define ELLIPSIS_CHAR ((wchar_t) 0xfffffffe)
#define IGNORE_CHAR ((wchar_t) 0xffffffff)

/* We use a special value for the usage counter in `locale_data' to
   signal that this data must never be removed anymore.  */
#define MAX_USAGE_COUNT UINT_MAX

/* Structure describing locale data in core for a category.  */
struct locale_data
{
  const char *name;
  const char *filedata;		/* Region mapping the file data.  */
  off_t filesize;		/* Size of the file (and the region).  */
  int mmaped;			/* If nonzero the data is mmaped.  */

  unsigned int usage_count;	/* Counter for users.  */

  unsigned int nstrings;	/* Number of strings below.  */
  union locale_data_value
  {
    const wchar_t *wstr;
    const char *string;
    unsigned int word;
  }
  values[0];	/* Items, usually pointers into `filedata'.  */
};

/* We know three kinds of collation sorting rules.  */
enum coll_sort_rule
{
  illegal_0__,
  sort_forward,
  sort_backward,
  illegal_3__,
  sort_position,
  sort_forward_position,
  sort_backward_position,
  sort_mask
};

/* We can map the types of the entries into a few categories.  */
enum value_type
{
  none,
  string,
  stringarray,
  byte,
  bytearray,
  word
};


/* Structure to access `era' information from LC_TIME.  */
struct era_entry
{
  u_int32_t direction;		/* Contains '+' or '-'.  */
  int32_t offset;
  int32_t start_date[3];
  int32_t stop_date[3];
  const char name_fmt[0];
};


/* For each category declare the variable for the current locale data.  */
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
extern struct locale_data *_nl_current_##category;
#include "categories.def"
#undef	DEFINE_CATEGORY

extern const char *const _nl_category_names[LC_ALL + 1];
extern const size_t _nl_category_name_sizes[LC_ALL + 1];
extern struct locale_data * *const _nl_current[LC_ALL];

/* Name of the standard locale.  */
extern const char _nl_C_name[];

/* Extract the current CATEGORY locale's string for ITEM.  */
#define _NL_CURRENT(category, item) \
  (_nl_current_##category->values[_NL_ITEM_INDEX (item)].string)

/* Extract the current CATEGORY locale's word for ITEM.  */
#define _NL_CURRENT_WORD(category, item) \
  (_nl_current_##category->values[_NL_ITEM_INDEX (item)].word)

/* This is used in lc-CATEGORY.c to define _nl_current_CATEGORY.  */
#define _NL_CURRENT_DEFINE(category) \
  extern struct locale_data _nl_C_##category; \
  struct locale_data *_nl_current_##category = &_nl_C_##category

/* Load the locale data for CATEGORY from the file specified by *NAME.
   If *NAME is "", use environment variables as specified by POSIX,
   and fill in *NAME with the actual name used.  The directories
   listed in LOCALE_PATH are searched for the locale files.  */
extern struct locale_data *_nl_find_locale (const char *locale_path,
					    size_t locale_path_len,
					    int category, const char **name);

/* Try to load the file described by FILE.  */
extern void _nl_load_locale (struct loaded_l10nfile *file, int category);

/* Free the locale and give back all memory if the usage count is one.  */
extern void _nl_remove_locale (int locale, struct locale_data *data);


/* Return `era' entry which corresponds to TP.  Used in strftime.  */
extern struct era_entry *_nl_get_era_entry (const struct tm *tp);

/* Return `alt_digit' which corresponds to NUMBER.  Used in strftime.  */
extern const char *_nl_get_alt_digit (unsigned int number);


/* Global variables for LC_COLLATE category data.  */
extern const u_int32_t *__collate_table;
extern const u_int32_t *__collate_extra;
extern const u_int32_t *__collate_element_hash;
extern const char *__collate_element_strings;
extern const wchar_t *__collate_element_values;
extern const u_int32_t *__collate_symbol_hash;
extern const char *__collate_symbol_strings;
extern const u_int32_t *__collate_symbol_classes;

#endif	/* localeinfo.h */
