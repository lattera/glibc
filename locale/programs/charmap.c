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
#include <dirent.h>
#include <errno.h>
#include <libintl.h>
#include <obstack.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "linereader.h"
#include "charset.h"
#include "locfile.h"
#include "repertoire.h"


/* Uncomment following line for production version.  */
/* define NDEBUG 1 */
#include <assert.h>


/* Define the lookup function.  */
#include "charmap-kw.h"


extern void *xmalloc (size_t __n);

/* Prototypes for local functions.  */
static struct charset_t *parse_charmap (const char *filename);
static void new_width (struct linereader *cmfile, struct charset_t *result,
		       const char *from, const char *to,
		       unsigned long int width);


struct charset_t *
charmap_read (const char *filename)
{
  const char *pathnfile;
  struct charset_t *result = NULL;

  if (filename != NULL)
    {
      if (euidaccess (filename, R_OK) >= 0)
	pathnfile = filename;
      else if (filename[0] != '/')
	{
	  char *cp = xmalloc (strlen (filename) + sizeof CHARMAP_PATH + 1);
	  stpcpy (stpcpy (stpcpy (cp, CHARMAP_PATH), "/"), filename);

	  pathnfile = (const char *) cp;
	}
      else
	pathnfile = NULL;

      if (pathnfile != NULL)
	{
	  result = parse_charmap (pathnfile);

	  if (result == NULL && !be_quiet)
	    error (0, errno, _("character map file `%s' not found"), filename);
	}
    }

  if (result == NULL)
    {
      /* OK, one more try.  We also accept the names given to the
	 character sets in the files.  Sometimes they differ from the
	 file name.  */
      DIR *dir;
      struct dirent *dirent;

      dir = opendir (CHARMAP_PATH);
      if (dir == NULL)
	{
	  while ((dirent = readdir (dir)) != NULL)
	    if (strcmp (dirent->d_name, ".") != 0
		&& strcmp (dirent->d_name, "..") != 0)
	      {
		char buf[sizeof (CHARMAP_PATH)
			+ strlen (dirent->d_name) + 1];
		FILE *fp;
#ifdef _DIRENT_HAVE_D_TYPE
		if (dirent->d_type != DT_UNKNOWN && dirent->d_type != DT_REG)
		  continue;
#endif
		stpcpy (stpcpy (stpcpy (buf, CHARMAP_PATH), "/"),
			dirent->d_name);

		fp = fopen (buf, "r");
		if (fp != NULL)
		  {
		    char *name = NULL;

		    while (!feof (fp))
		      {
			char junk[BUFSIZ];

			if (fscanf (fp, " <code_set_name> %as", &name) == 1
			    || fscanf (fp, "%% alias %as", &name) == 1)
			  {
			    if (strcasecmp (name, filename) == 0)
			      break;

			    free (name);
			    name = NULL;
			  }

			if (fgets (junk, sizeof junk, fp) != NULL)
			  {
			    if (strstr (junk, "CHARMAP") != NULL)
			      /* We cannot expect more aliases from now on.  */
			      break;

			    while (strchr (junk, '\n') == NULL
				   && fgets (junk, sizeof junk, fp) != NULL)
			      continue;
			  }
		      }

		    fclose (fp);

		    if (name != NULL)
		      {
			result = parse_charmap (buf);

			free (buf);

			if (result)
			  return result;

			break;
		      }
		  }
	      }

	  closedir (dir);
	}
    }

  if (result == NULL)
    {
      pathnfile = CHARMAP_PATH "/" DEFAULT_CHARMAP;

      result = parse_charmap (pathnfile);

      if (result == NULL)
	error (4, errno, _("default character map file `%s' not found"),
	       DEFAULT_CHARMAP);
    }

  return result;
}


static struct charset_t *
parse_charmap (const char *filename)
{
  struct linereader *cmfile;
  struct charset_t *result;
  int state;
  enum token_t expected_tok = tok_error;
  const char *expected_str = NULL;
  char *from_name = NULL;
  char *to_name = NULL;

  /* Determine path.  */
  cmfile = lr_open (filename, charmap_hash);
  if (cmfile == NULL)
    {
      if (strchr (filename, '/') == NULL)
	{
	  /* Look in the systems charmap directory.  */
	  char *buf = xmalloc (strlen (filename) + 1 + sizeof (CHARMAP_PATH));

	  stpcpy (stpcpy (stpcpy (buf, CHARMAP_PATH), "/"), filename);
	  cmfile = lr_open (buf, charmap_hash);

	  if (cmfile == NULL)
	    free (buf);
	}

      if (cmfile == NULL)
	return NULL;
    }

  /* Allocate room for result.  */
  result = (struct charset_t *) xmalloc (sizeof (struct charset_t));
  memset (result, '\0', sizeof (struct charset_t));
  /* The default DEFAULT_WIDTH is 1.  */
  result->width_default = 1;
  /* Let the user overwrite the repertoire map we use.  */
  result->repertoiremap = repertoiremap;

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
      struct token *now = lr_token (cmfile, NULL);
      enum token_t nowtok = now->tok;
      struct token *arg;

      if (nowtok == tok_eof)
	break;

      switch (state)
	{
	case 1:
	  /* The beginning.  We expect the special declarations, EOL or
	     `CHARMAP'.  */
	  if (nowtok == tok_eol)
	    /* Ignore empty lines.  */
	    continue;

	  if (nowtok == tok_charmap)
	    {
	      from_name = NULL;
	      to_name = NULL;

	      /* We have to set up the real work.  Fill in some
		 default values.  */
	      if (result->mb_cur_max == 0)
		result->mb_cur_max = 1;
	      if (result->mb_cur_min == 0)
		result->mb_cur_min = result->mb_cur_max;
	      if (result->mb_cur_min > result->mb_cur_max)
		{
		  if (!be_quiet)
		    error (0, 0, _("\
%s: <mb_cur_max> must be greater than <mb_cur_min>\n"),
			   cmfile->fname);

		  result->mb_cur_min = result->mb_cur_max;
		}

	      lr_ignore_rest (cmfile, 1);

	      /* Read the repertoire map now.  */
	      if (result->repertoiremap == NULL)
		/* This is fatal.  */
		error (4, 0, _("no repertoire map specified: cannot proceed"));

	      result->repertoire = repertoire_read (result->repertoiremap);
	      if (result->repertoire == NULL)
		/* This is also fatal.  */
		error (4, errno, _("cannot read repertoire map `%s'"),
		       result->repertoiremap);

	      state = 2;
	      continue;
	    }

	  if (nowtok != tok_code_set_name && nowtok != tok_mb_cur_max
	      && nowtok != tok_mb_cur_min && nowtok != tok_escape_char
	      && nowtok != tok_comment_char && nowtok != tok_g0esc
	      && nowtok != tok_g1esc && nowtok != tok_g2esc
	      && nowtok != tok_g3esc && nowtok != tok_repertoiremap)
	    {
	      lr_error (cmfile, _("syntax error in prolog: %s"),
			_("illegal definition"));

	      lr_ignore_rest (cmfile, 0);
	      continue;
	    }

	  /* We know that we need an argument.  */
	  arg = lr_token (cmfile, NULL);

	  switch (nowtok)
	    {
	    case tok_code_set_name:
	      if (arg->tok != tok_ident)
		{
		badarg:
		  lr_error (cmfile, _("syntax error in prolog: %s"),
			    _("bad argument"));

		  lr_ignore_rest (cmfile, 0);
		  continue;
		}

	      result->code_set_name = obstack_copy0 (&result->mem_pool,
						     arg->val.str.start,
						     arg->val.str.len);

	      lr_ignore_rest (cmfile, 1);
	      continue;

	    case tok_repertoiremap:
	      if (arg->tok != tok_ident)
		goto badarg;

	      if (result->repertoiremap == NULL)
		result->repertoiremap = obstack_copy0 (&result->mem_pool,
						       arg->val.str.start,
						       arg->val.str.len);

	      lr_ignore_rest (cmfile, 1);
	      continue;

	    case tok_mb_cur_max:
	    case tok_mb_cur_min:
	      if (arg->tok != tok_number)
		goto badarg;

	      if (arg->val.num < 1 || arg->val.num > 4)
		{
		  lr_error (cmfile,
			    _("value for <%s> must lie between 1 and 4"),
			    nowtok == tok_mb_cur_min ? "mb_cur_min"
						     : "mb_cur_max");

		  lr_ignore_rest (cmfile, 0);
		  continue;
		}
	      if ((nowtok == tok_mb_cur_max && result->mb_cur_min != 0
		   && (int) arg->val.num < result->mb_cur_min)
		  || (nowtok == tok_mb_cur_min && result->mb_cur_max != 0
		      && (int) arg->val.num > result->mb_cur_max))
		{
		  lr_error (cmfile, _("\
value of <mb_cur_max> must be greater than the value of <mb_cur_min>"));

		  lr_ignore_rest (cmfile, 0);
		  continue;
		}

	      if (nowtok == tok_mb_cur_max)
		result->mb_cur_max = arg->val.num;
	      else
		result->mb_cur_min = arg->val.num;

	      lr_ignore_rest (cmfile, 1);
	      continue;

	    case tok_escape_char:
	    case tok_comment_char:
	      if (arg->tok != tok_ident)
		goto badarg;

	      if (arg->val.str.len != 1)
		{
		  lr_error (cmfile, _("\
argument to <%s> must be a single character"),
			    nowtok == tok_escape_char ? "escape_char"
						      : "comment_char");

		  lr_ignore_rest (cmfile, 0);
		  continue;
		}

	      if (nowtok == tok_escape_char)
		cmfile->escape_char = *arg->val.str.start;
	      else
		cmfile->comment_char = *arg->val.str.start;

	      lr_ignore_rest (cmfile, 1);
	      continue;

	    case tok_g0esc:
	    case tok_g1esc:
	    case tok_g2esc:
	    case tok_g3esc:
	      lr_ignore_rest (cmfile, 0); /* XXX */
	      continue;

	    default:
	      /* Cannot happen.  */
	      assert (! "Should not happen");
	    }
	  break;

	case 2:
	  /* We have seen `CHARMAP' and now are in the body.  Each line
	     must have the format "%s %s %s\n" or "%s...%s %s %s\n".  */
	  if (nowtok == tok_eol)
	    /* Ignore empty lines.  */
	    continue;

	  if (nowtok == tok_end)
	    {
	      expected_tok = tok_charmap;
	      expected_str = "CHARMAP";
	      state = 90;
	      continue;
	    }

	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (cmfile, _("syntax error in %s definition: %s"),
			"CHARMAP", _("no symbolic name given"));

	      lr_ignore_rest (cmfile, 0);
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
	  if (nowtok != tok_charcode && nowtok != tok_ucs2
	      && nowtok != tok_ucs4)
	    {
	      lr_error (cmfile, _("syntax error in %s definition: %s"),
			"CHARMAP", _("illegal encoding given"));

	      lr_ignore_rest (cmfile, 0);

	      state = 2;
	      continue;
	    }

	  if (now->val.charcode.nbytes < result->mb_cur_min)
	    lr_error (cmfile, _("too few bytes in character encoding"));
	  else if (now->val.charcode.nbytes > result->mb_cur_max)
	    lr_error (cmfile, _("too many bytes in character encoding"));
	  else
	    charset_new_char (cmfile, &result->char_table,
			      now->val.charcode.nbytes,
			      now->val.charcode.val, from_name, to_name);

	  /* Ignore trailing comment silently.  */
	  lr_ignore_rest (cmfile, 0);

	  from_name = NULL;
	  to_name = NULL;

	  state = 2;
	  continue;

	case 4:
	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (cmfile, _("syntax error in %s definition: %s"),
			"CHARMAP",
			_("no symbolic name given for end of range"));

	      lr_ignore_rest (cmfile, 0);
	      continue;
	    }

	  /* Copy the to-name in a safe place.  */
	  to_name = (char *) obstack_copy0 (&result->mem_pool,
					    cmfile->token.val.str.start,
					    cmfile->token.val.str.len);

	  state = 5;
	  continue;

	case 90:
	  if (nowtok != expected_tok)
	    lr_error (cmfile, _("\
`%1$s' definition does not end with `END %1$s'"), expected_str);

	  lr_ignore_rest (cmfile, nowtok == expected_tok);
	  state = 91;
	  continue;

	case 91:
	  /* Waiting for WIDTH... */
	  if (nowtok == tok_eol)
	    /* Ignore empty lines.  */
	    continue;

	  if (nowtok == tok_width_default)
	    {
	      state = 92;
	      continue;
	    }

	  if (nowtok == tok_width)
	    {
	      lr_ignore_rest (cmfile, 1);
	      state = 93;
	      continue;
	    }

	  if (nowtok == tok_width_variable)
	    {
	      lr_ignore_rest (cmfile, 1);
	      state = 98;
	      continue;
	    }

	  lr_error (cmfile, _("\
only WIDTH definitions are allowed to follow the CHARMAP definition"));

	  lr_ignore_rest (cmfile, 0);
	  continue;

	case 92:
	  if (nowtok != tok_number)
	    lr_error (cmfile, _("value for %s must be an integer"),
		      "WIDTH_DEFAULT");
	  else
	    result->width_default = now->val.num;

	  lr_ignore_rest (cmfile, nowtok == tok_number);

	  state = 91;
	  continue;

	case 93:
	  /* We now expect `END WIDTH' or lines of the format "%s %d\n" or
	     "%s...%s %d\n".  */
	  if (nowtok == tok_eol)
	    /* ignore empty lines.  */
	    continue;

	  if (nowtok == tok_end)
	    {
	      expected_tok = tok_width;
	      expected_str = "WIDTH";
	      state = 90;
	      continue;
	    }

	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (cmfile, _("syntax error in %s definition: %s"),
			"WIDTH", _("no symbolic name given"));

	      lr_ignore_rest (cmfile, 0);
	      continue;
	    }

	  if (from_name != NULL)
	    obstack_free (&result->mem_pool, from_name);

	  from_name = (char *) obstack_copy0 (&result->mem_pool,
					      now->val.str.start,
					      now->val.str.len);
	  to_name = NULL;

	  state = 94;
	  continue;

	case 94:
	  if (nowtok == tok_ellipsis)
	    {
	      state = 95;
	      continue;
	    }

	case 96:
	  if (nowtok != tok_number)
	    lr_error (cmfile, _("value for %s must be an integer"),
		      "WIDTH");
	  else
	    {
	      /* Store width for chars.  */
	      new_width (cmfile, result, from_name, to_name, now->val.num);

	      from_name = NULL;
	      to_name = NULL;
	    }

	  lr_ignore_rest (cmfile, nowtok == tok_number);

	  state = 93;
	  continue;

	case 95:
	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (cmfile, _("syntax error in %s definition: %s"),
			"WIDTH", _("no symbolic name given for end of range"));

	      lr_ignore_rest (cmfile, 0);

	      state = 93;
	      continue;
	    }

	  to_name = (char *) obstack_copy0 (&result->mem_pool,
					    now->val.str.start,
					    now->val.str.len);

	  state = 96;
	  continue;

	case 98:
	  /* We now expect `END WIDTH_VARIABLE' or lines of the format
	     "%s\n" or "%s...%s\n".  */
	  if (nowtok == tok_eol)
	    /* ignore empty lines.  */
	    continue;

	  if (nowtok == tok_end)
	    {
	      expected_tok = tok_width_variable;
	      expected_str = "WIDTH_VARIABLE";
	      state = 90;
	      continue;
	    }

	  if (nowtok != tok_bsymbol)
	    {
	      lr_error (cmfile, _("syntax error in %s definition: %s"),
			"WIDTH_VARIABLE", _("no symbolic name given"));

	      lr_ignore_rest (cmfile, 0);

	      continue;
	    }

	  if (from_name != NULL)
	    obstack_free (&result->mem_pool, from_name);

	  from_name = (char *) obstack_copy0 (&result->mem_pool,
					      now->val.str.start,
					      now->val.str.len);
	  to_name = NULL;

	  state = 99;
	  continue;

	case 99:
	  if (nowtok == tok_ellipsis)
	    state = 100;

	  /* Store info.  */
	  from_name = NULL;

	  /* Warn */
	  state = 98;
	  continue;

	case 100:
	  if (nowtok != tok_bsymbol)
	    lr_error (cmfile, _("syntax error in %s definition: %s"),
		      "WIDTH_VARIABLE",
		      _("no symbolic name given for end of range"));
	  else
	    {
	      to_name = (char *) obstack_copy0 (&result->mem_pool,
						now->val.str.start,
						now->val.str.len);
	      /* XXX Enter value into table.  */
	    }

	  lr_ignore_rest (cmfile, nowtok == tok_bsymbol);

	  state = 98;
	  continue;

	default:
	  error (5, 0, _("%s: error in state machine"), __FILE__);
	  /* NOTREACHED */
	}
      break;
    }

  if (state != 91 && !be_quiet)
    error (0, 0, _("%s: premature end of file"), cmfile->fname);

  lr_close (cmfile);

  return result;
}


static void
new_width (struct linereader *cmfile, struct charset_t *result,
	   const char *from, const char *to, unsigned long int width)
{
  unsigned int from_val, to_val;

  from_val = charset_find_value (&result->char_table, from, strlen (from));
  if ((wchar_t) from_val == ILLEGAL_CHAR_VALUE)
    {
      lr_error (cmfile, _("unknown character `%s'"), from);
      return;
    }

  if (to == NULL)
    to_val = from_val;
  else
    {
      to_val = charset_find_value (&result->char_table, to, strlen (to));
      if ((wchar_t) to_val == ILLEGAL_CHAR_VALUE)
	{
	  lr_error (cmfile, _("unknown character `%s'"), to);
	  return;
	}
    }

  if (result->nwidth_rules >= result->nwidth_rules_max)
    {
      size_t new_size = result->nwidth_rules + 32;
      struct width_rule *new_rules =
	(struct width_rule *) obstack_alloc (&result->mem_pool,
					     (new_size
					      * sizeof (struct width_rule)));

      memcpy (new_rules, result->width_rules,
	      result->nwidth_rules_max * sizeof (struct width_rule));

      result->width_rules = new_rules;
      result->nwidth_rules_max = new_size;
    }

  result->width_rules[result->nwidth_rules].from = from_val;
  result->width_rules[result->nwidth_rules].to = to_val;
  result->width_rules[result->nwidth_rules].width = (unsigned int) width;
  ++result->nwidth_rules;
}
