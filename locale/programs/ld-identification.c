/* Copyright (C) 1998, 1999 Free Software Foundation, Inc.
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

#include <error.h>
#include <langinfo.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#include <assert.h>

#include "localeinfo.h"
#include "locfile.h"


/* The real definition of the struct for the LC_IDENTIFICATION locale.  */
struct locale_identification_t
{
  const char *title;
  const char *source;
  const char *address;
  const char *contact;
  const char *email;
  const char *tel;
  const char *fax;
  const char *language;
  const char *territory;
  const char *audience;
  const char *application;
  const char *abbreviation;
  const char *revision;
  const char *date;
  const char *category[__LC_LAST];
};


static void
identification_startup (struct linereader *lr, struct localedef_t *locale,
			int ignore_content)
{
  if (!ignore_content)
    {
      locale->categories[LC_IDENTIFICATION].identification =
	(struct locale_identification_t *)
	xcalloc (1, sizeof (struct locale_identification_t));

      locale->categories[LC_IDENTIFICATION].identification->category[LC_ALL] =
	"";
    }

  lr->translate_strings = 1;
  lr->return_widestr = 0;
}


void
identification_finish (struct localedef_t *locale, struct charmap_t *charmap)
{
  struct locale_identification_t *identification
    = locale->categories[LC_IDENTIFICATION].identification;

#define TEST_ELEM(cat) \
  if (identification->cat == NULL)					      \
    {									      \
      if (verbose)							      \
	error (0, 0, _("%s: field `%s' not defined"),			      \
	       "LC_IDENTIFICATION", #cat);				      \
      identification->cat = "";						      \
    }

  TEST_ELEM (title);
  TEST_ELEM (source);
  TEST_ELEM (address);
  TEST_ELEM (contact);
  TEST_ELEM (email);
  TEST_ELEM (tel);
  TEST_ELEM (fax);
  TEST_ELEM (language);
  TEST_ELEM (territory);
  TEST_ELEM (audience);
  TEST_ELEM (application);
  TEST_ELEM (abbreviation);
  TEST_ELEM (revision);
  TEST_ELEM (date);
}


void
identification_output (struct localedef_t *locale, struct charmap_t *charmap,
		       const char *output_path)
{
  struct locale_identification_t *identification
    = locale->categories[LC_IDENTIFICATION].identification;
  struct iovec iov[2 + _NL_ITEM_INDEX (_NL_NUM_LC_IDENTIFICATION)
		  + (__LC_LAST - 1)];
  struct locale_file data;
  uint32_t idx[_NL_ITEM_INDEX (_NL_NUM_LC_IDENTIFICATION)];
  size_t cnt = 0;
  size_t num;

  data.magic = LIMAGIC (LC_IDENTIFICATION);
  data.n = _NL_ITEM_INDEX (_NL_NUM_LC_IDENTIFICATION);
  iov[cnt].iov_base = (void *) &data;
  iov[cnt].iov_len = sizeof (data);
  ++cnt;

  iov[cnt].iov_base = (void *) idx;
  iov[cnt].iov_len = sizeof (idx);
  ++cnt;

  idx[cnt - 2] = iov[0].iov_len + iov[1].iov_len;
  iov[cnt].iov_base = (void *) identification->title;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->source;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->address;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->contact;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->email;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->tel;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->fax;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->language;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->territory;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->audience;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->application;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->abbreviation;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->revision;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) identification->date;
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  for (num = 0; num < __LC_LAST; ++num)
    {
      iov[cnt].iov_base = (void *) identification->category[num];
      iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
      ++cnt;
    }

  assert (cnt == (2 + _NL_ITEM_INDEX (_NL_NUM_LC_IDENTIFICATION)
		  + (__LC_LAST - 1)));

  write_locale_data (output_path, "LC_IDENTIFICATION",
		     2 + _NL_ITEM_INDEX (_NL_NUM_LC_IDENTIFICATION), iov);
}


/* The parser for the LC_IDENTIFICATION section of the locale definition.  */
void
identification_read (struct linereader *ldfile, struct localedef_t *result,
	       struct charmap_t *charmap, const char *repertoire_name,
	       int ignore_content)
{
  struct repertoire_t *repertoire = NULL;
  struct locale_identification_t *identification;
  struct token *now;
  struct token *arg;
  struct token *cattok;
  int category;
  enum token_t nowtok;

  /* Get the repertoire we have to use.  */
  if (repertoire_name != NULL)
    repertoire = repertoire_read (repertoire_name);

  /* The rest of the line containing `LC_IDENTIFICATION' must be free.  */
  lr_ignore_rest (ldfile, 1);

  do
    {
      now = lr_token (ldfile, charmap, NULL);
      nowtok = now->tok;
    }
  while (nowtok == tok_eol);

  /* If we see `copy' now we are almost done.  */
  if (nowtok == tok_copy)
    {
      handle_copy (ldfile, charmap, repertoire, tok_lc_identification,
		   LC_IDENTIFICATION, "LC_IDENTIFICATION", ignore_content);
      return;
    }

  /* Prepare the data structures.  */
  identification_startup (ldfile, result, ignore_content);
  identification = result->categories[LC_IDENTIFICATION].identification;

  while (1)
    {
      /* Of course we don't proceed beyond the end of file.  */
      if (nowtok == tok_eof)
	break;

      /* Ingore empty lines.  */
      if (nowtok == tok_eol)
	{
	  now = lr_token (ldfile, charmap, NULL);
	  nowtok = now->tok;
	  continue;
	}

      switch (nowtok)
	{
#define STR_ELEM(cat) \
	case tok_##cat:							      \
	  arg = lr_token (ldfile, charmap, NULL);			      \
	  if (arg->tok != tok_string)					      \
	    goto err_label;						      \
	  if (identification->cat != NULL)				      \
	    lr_error (ldfile, _("\
%s: field `%s' declared more than once"), "LC_IDENTIFICATION", #cat);	      \
	  else if (!ignore_content && arg->val.str.startmb == NULL)	      \
	    {								      \
	      lr_error (ldfile, _("\
%s: unknown character in field `%s'"), "LC_IDENTIFICATION", #cat);	      \
	      identification->cat = "";					      \
	    }								      \
	  else if (!ignore_content)					      \
	    identification->cat = arg->val.str.startmb;			      \
	  break

	  STR_ELEM (title);
	  STR_ELEM (source);
	  STR_ELEM (address);
	  STR_ELEM (contact);
	  STR_ELEM (email);
	  STR_ELEM (tel);
	  STR_ELEM (fax);
	  STR_ELEM (language);
	  STR_ELEM (territory);
	  STR_ELEM (audience);
	  STR_ELEM (application);
	  STR_ELEM (abbreviation);
	  STR_ELEM (revision);
	  STR_ELEM (date);

	case tok_category:
	  /* We expect two operands.  */
	  arg = lr_token (ldfile, charmap, NULL);
	  if (arg->tok != tok_string && arg->tok != tok_ident)
	    goto err_label;
	  /* Next is a semicolon.  */
	  cattok = lr_token (ldfile, charmap, NULL);
	  if (cattok->tok != tok_semicolon)
	    goto err_label;
	  /* Now a LC_xxx identifier.  */
	  cattok = lr_token (ldfile, charmap, NULL);
	  switch (cattok->tok)
	    {
#define CATEGORY(lname, uname) \
	    case tok_lc_##lname:					      \
	      category = LC_##uname;					      \
	      break

	      CATEGORY (identification, IDENTIFICATION);
	      CATEGORY (ctype, CTYPE);
	      CATEGORY (collate, COLLATE);
	      CATEGORY (time, TIME);
	      CATEGORY (numeric, NUMERIC);
	      CATEGORY (monetary, MONETARY);
	      CATEGORY (messages, MESSAGES);
	      CATEGORY (paper, PAPER);
	      CATEGORY (name, NAME);
	      CATEGORY (address, ADDRESS);
	      CATEGORY (telephone, TELEPHONE);
	      CATEGORY (measurement, MEASUREMENT);

	    default:
	      goto err_label;
	    }
	  if (identification->category[category] != NULL)
	    {
	      lr_error (ldfile, _("\
%s: duplicate category version definition"), "LC_IDENTIFICATION");
	      free (arg->val.str.startmb);
	    }
	  else
	    identification->category[category] = arg->val.str.startmb;
	  break;

	case tok_end:
	  /* Next we assume `LC_IDENTIFICATION'.  */
	  arg = lr_token (ldfile, charmap, NULL);
	  if (arg->tok == tok_eof)
	    break;
	  if (arg->tok == tok_eol)
	    lr_error (ldfile, _("%s: incomplete `END' line"),
		      "LC_IDENTIFICATION");
	  else if (arg->tok != tok_lc_identification)
	    lr_error (ldfile, _("\
%1$s: definition does not end with `END %1$s'"), "LC_IDENTIFICATION");
	  lr_ignore_rest (ldfile, arg->tok == tok_lc_identification);
	  return;

	default:
	err_label:
	  SYNTAX_ERROR (_("%s: syntax error"), "LC_IDENTIFICATION");
	}

      /* Prepare for the next round.  */
      now = lr_token (ldfile, charmap, NULL);
      nowtok = now->tok;
    }

  /* When we come here we reached the end of the file.  */
  lr_error (ldfile, _("%s: premature end of file"), "LC_IDENTIFICATION");
}
