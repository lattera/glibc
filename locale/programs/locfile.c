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

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include "locfile.h"
#include "linereader.h"
#include "localeinfo.h"
#include "locales.h"


/* Uncomment the following line in the production version. */
/* #define NDEBUG 1 */
#include <assert.h>

/* Define the lookup function.  */
#include "locfile-kw.h"


/* Some useful macros.  */
#define MIN(a, b) (__extension__ ({ typeof (a) _a = (a);		      \
				    typeof (b) _b = (b);		      \
				    _a < _b ? _a : _b; }))


void *xmalloc (size_t __n);
char *xstrdup (const char *__str);

struct localedef_t *
locfile_read (const char *filename, struct charset_t *charset)
{
  struct repertoire_t *repertoire = NULL;
  struct linereader *ldfile;
  struct localedef_t *result;
  int state;
  enum token_t expected_tok = tok_none;
  const char *expected_str = NULL;
  enum token_t ctype_tok_sym = tok_none;
  const char *ctype_tok_str = NULL;
  int copy_category = 0;
  int cnt;

  /* Allocate space for result.  */
  result = (struct localedef_t *) xmalloc (sizeof (struct localedef_t));
  memset (result, '\0', sizeof (struct localedef_t));

  ldfile = lr_open (filename, locfile_hash);
  if (ldfile == NULL)
    {
      if (filename[0] != '/')
	{
	  char *i18npath = __secure_getenv ("I18NPATH");
	  if (i18npath != NULL && *i18npath != '\0')
	    {
	      char path[strlen (filename) + 1 + strlen (i18npath)
		        + sizeof ("/locales/") - 1];
	      char *next;
	      i18npath = strdupa (i18npath);


	      while (ldfile == NULL
		     && (next = strsep (&i18npath, ":")) != NULL)
		{
		  stpcpy (stpcpy (stpcpy (path, next), "/locales/"), filename);

		  ldfile = lr_open (path, locfile_hash);
		}
	    }

	  /* Test in the default directory.  */
	  if (ldfile == NULL)
	    {
	      char path[strlen (filename) + 1 + sizeof (LOCSRCDIR)];

	      stpcpy (stpcpy (stpcpy (path, LOCSRCDIR), "/"), filename);
	      ldfile = lr_open (path, locfile_hash);
	    }
	}

      if (ldfile == NULL)
	{
	  result->failed = 1;
	  return result;
	}
    }

#define HANDLE_COPY(category, token, string)				      \
  if (nowtok == tok_copy)						      \
    {									      \
      copy_category = category;						      \
      expected_tok = token;						      \
      expected_str = string;						      \
      state = 8;							      \
      continue;								      \
    }									      \
  ++state

#define LOCALE_PROLOG(token, string)					      \
  if (nowtok == tok_eol)						      \
    /* Ignore empty lines.  */						      \
    continue;								      \
  if (nowtok == tok_end)						      \
    {									      \
      expected_tok = token;						      \
      expected_str = string;						      \
      state = 4;							      \
      continue;								      \
    }									      \
  if (nowtok == tok_copy)						      \
    goto only_copy;


#define READ_STRING(fn, errlabel)					      \
  do									      \
    {									      \
      arg = lr_token (ldfile, charset);					      \
      if (arg->tok != tok_string)					      \
	goto errlabel;							      \
      fn (ldfile, result, nowtok, arg, charset);			      \
      lr_ignore_rest (ldfile, 1);					      \
    }									      \
  while (0)

#define READ_STRING_LIST(fn, errlabel)					      \
  do									      \
    {									      \
      arg = lr_token (ldfile, charset);					      \
      while (arg->tok == tok_string)					      \
	{								      \
	  fn (ldfile, result, nowtok, arg, charset);			      \
	  arg = lr_token (ldfile, charset);				      \
	  if (arg->tok != tok_semicolon)				      \
	    break;							      \
	  arg = lr_token (ldfile, charset);				      \
	}								      \
      if (arg->tok != tok_eol)						      \
	goto errlabel;							      \
    }									      \
  while (0)

#define READ_NUMBER(fn, errlabel)					      \
  do									      \
    {									      \
      arg = lr_token (ldfile, charset);					      \
      if (arg->tok != tok_minus1 && arg->tok != tok_number)		      \
	goto errlabel;							      \
      fn (ldfile, result, nowtok, arg, charset);			      \
      lr_ignore_rest (ldfile, 1);					      \
    }									      \
  while (0)

#define READ_NUMBER_LIST(fn, errlabel)					      \
  do									      \
    {									      \
      arg = lr_token (ldfile, charset);					      \
      while (arg->tok == tok_minus1 || arg->tok == tok_number)		      \
	{								      \
	  fn (ldfile, result, nowtok, arg, charset);			      \
	  arg = lr_token (ldfile, charset);				      \
	  if (arg->tok != tok_semicolon)				      \
	    break;							      \
	  arg = lr_token (ldfile, charset);				      \
	}								      \
      if (arg->tok != tok_eol)						      \
	goto errlabel;							      \
    }									      \
  while (0)

#define SYNTAX_ERROR(string)						      \
  lr_error (ldfile, string);						      \
  lr_ignore_rest (ldfile, 0);


  /* Parse locale definition file and store result in RESULT.  */
  state = 1;
  while (1)
    {
      /* What's on?  */
      struct token *now = lr_token (ldfile, charset);
      enum token_t nowtok = now->tok;
      struct token *arg;

      if (nowtok == tok_eof)
	break;

      switch (state)
	{
	case 1:
	  /* The beginning.  We expect the special declarations, EOL or
	     the start of any locale.  */
	  if (nowtok == tok_eol)
	    /* Ignore empty lines.  */
	    continue;

	  switch (nowtok)
	    {
	    case tok_escape_char:
	    case tok_comment_char:
	      /* We need an argument.  */
	      arg = lr_token (ldfile, charset);

	      if (arg->tok != tok_ident)
		{
		  SYNTAX_ERROR (_("bad argument"));
		  continue;
		}

	      if (arg->val.str.len != 1)
		{
		  lr_error (ldfile, _("\
argument to `%s' must be a single character"),
			    nowtok == tok_escape_char ? "escape_char"
						      : "comment_char");

		  lr_ignore_rest (ldfile, 0);
		  continue;
		}

	      if (nowtok == tok_escape_char)
		ldfile->escape_char = *arg->val.str.start;
	      else
		ldfile->comment_char = *arg->val.str.start;
	      break;

	    case tok_repertoiremap:
	      /* We need an argument.  */
	      arg = lr_token (ldfile, charset);

	      if (arg->tok != tok_ident)
		{
		  SYNTAX_ERROR (_("bad argument"));
		  continue;
		}

	      if (repertoiremap == NULL)
		{
		  repertoiremap = memcpy (xmalloc (arg->val.str.len + 1),
					  arg->val.str.start,
					  arg->val.str.len);
		  ((char *) repertoiremap)[arg->val.str.len] = '\0';
		}

	      lr_ignore_rest (ldfile, 1);
	      continue;

	    case tok_lc_ctype:
	      if (repertoire == NULL)
		{
		  /* Read the repertoire map now.  */
		  if (repertoiremap == NULL)
		    /* This is fatal.  */
		    error (4, 0,
			   _("no repertoire map specified: cannot proceed"));

		  repertoire = repertoire_read (repertoiremap);
		  if (repertoire == NULL)
		    /* This is also fatal.  */
		    error (4, errno, _("cannot read repertoire map `%s'"),
			   repertoiremap);
		}
	      state = 2;
	      break;

	    case tok_lc_collate:
	      if (repertoire == NULL)
		{
		  /* Read the repertoire map now.  */
		  if (repertoiremap == NULL)
		    /* This is fatal.  */
		    error (4, 0,
			   _("no repertoire map specified: cannot proceed"));

		  repertoire = repertoire_read (repertoiremap);
		  if (repertoire == NULL)
		    /* This is also fatal.  */
		    error (4, errno, _("cannot read repertoire map `%s'"),
			   repertoiremap);
		}
	      state = 10;
	      break;

	    case tok_lc_monetary:
	      if (repertoire == NULL)
		{
		  /* Read the repertoire map now.  */
		  if (repertoiremap == NULL)
		    /* This is fatal.  */
		    error (4, 0,
			   _("no repertoire map specified: cannot proceed"));

		  repertoire = repertoire_read (repertoiremap);
		  if (repertoire == NULL)
		    /* This is also fatal.  */
		    error (4, errno, _("cannot read repertoire map `%s'"),
			   repertoiremap);
		}
	      state = 20;
	      break;

	    case tok_lc_numeric:
	      if (repertoire == NULL)
		{
		  /* Read the repertoire map now.  */
		  if (repertoiremap == NULL)
		    /* This is fatal.  */
		    error (4, 0,
			   _("no repertoire map specified: cannot proceed"));

		  repertoire = repertoire_read (repertoiremap);
		  if (repertoire == NULL)
		    /* This is also fatal.  */
		    error (4, errno, _("cannot read repertoire map `%s'"),
			   repertoiremap);
		}
	      state = 30;
	      break;

	    case tok_lc_time:
	      if (repertoire == NULL)
		{
		  /* Read the repertoire map now.  */
		  if (repertoiremap == NULL)
		    /* This is fatal.  */
		    error (4, 0,
			   _("no repertoire map specified: cannot proceed"));

		  repertoire = repertoire_read (repertoiremap);
		  if (repertoire == NULL)
		    /* This is also fatal.  */
		    error (4, errno, _("cannot read repertoire map `%s'"),
			   repertoiremap);
		}
	      state = 40;
	      break;

	    case tok_lc_messages:
	      if (repertoire == NULL)
		{
		  /* Read the repertoire map now.  */
		  if (repertoiremap == NULL)
		    /* This is fatal.  */
		    error (4, 0,
			   _("no repertoire map specified: cannot proceed"));

		  repertoire = repertoire_read (repertoiremap);
		  if (repertoire == NULL)
		    /* This is also fatal.  */
		    error (4, errno, _("cannot read repertoire map `%s'"),
			   repertoiremap);
		}
	      state = 50;
	      break;

	    default:
	      SYNTAX_ERROR (_("\
syntax error: not inside a locale definition section"));
	      continue;
	    }
	  lr_ignore_rest (ldfile, 1);
	  continue;

	case 2:
	  HANDLE_COPY (LC_CTYPE, tok_lc_ctype, "LC_CYTPE");

	  ctype_startup (ldfile, result, charset);
	  /* FALLTHROUGH */

	case 3:
	  /* Here we accept all the character classes, tolower/toupper,
	     and following ANSI C:1995 self-defined classes.  */
	  LOCALE_PROLOG (tok_lc_ctype, "LC_CTYPE");

	  if (nowtok == tok_charclass)
	    {
	      READ_STRING_LIST (ctype_class_new, bad_new_charclass);
	      continue;
	    bad_new_charclass:
	      SYNTAX_ERROR (_("\
syntax error in definition of new character class"));
	      continue;
	    }

	  if (nowtok == tok_charconv)
	    {
	      READ_STRING_LIST (ctype_map_new, bad_new_charconv);
	      continue;
	    bad_new_charconv:
	      SYNTAX_ERROR (_("\
syntax error in definition of new character map"));
	      continue;
	    }

	  if (nowtok == tok_upper || nowtok == tok_lower
	      || nowtok == tok_alpha || nowtok == tok_digit
	      || nowtok == tok_alnum || nowtok == tok_space
	      || nowtok == tok_cntrl || nowtok == tok_punct
	      || nowtok == tok_graph || nowtok == tok_print
	      || nowtok == tok_xdigit || nowtok == tok_blank)
	    {
	      ctype_tok_sym = nowtok;
	      ctype_tok_str = NULL;
	      state = 5;
	      continue;
	    }

	  if (nowtok == tok_toupper|| nowtok == tok_tolower)
	    {
	      ctype_tok_sym = nowtok;
	      ctype_tok_str = NULL;
	      state = 6;
	      continue;
	    }

	  if (nowtok != tok_ident)
	    goto bad_charclass;

	  /* We possibly have a self-defined character class.  */
	  if (ctype_is_charclass (ldfile, result, now->val.str.start))
	    {
	      ctype_tok_sym = nowtok;
	      ctype_tok_str = now->val.str.start;
	      state = 5;
	      continue;
	    }

	  /* ...or a self-defined character map.  */
	  if (ctype_is_charconv (ldfile, result, now->val.str.start))
	    {
	      ctype_tok_sym = nowtok;
	      ctype_tok_str = now->val.str.start;
	      state = 6;
	      continue;
	    }

	  SYNTAX_ERROR (_("syntax error in definition of LC_CTYPE category"));
	  continue;

	case 4:
	  /* Handle `END xxx'.  */
	  if (nowtok != expected_tok)
	    lr_error (ldfile, _("\
`%1$s' definition does not end with `END %1$s'"), expected_str);

	  lr_ignore_rest (ldfile, nowtok == expected_tok);
	  state = 1;
	  continue;

	case 5:
	  /* Here we expect a semicolon separated list of bsymbols.  The
	     bit to be set in the word is given in CHARCLASS_BIT.  */
	  arg = now;

	  ctype_class_start (ldfile, result, ctype_tok_sym, ctype_tok_str,
			     charset);

	  while (arg->tok != tok_eol)
	    {
	      /* Any token other than a bsymbol is an error.  */
	      if (arg->tok != tok_bsymbol)
		{
		bad_charclass:
		  SYNTAX_ERROR (_("\
syntax error in character class definition"));
		  break;
		}

	      /* Lookup value for token and write into array.  */
	      ctype_class_from (ldfile, result, arg, charset);

	      arg = lr_token (ldfile, charset);
	      if (arg->tok == tok_semicolon)
		arg = lr_token (ldfile, charset);
	      else if (arg->tok != tok_eol)
		goto bad_charclass;

	      /* Look for ellipsis.  */
	      if (arg->tok == tok_ellipsis)
		{
		  arg = lr_token (ldfile, charset);
		  if (arg->tok != tok_semicolon)
		    goto bad_charclass;

		  arg = lr_token (ldfile, charset);
		  if (arg->tok != tok_bsymbol)
		    goto bad_charclass;

		  /* Write range starting at LAST to ARG->VAL.  */
		  ctype_class_to (ldfile, result, arg, charset);

		  arg = lr_token (ldfile, charset);
		  if (arg->tok == tok_semicolon)
		    arg = lr_token (ldfile, charset);
		  else if (arg->tok != tok_eol)
		    goto bad_charclass;
		}
	  }

	  /* Mark class as already seen.  */
	  ctype_class_end (ldfile, result);
	  state = 3;

	  continue;

	case 6:
	  /* Here we expect a list of character mappings.  Note: the
	     first opening brace is already matched.  */
	  ctype_map_start (ldfile, result, ctype_tok_sym, ctype_tok_str,
			   charset);

	  while (1)
	    {
	      /* Match ( bsymbol , bsymbol )  */
	      if (now->tok != tok_open_brace)
		goto bad_charconv;

	      now = lr_token (ldfile, charset);
	      if (now->tok != tok_bsymbol)
		{
		bad_charconv:
		  SYNTAX_ERROR (_("\
syntax error in character conversion definition"));
		  state = 3;
		  break;
		}

	      /* Lookup arg and assign to FROM.  */
	      ctype_map_from (ldfile, result, now, charset);

	      now = lr_token (ldfile, charset);
	      if (now->tok != tok_comma)
		goto bad_charconv;

	      now = lr_token (ldfile, charset);
	      if (now->tok != tok_bsymbol)
		goto bad_charconv;

	      /* Lookup arg and assign to TO.  */
	      ctype_map_to (ldfile, result, now, charset);

	      now = lr_token (ldfile, charset);
	      if (now->tok != tok_close_brace)
		goto bad_charconv;

	      now = lr_token (ldfile, charset);
	      if (now->tok == tok_eol)
		{
		  state = 3;
		  break;
		}
	      if (now->tok != tok_semicolon)
		goto bad_charconv;

	      now = lr_token (ldfile, charset);
	    }

	  ctype_map_end (ldfile, result);
	  continue;

	case 8:
	  {
	    /* We have seen `copy'.  First match the argument.  */
	    int warned = 0;

	    if (nowtok != tok_string)
	      lr_error (ldfile, _("expect string argument for `copy'"));
	    else
	      def_to_process (now->val.str.start, 1 << copy_category);

	    lr_ignore_rest (ldfile, nowtok == tok_string);

	    /* The rest of the line must be empty
	       and the next keyword must be `END xxx'.  */

	    while (lr_token (ldfile, charset)->tok != tok_end)
	      {
		if (warned == 0)
		  {
		  only_copy:
		    lr_error (ldfile, _("\
no other keyword shall be specified when `copy' is used"));
		    warned = 1;
		  }

		lr_ignore_rest (ldfile, 0);
	      }

	    state = 4;
	  }
	  continue;

	case 10:
	  HANDLE_COPY (LC_COLLATE, tok_lc_collate, "LC_COLLATE");

	  collate_startup (ldfile, result, charset);
	  /* FALLTHROUGH */

	case 11:
	  /* Process the LC_COLLATE section.  We expect `END LC_COLLATE'
	     any of the collation specifications, or any bsymbol.  */
	  LOCALE_PROLOG (tok_lc_collate, "LC_COLLATE");

	  if (nowtok == tok_order_start)
	    {
	      state = 12;
	      continue;
	    }

	  if (nowtok != tok_collating_element
	      && nowtok != tok_collating_symbol)
	    {
	    bad_collation:
	      lr_error (ldfile, _("\
syntax error in collation definition"));
	      lr_ignore_rest (ldfile, 0);
	      continue;
	    }

	  /* Get argument.  */
	  arg = lr_token (ldfile, charset);
	  if (arg->tok != tok_bsymbol)
	    {
	      lr_error (ldfile, _("\
collation symbol expected after `%s'"),
			nowtok == tok_collating_element
			? "collating-element" : "collating-symbol");
	      lr_ignore_rest (ldfile, 0);
	      continue;
	    }

	  if (nowtok == tok_collating_element)
	    {
	      /* Save to-value as new name.  */
	      collate_element_to (ldfile, result, arg, charset);

	      arg = lr_token (ldfile, charset);
	      if (arg->tok != tok_from)
		{
		  lr_error (ldfile, _("\
`from' expected after first argument to `collating-element'"));
		  lr_ignore_rest (ldfile, 0);
		  continue;
		}

	      arg = lr_token (ldfile, charset);
	      if (arg->tok != tok_string)
		{
		  lr_error (ldfile, _("\
from-value of `collating-element' must be a string"));
		  lr_ignore_rest (ldfile, 0);
		  continue;
		}

	      /* Enter new collating element.  */
	      collate_element_from (ldfile, result, arg, charset);
	    }
	  else
	    /* Enter new collating symbol into table.  */
	    collate_symbol (ldfile, result, arg, charset);

	  lr_ignore_rest (ldfile, 1);
	  continue;

	case 12:
	  /* We parse the rest of the line containing `order_start'.
	     In any case we continue with parsing the symbols.  */
	  state = 13;

	  cnt = 0;
	  while (now->tok != tok_eol)
	    {
	      int collation_method = 0;

	      ++cnt;

	      do
		{
		  if (now->tok == tok_forward)
		    collation_method |= sort_forward;
		  else if (now->tok == tok_backward)
		    collation_method |= sort_backward;
		  else if (now->tok == tok_position)
		    collation_method |= sort_position;
		  else
		    {
		      lr_error (ldfile, _("unknown collation directive"));
		      lr_ignore_rest (ldfile, 0);
		      continue;
		    }

		  now = lr_token (ldfile, charset);
		}
	      while (now->tok == tok_comma
		     && ((now = lr_token (ldfile, charset)) != tok_none));

	      /* Check for consistency: forward and backwards are
		 mutually exclusive.  */
	      if ((collation_method & sort_forward) != 0
		  && (collation_method & sort_backward) != 0)
		{
		  lr_error (ldfile, _("\
sorting order `forward' and `backward' are mutually exclusive"));
		  /* The recover clear the backward flag.  */
		  collation_method &= ~sort_backward;
		}

	      /* ??? I don't know whether this is correct but while
		 thinking about the `strcoll' functions I found that I
		 need a direction when performing position depended
		 collation.  So I assume here that implicitly the
		 direction `forward' is given when `position' alone is
		 written.  --drepper  */
	      if (collation_method == sort_position)
		collation_method |= sort_forward;

	      /* Enter info about next collation order.  */
	      collate_new_order (ldfile, result, collation_method);

	      if (now->tok != tok_eol && now->tok != tok_semicolon)
		{
		  lr_error (ldfile, _("\
syntax error in `order_start' directive"));
		  lr_ignore_rest (ldfile, 0);
		  break;
		}

	      if (now->tok == tok_semicolon)
		now = lr_token (ldfile, charset);
	    }

	  /* If no argument to `order_start' is given, one `forward'
	     argument is implicitly assumed.  */
	  if (cnt == 0)
	    collate_new_order (ldfile, result, sort_forward);


	  /* We now know about all sorting rules.  */
	  collate_build_arrays (ldfile, result);

	  continue;

	case 13:
	  /* We read one symbol a line until `order_end' is found.  */
	  {
	    static int last_correct = 1;

	    if (nowtok == tok_order_end)
	      {
		state = 14;
		lr_ignore_rest (ldfile, 1);
		continue;
	      }

	    /* Ignore empty lines.  */
	    if (nowtok == tok_eol)
	      continue;

	    if (nowtok != tok_bsymbol && nowtok != tok_undefined
		&& nowtok != tok_ellipsis)
	      {
		if (last_correct == 1)
		  {
		    lr_error (ldfile, _("\
syntax error in collating order definition"));
		    last_correct = 0;
		  }
		lr_ignore_rest (ldfile, 0);
		continue;
	      }
	    else
	      {
		last_correct = 1;

		/* Remember current token.  */
		if (collate_order_elem (ldfile, result, now, charset) < 0)
		  continue;
	      }

	    /* Read optional arguments.  */
	    arg = lr_token (ldfile, charset);
	    while (arg->tok != tok_eol)
	      {
		if (arg->tok != tok_ignore && arg->tok != tok_ellipsis
		    && arg->tok != tok_bsymbol && arg->tok != tok_string)
		  break;

		if (arg->tok == tok_ignore || arg->tok == tok_ellipsis
		    || arg->tok == tok_string)
		  {
		    /* Call handler for simple weights.  */
		    if (collate_simple_weight (ldfile, result, arg, charset)
			< 0)
		      goto illegal_weight;

		    arg = lr_token (ldfile, charset);
		  }
		else
		  do
		    {
		      /* Collect char.  */
		      int ok = collate_weight_bsymbol (ldfile, result, arg,
						       charset);
		      if (ok < 0)
			goto illegal_weight;

		      arg = lr_token (ldfile, charset);
		    }
		  while (arg->tok == tok_bsymbol);

		/* Are there more weights?  */
		if (arg->tok != tok_semicolon)
		  break;

		/* Yes, prepare next weight.  */
		if (collate_next_weight (ldfile, result) < 0)
		  goto illegal_weight;

		arg = lr_token (ldfile, charset);
	      }

	    if (arg->tok != tok_eol)
	      {
		SYNTAX_ERROR (_("syntax error in order specification"));
	      }

	    collate_end_weight (ldfile, result);
	  illegal_weight:
	  }
	  continue;

	case 14:
	  /* Following to the `order_end' keyword we don't expect
	     anything but the `END'.  */
	  if (nowtok == tok_eol)
	    continue;

	  if (nowtok != tok_end)
	    goto bad_collation;

	  expected_tok = tok_lc_collate;
	  expected_str = "LC_COLLATE";
	  state = 4;

	  ldfile->translate_strings = 1;
	  continue;

	case 20:
	  HANDLE_COPY (LC_MONETARY, tok_lc_monetary, "LC_MONETARY");

	  monetary_startup (ldfile, result, charset);
	  /* FALLTHROUGH */

	case 21:
	  LOCALE_PROLOG (tok_lc_monetary, "LC_MONETARY");

	  switch (nowtok)
	    {
	    case tok_int_curr_symbol:
	    case tok_currency_symbol:
	    case tok_mon_decimal_point:
	    case tok_mon_thousands_sep:
	    case tok_positive_sign:
	    case tok_negative_sign:
	      READ_STRING (monetary_add, bad_monetary);
	      break;

	    case tok_int_frac_digits:
	    case tok_frac_digits:
	    case tok_p_cs_precedes:
	    case tok_p_sep_by_space:
	    case tok_n_cs_precedes:
	    case tok_n_sep_by_space:
	    case tok_p_sign_posn:
	    case tok_n_sign_posn:
	      READ_NUMBER (monetary_add, bad_monetary);
	      break;

	    case tok_mon_grouping:
	      /* We have a semicolon separated list of integers.  */
	      READ_NUMBER_LIST (monetary_add, bad_monetary);
	      break;

	    default:
	    bad_monetary:
	      SYNTAX_ERROR (_("syntax error in monetary locale definition"));
	    }
	  continue;

	case 30:
	  HANDLE_COPY (LC_NUMERIC, tok_lc_numeric, "LC_NUMERIC");

	  numeric_startup (ldfile, result, charset);
	  /* FALLTHROUGH */

	case 31:
	  LOCALE_PROLOG (tok_lc_numeric, "LC_NUMERIC");

	  switch (nowtok)
	    {
	    case tok_decimal_point:
	    case tok_thousands_sep:
	      READ_STRING (numeric_add, bad_numeric);
	      break;

	    case tok_grouping:
	      /* We have a semicolon separated list of integers.  */
	      READ_NUMBER_LIST (numeric_add, bad_numeric);
	      break;

	    default:
	    bad_numeric:
	      SYNTAX_ERROR (_("syntax error in numeric locale definition"));
	    }
	  continue;

	case 40:
	  HANDLE_COPY (LC_TIME, tok_lc_time, "LC_TIME");

	  time_startup (ldfile, result, charset);
	  /* FALLTHROUGH */

	case 41:
	  LOCALE_PROLOG (tok_lc_time, "LC_TIME");

	  switch (nowtok)
	    {
	    case tok_abday:
	    case tok_day:
	    case tok_abmon:
	    case tok_mon:
	    case tok_am_pm:
	    case tok_alt_digits:
	    case tok_era:
	      READ_STRING_LIST (time_add, bad_time);
	      continue;

	    case tok_d_t_fmt:
	    case tok_d_fmt:
	    case tok_t_fmt:
	    case tok_t_fmt_ampm:
	    case tok_era_year:
	    case tok_era_d_t_fmt:
	    case tok_era_d_fmt:
	    case tok_era_t_fmt:
	      READ_STRING (time_add, bad_time);
	      break;

	    default:
	    bad_time:
	      SYNTAX_ERROR (_("syntax error in time locale definition"));
	    }
	  continue;

	case 50:
	  HANDLE_COPY (LC_MESSAGES, tok_lc_messages, "LC_MESSAGES");

	  messages_startup (ldfile, result, charset);
	  /* FALLTHROUGH */

	case 51:
	  LOCALE_PROLOG (tok_lc_messages, "LC_MESSAGES");

	  switch (nowtok)
	    {
	    case tok_yesexpr:
	    case tok_noexpr:
	    case tok_yesstr:
	    case tok_nostr:
	      READ_STRING (messages_add, bad_message);
	      break;

	    default:
	    bad_message:
	      SYNTAX_ERROR (_("syntax error in message locale definition"));
	    }
	  continue;

	default:
	  error (5, 0, _("%s: error in state machine"), __FILE__);
	  /* NOTREACHED */
	}

      break;
    }

  /* We read all of the file.  */
  lr_close (ldfile);

  /* Let's see what information is available.  */
  for (cnt = LC_CTYPE; cnt <= LC_MESSAGES; ++cnt)
    if (result->categories[cnt].generic != NULL)
      result->avail |= 1 << cnt;

  return result;
}


void
check_all_categories (struct localedef_t *locale, struct charset_t *charset)
{
 /* Call the finishing functions for all locales.  */
  if ((locale->avail & (1 << LC_CTYPE)) != 0
      && (locale->binary & (1 << LC_CTYPE)) == 0)
    ctype_finish (locale, charset);
  if ((locale->avail & (1 << LC_COLLATE)) != 0
      && (locale->binary & (1 << LC_COLLATE)) == 0)
    collate_finish (locale, charset);
  if ((locale->avail & (1 << LC_MONETARY)) != 0
      && (locale->binary & (1 << LC_MONETARY)) == 0)
    monetary_finish (locale);
  if ((locale->avail & (1 << LC_NUMERIC)) != 0
      && (locale->binary & (1 << LC_NUMERIC)) == 0)
    numeric_finish (locale);
  if ((locale->avail & (1 << LC_TIME)) != 0
      && (locale->binary & (1 << LC_TIME)) == 0)
    time_finish (locale);
  if ((locale->avail & (1 << LC_MESSAGES)) != 0
      && (locale->binary & (1 << LC_MESSAGES)) == 0)
    messages_finish (locale);
}


void
write_all_categories (struct localedef_t *locale, struct charset_t *charset,
		      const char *output_path)
{
  /* Call all functions to write locale data.  */
  if ((locale->avail & (1 << LC_CTYPE)) != 0)
    ctype_output (locale, charset, output_path);
  if ((locale->avail & (1 << LC_COLLATE)) != 0)
    collate_output (locale, charset, output_path);
  if ((locale->avail & (1 << LC_MONETARY)) != 0)
    monetary_output (locale, output_path);
  if ((locale->avail & (1 << LC_NUMERIC)) != 0)
    numeric_output (locale, output_path);
  if ((locale->avail & (1 << LC_TIME)) != 0)
    time_output (locale, output_path);
  if ((locale->avail & (1 << LC_MESSAGES)) != 0)
    messages_output (locale, output_path);
}


void
write_locale_data (const char *output_path, const char *category,
		   size_t n_elem, struct iovec *vec)
{
  size_t cnt, step, maxiov;
  int fd;
  char *fname;

  fname = malloc (strlen (output_path) + 2 * strlen (category) + 6);
  if (fname == NULL)
    error (5, errno, _("memory exhausted"));

  /* Normally we write to the directory pointed to by the OUTPUT_PATH.
     But for LC_MESSAGES we have to take care for the translation
     data.  This means we need to have a directory LC_MESSAGES in
     which we place the file under the name SYS_LC_MESSAGES.  */
  sprintf (fname, "%s%s", output_path, category);
  if (strcmp (category, "LC_MESSAGES") == 0)
    {
      struct stat st;

      if (stat (fname, &st) < 0)
	{
	  if (mkdir (fname, 0777) < 0)
	    fd = creat (fname, 0666);
	  else
	    {
	      fd = -1;
	      errno = EISDIR;
	    }
	}
      else if (S_ISREG (st.st_mode))
	fd = creat (fname, 0666);
      else
	{
	  fd = -1;
	  errno = EISDIR;
	}
    }
  else
    fd = creat (fname, 0666);

  if (fd == -1)
    {
      int save_err = errno;

      if (errno == EISDIR)
	{
	  sprintf (fname, "%1$s%2$s/SYS_%2$s", output_path, category);
	  fd = creat (fname, 0666);
	  if (fd == -1)
	    save_err = errno;
	}

      if (fd == -1)
	{
	  if (!be_quiet)
	    error (0, save_err, _("\
cannot open output file `%s' for category `%s'"),
		   fname, category);
	  return;
	}
    }
  free (fname);

#ifdef UIO_MAXIOV
  maxiov = UIO_MAXIOV;
#else
  maxiov = sysconf (_SC_UIO_MAXIOV);
#endif

  /* Write the data using writev.  But we must take care for the
     limitation of the implementation.  */
  for (cnt = 0; cnt < n_elem; cnt += step)
    {
      step = n_elem - cnt;
      if (maxiov > 0)
	step = MIN (maxiov, step);

      if (writev (fd, &vec[cnt], step) < 0)
	{
	  if (!be_quiet)
	    error (0, errno, _("failure while writing data for category `%s'"),
		   category);
	  break;
	}
    }

  close (fd);
}
