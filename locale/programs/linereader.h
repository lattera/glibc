/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _LINEREADER_H
#define _LINEREADER_H 1

#include <ctype.h>
#include <libintl.h>
#include <stdio.h>

#include "error.h"
#include "locfile-token.h"


typedef const struct keyword_t *(*kw_hash_fct_t) (const char *, int);
struct charset_t;


struct token
{
  enum token_t tok;
  union
  {
    struct
    {
      char *start;
      size_t len;
    } str;
    unsigned long int num;
    struct
    {
      unsigned int val;
      int nbytes;
    } charcode;
  } val;
};


struct linereader
{
  FILE *fp;
  const char *fname;
  char *buf;
  size_t bufsize;
  size_t bufact;
  size_t lineno;

  size_t idx;

  char comment_char;
  char escape_char;

  struct token token;

  int translate_strings;

  kw_hash_fct_t hash_fct;
};


/* Functions defined in linereader.c.  */
struct linereader *lr_open (const char *fname, kw_hash_fct_t hf);
int lr_eof (struct linereader *lr);
void lr_close (struct linereader *lr);
int lr_next (struct linereader *lr);
struct token *lr_token (struct linereader *lr,
			const struct charset_t *charset);


#define lr_error(lr, fmt, args...) \
  error_at_line (0, 0, lr->fname, lr->lineno, fmt, ## args)



static inline int
lr_getc (struct linereader *lr)
{
  if (lr->idx == lr->bufact)
    {
      if (lr->bufact != 0)
	if (lr_next (lr) < 0)
	  return EOF;

      if (lr->bufact == 0)
	return EOF;
    }

  return lr->buf[lr->idx] == '\32' ? EOF : lr->buf[lr->idx++];
}


static inline int
lr_ungetc (struct linereader *lr, int ch)
{
  if (lr->idx == 0)
    return -1;

  lr->buf[--lr->idx] = ch;
  return 0;
}


static inline int
lr_ungetn (struct linereader *lr, int n)
{
  if (lr->idx < n)
    return -1;

  lr->idx -= n;
  return 0;
}


static inline void
lr_ignore_rest (struct linereader *lr, int verbose)
{
  if (verbose)
    {
      while (isspace (lr->buf[lr->idx]) && lr->buf[lr->idx] != '\n'
	     && lr->buf[lr->idx] != lr->comment_char)
	if (lr->buf[lr->idx] == '\0')
	  {
	    if (lr_next (lr) < 0)
	      return;
	  }
	else
	  ++lr->idx;

      if (lr->buf[lr->idx] != '\n' &&lr->buf[lr->idx] != lr->comment_char)
	lr_error (lr, _("trailing garbage at end of line"));
    }

  /* Ignore continued line.  */
  while (lr->bufact > 0 && lr->buf[lr->bufact - 1] != '\n')
    if (lr_next (lr) < 0)
      break;

  lr->idx = lr->bufact;
}


#endif /* linereader.h */
