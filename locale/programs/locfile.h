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

#ifndef _LOCFILE_H
#define _LOCFILE_H

#include <sys/uio.h>

#include "charset.h"

/* Opaque types for the different locales.  */
struct locale_ctype_t;
struct locale_collate_t;
struct locale_monetary_t;
struct locale_numeric_t;
struct locale_time_t;
struct locale_messages_t;

struct localedef_t
{
  int failed;

  int avail;
  int binary;

  union
  {
    void *generic;
    struct locale_ctype_t *ctype;
    struct locale_collate_t *collate;
    struct locale_monetary_t *monetary;
    struct locale_numeric_t *numeric;
    struct locale_time_t *time;
    struct locale_messages_t *messages;
  } categories[6];

  size_t len[6];
};

/* Declared in localedef.c.  */
extern int be_quiet;
extern const char *repertoiremap;

/* Found in localedef.c.  */
void def_to_process (const char *name, int category);


/* Found in locfile.c.  */
struct localedef_t *locfile_read (const char *filename,
				  struct charset_t *charset);

void check_all_categories (struct localedef_t *locale,
			   struct charset_t *charset);

void write_all_categories (struct localedef_t *locale,
			   struct charset_t *charset, const char *output_path);


void write_locale_data (const char *output_path, const char *category,
			size_t n_elem, struct iovec *vec);

#endif /* locfile.h */
