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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "linereader.h"
#include "charset.h"
#include "stringtrans.h"


void *xmalloc (size_t __n);
void *xrealloc (void *__p, size_t __n);
char *xstrdup (const char *__str);


static struct token *get_toplvl_escape (struct linereader *lr);
static struct token *get_symname (struct linereader *lr);
static struct token *get_ident (struct linereader *lr);
static struct token *get_string (struct linereader *lr,
				 const struct charset_t *charset);


struct linereader *
lr_open (const char *fname, kw_hash_fct_t hf)
{
  FILE *fp;
  struct linereader *result;
  int n;

  if (fname == NULL || strcmp (fname, "-") == 0
      || strcmp (fname, "/dev/stdin") == 0)
    fp = stdin;
  else
    {
      fp = fopen (fname, "r");
      if (fp == NULL)
	return NULL;
    }

  result = (struct linereader *) xmalloc (sizeof (*result));

  result->fp = fp;
  result->fname = xstrdup (fname ? : "<stdin>");
  result->buf = NULL;
  result->bufsize = 0;
  result->lineno = 1;
  result->idx = 0;
  result->comment_char = '#';
  result->escape_char = '\\';
  result->translate_strings = 1;

  n = getdelim (&result->buf, &result->bufsize, '\n', result->fp);
  if (n < 0)
    {
      int save = errno;
      fclose (result->fp);
      free ((char *) result->fname);
      free (result);
      errno = save;
      return NULL;
    }

  if (n > 1 && result->buf[n - 2] == '\\' && result->buf[n - 1] == '\n')
    n -= 2;

  result->buf[n] = '\0';
  result->bufact = n;
  result->hash_fct = hf;

  return result;
}


int
lr_eof (struct linereader *lr)
{
  return lr->bufact = 0;
}


void
lr_close (struct linereader *lr)
{
  fclose (lr->fp);
  free (lr->buf);
  free (lr);
}


int
lr_next (struct linereader *lr)
{
  int n;

  n = getdelim (&lr->buf, &lr->bufsize, '\n', lr->fp);
  if (n < 0)
    return -1;

  ++lr->lineno;

  if (n > 1 && lr->buf[n - 2] == lr->escape_char && lr->buf[n - 1] == '\n')
    {
      /* An escaped newline character is substituted with a single <SP>.  */
      --n;
      lr->buf[n - 1] = ' ';
    }

  lr->buf[n] = '\0';
  lr->bufact = n;
  lr->idx = 0;

  return 0;
}


/* Defined in error.c.  */
/* This variable is incremented each time `error' is called.  */
extern unsigned int error_message_count;

/* The calling program should define program_name and set it to the
   name of the executing program.  */
extern char *program_name;


struct token *
lr_token (struct linereader *lr, const struct charset_t *charset)
{
  int ch;

  while (1)
    {
      do
	{
	  ch = lr_getc (lr);

	  if (ch == EOF)
	    {
	      lr->token.tok = tok_eof;
	      return &lr->token;
	    };

	  if (ch == '\n')
	    {
	      lr->token.tok = tok_eol;
	      return &lr->token;
	    }
	}
      while (isspace (ch));

      if (ch == EOF)
	{
	  lr->token.tok = tok_eof;
	  return &lr->token;
	};

      if (ch != lr->comment_char)
	break;

      /* Ignore rest of line.  */
      lr_ignore_rest (lr, 0);
      lr->token.tok = tok_eol;
      return &lr->token;
    }

  /* Match escape sequences.  */
  if (ch == lr->escape_char)
    return get_toplvl_escape (lr);

  /* Match ellipsis.  */
  if (ch == '.' && strncmp (&lr->buf[lr->idx], "..", 2) == 0)
    {
      lr_getc (lr);
      lr_getc (lr);
      lr->token.tok = tok_ellipsis;
      return &lr->token;
    }

  switch (ch)
    {
    case '<':
      return get_symname (lr);

    case '0' ... '9':
      lr->token.tok = tok_number;
      lr->token.val.num = ch - '0';

      while (isdigit (ch = lr_getc (lr)))
	{
	  lr->token.val.num *= 10;
	  lr->token.val.num += ch - '0';
	}
      if (isalpha (ch))
	lr_error (lr, _("garbage at end of number"));
      lr_ungetn (lr, 1);

      return &lr->token;

    case ';':
      lr->token.tok = tok_semicolon;
      return &lr->token;

    case ',':
      lr->token.tok = tok_comma;
      return &lr->token;

    case '(':
      lr->token.tok = tok_open_brace;
      return &lr->token;

    case ')':
      lr->token.tok = tok_close_brace;
      return &lr->token;

    case '"':
      return get_string (lr, charset);

    case '-':
      ch = lr_getc (lr);
      if (ch == '1')
	{
	  lr->token.tok = tok_minus1;
	  return &lr->token;
	}
      lr_ungetn (lr, 2);
      break;
    }

  return get_ident (lr);
}


static struct token *
get_toplvl_escape (struct linereader *lr)
{
  /* This is supposed to be a numeric value.  We return the
     numerical value and the number of bytes.  */
  size_t start_idx = lr->idx - 1;
  unsigned int value = 0;
  int nbytes = 0;
  int ch;

  do
    {
      unsigned int byte = 0;
      unsigned int base = 8;

      ch = lr_getc (lr);

      if (ch == 'd')
	{
	  base = 10;
	  ch = lr_getc (lr);
	}
      else if (ch == 'x')
	{
	  base = 16;
	  ch = lr_getc (lr);
	}

      if ((base == 16 && !isxdigit (ch))
	  || (base != 16 && (ch < '0' || ch >= (int) ('0' + base))))
	{
	esc_error:
	  lr->token.val.str.start = &lr->buf[start_idx];

	  while (ch != EOF && !isspace (ch))
	    ch = lr_getc (lr);
	  lr->token.val.str.len = lr->idx - start_idx;

	  lr->token.tok = tok_error;
	  return &lr->token;
	}

      if (isdigit (ch))
	byte = ch - '0';
      else
	byte = tolower (ch) - 'a' + 10;

      ch = lr_getc (lr);
      if ((base == 16 && !isxdigit (ch))
	  || (base != 16 && (ch < '0' || ch >= (int) ('0' + base))))
	goto esc_error;

      byte *= base;
      if (isdigit (ch))
	byte += ch - '0';
      else
	byte += tolower (ch) - 'a' + 10;

      ch = lr_getc (lr);
      if (base != 16 && isdigit (ch))
	{
	  byte *= base;
	  byte += ch - '0';

	  ch = lr_getc (lr);
	}

      value *= 256;
      value += byte;

      ++nbytes;
    }
  while (ch == lr->escape_char && nbytes < 4);

  if (!isspace (ch))
    lr_error (lr, _("garbage at end of character code specification"));

  lr_ungetn (lr, 1);

  lr->token.tok = tok_charcode;
  lr->token.val.charcode.val = value;
  lr->token.val.charcode.nbytes = nbytes;

  return &lr->token;
}


#define ADDC(ch)							    \
  do									    \
    {									    \
      if (bufact == bufmax)						    \
	{								    \
	  bufmax *= 2;							    \
	  buf = xrealloc (buf, bufmax);					    \
	}								    \
      buf[bufact++] = (ch);						    \
    }									    \
  while (0)


static struct token *
get_symname (struct linereader *lr)
{
  /* Symbol in brackets.  We must distinguish three kinds:
     1. reserved words
     2. ISO 10646 position values
     3. all other.  */
  char *buf;
  size_t bufact = 0;
  size_t bufmax = 56;
  const struct keyword_t *kw;
  int ch;

  buf = (char *) xmalloc (bufmax);

  do
    {
      ch = lr_getc (lr);
      if (ch == lr->escape_char)
	{
	  int c2 = lr_getc (lr);
	  ADDC (c2);

	  if (c2 == '\n')
	    ch = '\n';
	}
      else
	ADDC (ch);
    }
  while (ch != '>' && ch != '\n');

  if (ch == '\n')
    lr_error (lr, _("unterminated symbolic name"));

  /* Test for ISO 10646 position value.  */
  if (buf[0] == 'U' && (bufact == 6 || bufact == 10))
    {
      char *cp = buf + 1;
      while (cp < &buf[bufact - 1] && isxdigit (*cp))
	++cp;

      if (cp == &buf[bufact - 1])
	{
	  /* Yes, it is.  */
	  lr->token.tok = bufact == 6 ? tok_ucs2 : tok_ucs4;
	  lr->token.val.charcode.val = strtoul (buf, NULL, 16);
	  lr->token.val.charcode.nbytes = lr->token.tok == tok_ucs2 ? 2 : 4;

	  return &lr->token;
	}
    }

  /* It is a symbolic name.  Test for reserved words.  */
  kw = lr->hash_fct (buf, bufact - 1);

  if (kw != NULL && kw->symname_or_ident == 1)
    {
      lr->token.tok = kw->token;
      free (buf);
    }
  else
    {
      lr->token.tok = tok_bsymbol;

      buf[bufact] = '\0';
      buf = xrealloc (buf, bufact + 1);

      lr->token.val.str.start = buf;
      lr->token.val.str.len = bufact - 1;
    }

  return &lr->token;
}


static struct token *
get_ident (struct linereader *lr)
{
  char *buf;
  size_t bufact;
  size_t bufmax = 56;
  const struct keyword_t *kw;
  int ch;

  buf = xmalloc (bufmax);
  bufact = 0;

  ADDC (lr->buf[lr->idx - 1]);

  while (!isspace ((ch = lr_getc (lr))) && ch != '"' && ch != ';'
	 && ch != '<' && ch != ',')
    /* XXX Handle escape sequences?  */
    ADDC (ch);

  lr_ungetn (lr, 1);

  kw = lr->hash_fct (buf, bufact);

  if (kw != NULL && kw->symname_or_ident == 0)
    {
      lr->token.tok = kw->token;
      free (buf);
    }
  else
    {
      lr->token.tok = tok_ident;

      buf[bufact] = '\0';
      buf = xrealloc (buf, bufact + 1);

      lr->token.val.str.start = buf;
      lr->token.val.str.len = bufact;
    }

  return &lr->token;
}


static struct token *
get_string (struct linereader *lr, const struct charset_t *charset)
{
  int illegal_string = 0;
  char *buf, *cp;
  size_t bufact;
  size_t bufmax = 56;
  int ch;

  buf = xmalloc (bufmax);
  bufact = 0;

  while ((ch = lr_getc (lr)) != '"' && ch != '\n' && ch != EOF)
    if (ch != '<' || charset == NULL)
      {
	if (ch == lr->escape_char)
	  {
	    ch = lr_getc (lr);
	    if (ch == '\n' || ch == EOF)
	      break;
	  }
	ADDC (ch);
      }
    else
      {
	/* We have to get the value of the symbol.  */
	unsigned int value;
	size_t startidx = bufact;

	if (!lr->translate_strings)
	  ADDC ('<');

	while ((ch = lr_getc (lr)) != '>' && ch != '\n' && ch != EOF)
	  {
	    if (ch == lr->escape_char)
	      {
		ch = lr_getc (lr);
		if (ch == '\n' || ch == EOF)
		  break;
	      }
	    ADDC (ch);
	  }

	if (ch == '\n' || ch == EOF)
	  lr_error (lr, _("unterminated string"));
	else
	  if (!lr->translate_strings)
	    ADDC ('>');

	if (lr->translate_strings)
	  {
	    value = charset_find_value (&charset->char_table, &buf[startidx],
					bufact - startidx);
	    if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	      illegal_string = 1;
	    bufact = startidx;

	    if (bufmax - bufact < 8)
	      {
		bufmax *= 2;
		buf = (char *) xrealloc (buf, bufmax);
	      }

	    cp = &buf[bufact];
	    if (encode_char (value, &cp))
	      illegal_string = 1;

	    bufact = cp - buf;
	  }
      }

  /* Catch errors with trailing escape character.  */
  if (bufact > 0 && buf[bufact - 1] == lr->escape_char
      && (bufact == 1 || buf[bufact - 2] != lr->escape_char))
    {
      lr_error (lr, _("illegal escape sequence at end of string"));
      --bufact;
    }
  else if (ch == '\n' || ch == EOF)
    lr_error (lr, _("unterminated string"));

  /* Terminate string if necessary.  */
  if (lr->translate_strings)
    {
      cp = &buf[bufact];
      if (encode_char (0, &cp))
	illegal_string = 1;

      bufact = cp - buf;
    }
  else
    ADDC ('\0');

  lr->token.tok = tok_string;

  if (illegal_string)
    {
      free (buf);
      lr->token.val.str.start = NULL;
      lr->token.val.str.len = 0;
    }
  else
    {
      buf = xrealloc (buf, bufact + 1);

      lr->token.val.str.start = buf;
      lr->token.val.str.len = bufact;
    }

  return &lr->token;
}
