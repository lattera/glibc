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

#ifndef _LOCALES_H
#define _LOCALES_H

#include <ctype.h>

/* Undefine following line in production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include "linereader.h"
#include "locfile-token.h"
#include "charset.h"
#include "locfile.h"
#include "localeinfo.h"


/* List of locale definition files which are used in `copy' instructions.  */
struct copy_def_list_t
{
  struct copy_def_list_t *next;

  const char *name;
  int mask;

  struct localedef_t *locale;

  struct
  {
    void *data;
    size_t len;
  } binary[6];
};

extern struct copy_def_list_t copy_posix;


/* Header of the locale data files.  */
struct locale_file
{
  int magic;
  int n;
};


/* Handle LC_CTYPE category.  */

static inline unsigned int
charclass_to_bit (enum token_t tok)
{
  static unsigned int lastbit = _ISalnum;

  switch (tok)
    {
#define CLASS(name) case tok_##name: return _IS##name
    CLASS (upper);
    CLASS (lower);
    CLASS (alpha);
    CLASS (digit);
    CLASS (alnum);
    CLASS (space);
    CLASS (cntrl);
    CLASS (punct);
    CLASS (graph);
    CLASS (print);
    CLASS (xdigit);
    CLASS (blank);
#undef CLASS
    case tok_string:
      lastbit <<= 1;
      if (lastbit == 0ul)
	/* Exit status 2 means a limitation in the implementation is
	   exceeded.  */
	error (2, 0, _("too many character classes defined"));
      return lastbit;
    default:
      assert (1 == 0);
    }
  return 0;
}

/* Remember name of newly created charclass.  */
void ctype_startup (struct linereader *lr, struct localedef_t *locale,
		    struct charset_t *charset);
void ctype_finish (struct localedef_t *locale, struct charset_t *charset);

void ctype_output (struct localedef_t *locale, struct charset_t *charset,
		   const char *output_path);

int ctype_is_charclass (struct linereader *lr, struct localedef_t *locale,
			const char *name);
void ctype_class_new (struct linereader *lr, struct localedef_t *locale,
		      enum token_t tok, struct token *code,
		      struct charset_t *charset);
void ctype_class_start (struct linereader *lr, struct localedef_t *locale,
			enum token_t tok, const char *name,
			struct charset_t *charset);
void ctype_class_from (struct linereader *lr, struct localedef_t *locale,
		       struct token *code, struct charset_t *charset);
void ctype_class_to (struct linereader *lr, struct localedef_t *locale,
		     struct token *code, struct charset_t *charset);
void ctype_class_end (struct linereader *lr, struct localedef_t *locale);

int ctype_is_charconv (struct linereader *lr, struct localedef_t *locale,
		       const char *name);
void ctype_map_new (struct linereader *lr, struct localedef_t *locale,
		    enum token_t tok, struct token *code,
		    struct charset_t *charset);
void ctype_map_start (struct linereader *lr, struct localedef_t *locale,
		      enum token_t tok, const char *name,
		      struct charset_t *charset);
void ctype_map_from (struct linereader *lr, struct localedef_t *locale,
		     struct token *code, struct charset_t *charset);
void ctype_map_to (struct linereader *lr, struct localedef_t *locale,
		   struct token *code, struct charset_t *charset);
void ctype_map_end (struct linereader *lr, struct localedef_t *locale);


/* Handle LC_COLLATE category.  */

void collate_startup (struct linereader *lr, struct localedef_t *locale,
		      struct charset_t *charset);

void collate_finish (struct localedef_t *locale,
		     struct charset_t *charset);

void collate_output (struct localedef_t *locale, struct charset_t *charset,
		     const char *output_path);

void collate_element_to (struct linereader *lr, struct localedef_t *locale,
			 struct token *code, struct charset_t *charset);
void collate_element_from (struct linereader *lr,
			   struct localedef_t *locale, struct token *code,
			   struct charset_t *charset);
void collate_symbol (struct linereader *lr, struct localedef_t *locale,
		     struct token *code, struct charset_t *charset);
void collate_new_order (struct linereader *lr, struct localedef_t *locale,
			enum coll_sort_rule sort_rule);
void collate_build_arrays (struct linereader *lr,
			   struct localedef_t *locale);
int collate_order_elem (struct linereader *lr, struct localedef_t *locale,
			struct token *code, struct charset_t *charset);
int collate_weight_bsymbol (struct linereader *lr,
			    struct localedef_t *locale,
			    struct token *code, struct charset_t *charset);
int collate_next_weight (struct linereader *lr,
			 struct localedef_t *locale);
int collate_simple_weight (struct linereader *lr,
			   struct localedef_t *locale,
			   struct token *code, struct charset_t *charset);
void collate_end_weight (struct linereader *lr,
			 struct localedef_t *locale);


/* Handle LC_MONETARY category.  */

void monetary_startup (struct linereader *lr, struct localedef_t *locale,
		       struct charset_t *charset);

void monetary_finish (struct localedef_t *locale);

void monetary_output (struct localedef_t *locale, const char *output_path);

void monetary_add (struct linereader *lr, struct localedef_t *locale,
		   enum token_t tok, struct token *code,
		   struct charset_t *charset);


/* Handle LC_NUMERIC category.  */

void numeric_startup (struct linereader *lr, struct localedef_t *locale,
		      struct charset_t *charset);

void numeric_finish (struct localedef_t *locale);

void numeric_output (struct localedef_t *locale, const char *output_path);

void numeric_add (struct linereader *lr, struct localedef_t *locale,
		  enum token_t tok, struct token *code,
		  struct charset_t *charset);


/* Handle LC_TIME category.  */

void time_startup (struct linereader *lr, struct localedef_t *locale,
		   struct charset_t *charset);

void time_finish (struct localedef_t *locale);

void time_output (struct localedef_t *locale, const char *output_path);

void time_add (struct linereader *lr, struct localedef_t *locale,
	       enum token_t tok, struct token *code,
	       struct charset_t *charset);


/* Handle LC_MESSAGES category.  */

void messages_startup (struct linereader *lr, struct localedef_t *locale,
		       struct charset_t *charset);

void messages_finish (struct localedef_t *locale);

void messages_output (struct localedef_t *locale, const char *output_path);

void messages_add (struct linereader *lr, struct localedef_t *locale,
		   enum token_t tok, struct token *code,
		   struct charset_t *charset);


#endif /* locales.h */
