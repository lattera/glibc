/* Copyright (C) 1996-2001, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOCFILE_H
#define _LOCFILE_H	1

#include <sys/uio.h>

#include "linereader.h"
#include "localedef.h"


/* Header of the locale data files.  */
struct locale_file
{
  int magic;
  int n;
};


/* Macros used in the parser.  */
#define SYNTAX_ERROR(string, args...) \
  do									      \
    {									      \
      lr_error (ldfile, string, ## args);				      \
      lr_ignore_rest (ldfile, 0);					      \
    }									      \
  while (0)


/* General handling of `copy'.  */
extern void handle_copy (struct linereader *ldfile,
			 const struct charmap_t *charmap,
			 const char *repertoire_name,
			 struct localedef_t *result, enum token_t token,
			 int locale, const char *locale_name,
			 int ignore_content);

/* Found in locfile.c.  */
extern int locfile_read (struct localedef_t *result,
			 const struct charmap_t *charmap);

/* Check validity of all the locale data.  */
extern void check_all_categories (struct localedef_t *definitions,
				  const struct charmap_t *charmap);

/* Write out all locale categories.  */
extern void write_all_categories (struct localedef_t *definitions,
				  const struct charmap_t *charmap,
				  const char *locname,
				  const char *output_path);

/* Write out the data.  */
extern void write_locale_data (const char *output_path, int catidx,
			       const char *category, size_t n_elem,
			       struct iovec *vec);


/* Entrypoints for the parsers of the individual categories.  */

/* Handle LC_CTYPE category.  */
extern void ctype_read (struct linereader *ldfile,
			struct localedef_t *result,
			const struct charmap_t *charmap,
			const char *repertoire_name,
			int ignore_content);
extern void ctype_finish (struct localedef_t *locale,
			  const struct charmap_t *charmap);
extern void ctype_output (struct localedef_t *locale,
			  const struct charmap_t *charmap,
			  const char *output_path);
extern uint32_t *find_translit (struct localedef_t *locale,
				const struct charmap_t *charmap, uint32_t wch);

/* Handle LC_COLLATE category.  */
extern void collate_read (struct linereader *ldfile,
			  struct localedef_t *result,
			  const struct charmap_t *charmap,
			  const char *repertoire_name,
			  int ignore_content);
extern void collate_finish (struct localedef_t *locale,
			    const struct charmap_t *charmap);
extern void collate_output (struct localedef_t *locale,
			    const struct charmap_t *charmap,
			    const char *output_path);

/* Handle LC_MONETARY category.  */
extern void monetary_read (struct linereader *ldfile,
			   struct localedef_t *result,
			   const struct charmap_t *charmap,
			   const char *repertoire_name,
			   int ignore_content);
extern void monetary_finish (struct localedef_t *locale,
			     const struct charmap_t *charmap);
extern void monetary_output (struct localedef_t *locale,
			     const struct charmap_t *charmap,
			     const char *output_path);

/* Handle LC_NUMERIC category.  */
extern void numeric_read (struct linereader *ldfile,
			  struct localedef_t *result,
			  const struct charmap_t *charmap,
			  const char *repertoire_name,
			  int ignore_content);
extern void numeric_finish (struct localedef_t *locale,
			    const struct charmap_t *charmap);
extern void numeric_output (struct localedef_t *locale,
			    const struct charmap_t *charmap,
			    const char *output_path);

/* Handle LC_MESSAGES category.  */
extern void messages_read (struct linereader *ldfile,
			   struct localedef_t *result,
			   const struct charmap_t *charmap,
			   const char *repertoire_name,
			   int ignore_content);
extern void messages_finish (struct localedef_t *locale,
			     const struct charmap_t *charmap);
extern void messages_output (struct localedef_t *locale,
			     const struct charmap_t *charmap,
			     const char *output_path);

/* Handle LC_TIME category.  */
extern void time_read (struct linereader *ldfile,
		       struct localedef_t *result,
		       const struct charmap_t *charmap,
		       const char *repertoire_name,
		       int ignore_content);
extern void time_finish (struct localedef_t *locale,
			 const struct charmap_t *charmap);
extern void time_output (struct localedef_t *locale,
			 const struct charmap_t *charmap,
			 const char *output_path);

/* Handle LC_PAPER category.  */
extern void paper_read (struct linereader *ldfile,
			struct localedef_t *result,
			const struct charmap_t *charmap,
			const char *repertoire_name,
			int ignore_content);
extern void paper_finish (struct localedef_t *locale,
			  const struct charmap_t *charmap);
extern void paper_output (struct localedef_t *locale,
			  const struct charmap_t *charmap,
			  const char *output_path);

/* Handle LC_NAME category.  */
extern void name_read (struct linereader *ldfile,
		       struct localedef_t *result,
		       const struct charmap_t *charmap,
		       const char *repertoire_name,
		       int ignore_content);
extern void name_finish (struct localedef_t *locale,
			 const struct charmap_t *charmap);
extern void name_output (struct localedef_t *locale,
			 const struct charmap_t *charmap,
			 const char *output_path);

/* Handle LC_ADDRESS category.  */
extern void address_read (struct linereader *ldfile,
			  struct localedef_t *result,
			  const struct charmap_t *charmap,
			  const char *repertoire_name,
			  int ignore_content);
extern void address_finish (struct localedef_t *locale,
			    const struct charmap_t *charmap);
extern void address_output (struct localedef_t *locale,
			    const struct charmap_t *charmap,
			    const char *output_path);

/* Handle LC_TELEPHONE category.  */
extern void telephone_read (struct linereader *ldfile,
			    struct localedef_t *result,
			    const struct charmap_t *charmap,
			    const char *repertoire_name,
			    int ignore_content);
extern void telephone_finish (struct localedef_t *locale,
			      const struct charmap_t *charmap);
extern void telephone_output (struct localedef_t *locale,
			      const struct charmap_t *charmap,
			      const char *output_path);

/* Handle LC_MEASUREMENT category.  */
extern void measurement_read (struct linereader *ldfile,
			      struct localedef_t *result,
			      const struct charmap_t *charmap,
			      const char *repertoire_name,
			      int ignore_content);
extern void measurement_finish (struct localedef_t *locale,
				const struct charmap_t *charmap);
extern void measurement_output (struct localedef_t *locale,
				const struct charmap_t *charmap,
				const char *output_path);

/* Handle LC_IDENTIFICATION category.  */
extern void identification_read (struct linereader *ldfile,
				 struct localedef_t *result,
				 const struct charmap_t *charmap,
				 const char *repertoire_name,
				 int ignore_content);
extern void identification_finish (struct localedef_t *locale,
				   const struct charmap_t *charmap);
extern void identification_output (struct localedef_t *locale,
				   const struct charmap_t *charmap,
				   const char *output_path);

#endif /* locfile.h */
