/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
#include <error.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linereader.h"
#include "charset.h"
#include "repertoire.h"
#include "simple-hash.h"


extern void *xmalloc (size_t __n);


/* Simple keyword hashing for the repertoiremap.  */
static const struct keyword_t *repertoiremap_hash (const char *str, int len);


struct repertoire_t *
repertoire_read (const char *filename)
{
  struct linereader *repfile;
  struct repertoire_t *result;
  int state;
  char *from_name = NULL;
  char *to_name = NULL;

  /* Determine path.  */
  repfile = lr_open (filename, repertoiremap_hash);
  if (repfile == NULL)
    {
      if (strchr (filename, '/') == NULL)
	{
	  char *i18npath = __secure_getenv ("I18NPATH");
	  if (i18npath != NULL && *i18npath != '\0')
	    {
	      char path[strlen (filename) + 1 + strlen (i18npath)
		        + sizeof ("/repertoiremaps/") - 1];
	      char *next;
	      i18npath = strdupa (i18npath);


	      while (repfile == NULL
		     && (next = strsep (&i18npath, ":")) != NULL)
		{
		  stpcpy (stpcpy (stpcpy (path, next), "/repertoiremaps/"),
			  filename);

		  repfile = lr_open (path, repertoiremap_hash);
		}
	    }

	  if (repfile == NULL)
	    {
	      /* Look in the systems charmap directory.  */
	      char *buf = xmalloc (strlen (filename) + 1
				   + sizeof (REPERTOIREMAP_PATH));

	      stpcpy (stpcpy (stpcpy (buf, REPERTOIREMAP_PATH), "/"),
		      filename);
	      repfile = lr_open (buf, repertoiremap_hash);

	      if (repfile == NULL)
		free (buf);
	    }
	}

      if (repfile == NULL)
	{
	  error (0, errno, _("repertoire map file `%s' not found"), filename);
	  return NULL;
	}
    }

  /* Allocate room for result.  */
  result = (struct repertoire_t *) xmalloc (sizeof (struct repertoire_t));
  memset (result, '\0', sizeof (struct repertoire_t));

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
  obstack_init (&result->mem_pool);

  if (init_hash (&result->char_table, 256))
    {
      free (result);
      return NULL;
    }

  /* We use a state machine to describe the charmap description file
     format.  */
  state = 1;
  while (1)
    {
      /* What's on?  */
      struct token *now = lr_token (repfile, NULL);
      enum token_t nowtok = now->tok;
      struct token *arg;

      if (nowtok == tok_eof)
	break;

      switch (state)
	{
	case 1:
	  /* We haven't yet read any character definition.  This is where
	     we accept escape_char and comment_char definitions.  */
	  if (nowtok == tok_eol)
	    /* Ignore empty lines.  */
	    continue;

	  if (nowtok == tok_escape_char || nowtok == tok_comment_char)
	    {
	      /* We know that we need an argument.  */
	      arg = lr_token (repfile, NULL);

	      if (arg->tok != tok_ident)
		{
		  lr_error (repfile, _("syntax error in prolog: %s"),
			    _("bad argument"));

		  lr_ignore_rest (repfile, 0);
		  continue;
		}

	      if (arg->val.str.len != 1)
		{
		  lr_error (repfile, _("\
argument to <%s> must be a single character"),
			    nowtok == tok_escape_char ? "escape_char"
						      : "comment_char");

		  lr_ignore_rest (repfile, 0);
		  continue;
		}

	      if (nowtok == tok_escape_char)
		repfile->escape_char = *arg->val.str.start;
	      else
		repfile->comment_char = *arg->val.str.start;

	      lr_ignore_rest (repfile, 1);
	      continue;
	    }

	  if (nowtok == tok_charids)
	    {
	      lr_ignore_rest (repfile, 1);

	      state = 2;
	      continue;
	    }

	  /* Otherwise we start reading the character definitions.  */
	  state = 2;
	  /* FALLTHROUGH */

	case 2:
	  /* We are now are in the body.  Each line
	     must have the format "%s %s %s\n" or "%s...%s %s %s\n".  */
	  if (nowtok == tok_eol)
	    /* Ignore empty lines.  */
	    continue;

	  if (nowtok == tok_end)
	    {
	      state = 90;
	      continue;
	    }

	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (repfile,
			_("syntax error in repertoire map definition: %s"),
			_("no symbolic name given"));

	      lr_ignore_rest (repfile, 0);
	      continue;
	    }

	  /* If the previous line was not completely correct free the
	     used memory.  */
	  if (from_name != NULL)
	    obstack_free (&result->mem_pool, from_name);

	  from_name = (char *) obstack_copy0 (&result->mem_pool,
					      now->val.str.start,
					      now->val.str.len);
	  to_name = NULL;

	  state = 3;
	  continue;

	case 3:
	  /* We have two possibilities: We can see an ellipsis or an
	     encoding value.  */
	  if (nowtok == tok_ellipsis)
	    {
	      state = 4;
	      continue;
	    }
	  /* FALLTHROUGH */

	case 5:
	  /* We expect a value of the form <Uxxxx> or <Uxxxxxxxx> where
	     the xxx mean a hexadecimal value.  */
	  state = 2;

	  errno = 0;
	  if (nowtok != tok_ucs2 && nowtok != tok_ucs4)
	    {
	      lr_error (repfile,
			_("syntax error in repertoire map definition: %s"),
			_("no <Uxxxx> or <Uxxxxxxxx> value given"));

	      lr_ignore_rest (repfile, 0);
	      continue;
	    }

	  /* We've found a new valid definition.  */
	  charset_new_char (repfile, &result->char_table, 4,
			    now->val.charcode.val, from_name, to_name);

	  /* Ignore the rest of the line.  */
	  lr_ignore_rest (repfile, 0);

	  from_name = NULL;
	  to_name = NULL;

	  continue;

	case 4:
	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (repfile,
			_("syntax error in repertoire map definition: %s"),
			_("no symbolic name given for end of range"));

	      lr_ignore_rest (repfile, 0);
	      state = 2;
	      continue;
	    }

	  /* Copy the to-name in a safe place.  */
	  to_name = (char *) obstack_copy0 (&result->mem_pool,
					    repfile->token.val.str.start,
					    repfile->token.val.str.len);

	  state = 5;
	  continue;

	case 90:
	  if (nowtok != tok_charids)
	    lr_error (repfile, _("\
`%1$s' definition does not end with `END %1$s'"), "CHARIDS");

	  lr_ignore_rest (repfile, nowtok == tok_charids);
	  break;
	}

      break;
    }

  if (state != 2 && state != 90 && !be_quiet)
    error (0, 0, _("%s: premature end of file"), repfile->fname);

  lr_close (repfile);

  return result;
}


static const struct keyword_t *
repertoiremap_hash (const char *str, int len)
{
  static const struct keyword_t wordlist[0] =
  {
    {"escape_char",      tok_escape_char,     0},
    {"comment_char",     tok_comment_char,    0},
    {"CHARIDS",          tok_charids,         0},
    {"END",              tok_end,             0},
  };

  if (len == 11 && memcmp (wordlist[0].name, str, 11) == 0)
    return &wordlist[0];
  if (len == 12 && memcmp (wordlist[1].name, str, 12) == 0)
    return &wordlist[1];
  if (len == 7 && memcmp (wordlist[2].name, str, 7) == 0)
    return &wordlist[2];
  if (len == 3 && memcmp (wordlist[3].name, str, 3) == 0)
    return &wordlist[3];

  return NULL;
}
