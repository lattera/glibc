/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#include <alloca.h>
#include <langinfo.h>
#include <string.h>
#include <sys/uio.h>

#ifdef HAVE_REGEX
# include <regex.h>
#else
# include <rx.h>
#endif

/* Undefine following line in production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include "locales.h"
#include "stringtrans.h"
#include "localeinfo.h"


extern void *xmalloc (size_t __n);


/* The real definition of the struct for the LC_MESSAGES locale.  */
struct locale_messages_t
{
  const char *yesexpr;
  const char *noexpr;
  const char *yesstr;
  const char *nostr;
};


void
messages_startup (struct linereader *lr, struct localedef_t *locale,
		  struct charset_t *charset)
{
  struct locale_messages_t *messages;

  /* We have a definition for LC_MESSAGES.  */
  copy_posix.mask &= ~(1 << LC_MESSAGES);

  /* It is important that we always use UCS1 encoding for strings now.  */
  encoding_method = ENC_UCS1;

  locale->categories[LC_MESSAGES].messages = messages =
    (struct locale_messages_t *) xmalloc (sizeof (struct locale_messages_t));

  memset (messages, '\0', sizeof (struct locale_messages_t));
}


void
messages_finish (struct localedef_t *locale)
{
  struct locale_messages_t *messages
    = locale->categories[LC_MESSAGES].messages;

  /* The fields YESSTR and NOSTR are optional.  */
  if (messages->yesexpr == NULL)
    {
      if (!be_quiet)
	error (0, 0, _("field `%s' in category `%s' undefined"),
	       "yesexpr", "LC_MESSAGES");
    }
  else
    {
      int result;
      regex_t re;

      /* Test whether it are correct regular expressions.  */
      result = regcomp (&re, messages->yesexpr, REG_EXTENDED);
      if (result != 0 && !be_quiet)
	{
	  char errbuf[BUFSIZ];

	  (void) regerror (result, &re, errbuf, BUFSIZ);
	  error (0, 0, _("\
no correct regular expression for field `%s' in category `%s': %s"),
		 "yesexpr", "LC_MESSAGES", errbuf);
	}
    }

  if (messages->noexpr == NULL)
    {
      if (!be_quiet)
	error (0, 0, _("field `%s' in category `%s' undefined"),
	       "noexpr", "LC_MESSAGES");
    }
  else
    {
      int result;
      regex_t re;

      /* Test whether it are correct regular expressions.  */
      result = regcomp (&re, messages->noexpr, REG_EXTENDED);
      if (result != 0 && !be_quiet)
	{
	  char errbuf[BUFSIZ];

	  (void) regerror (result, &re, errbuf, BUFSIZ);
	  error (0, 0, _("\
no correct regular expression for field `%s' in category `%s': %s"),
		 "noexpr", "LC_MESSAGES", errbuf);
	}
    }
}


void
messages_output (struct localedef_t *locale, const char *output_path)
{
  struct locale_messages_t *messages
    = locale->categories[LC_MESSAGES].messages;
  struct iovec iov[2 + _NL_ITEM_INDEX (_NL_NUM_LC_MESSAGES)];
  struct locale_file data;
  u_int32_t idx[_NL_ITEM_INDEX (_NL_NUM_LC_MESSAGES)];
  size_t cnt = 0;

  if ((locale->binary & (1 << LC_MESSAGES)) != 0)
    {
      iov[0].iov_base = messages;
      iov[0].iov_len = locale->len[LC_MESSAGES];

      write_locale_data (output_path, "LC_MESSAGES", 1, iov);

      return;
    }

  data.magic = LIMAGIC (LC_MESSAGES);
  data.n = _NL_ITEM_INDEX (_NL_NUM_LC_MESSAGES);
  iov[cnt].iov_base = (void *) &data;
  iov[cnt].iov_len = sizeof (data);
  ++cnt;

  iov[cnt].iov_base = (void *) idx;
  iov[cnt].iov_len = sizeof (idx);
  ++cnt;

  idx[cnt - 2] = iov[0].iov_len + iov[1].iov_len;
  iov[cnt].iov_base = (void *) (messages->yesexpr ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (messages->noexpr ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (messages->yesstr ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;
  ++cnt;

  idx[cnt - 2] = idx[cnt - 3] + iov[cnt - 1].iov_len;
  iov[cnt].iov_base = (void *) (messages->nostr ?: "");
  iov[cnt].iov_len = strlen (iov[cnt].iov_base) + 1;

  assert (cnt + 1 == 2 + _NL_ITEM_INDEX (_NL_NUM_LC_MESSAGES));

  write_locale_data (output_path, "LC_MESSAGES",
		     2 + _NL_ITEM_INDEX (_NL_NUM_LC_MESSAGES), iov);
}


void
messages_add (struct linereader *lr, struct localedef_t *locale,
	      enum token_t tok, struct token *code,
	      struct charset_t *charset)
{
  struct locale_messages_t *messages
    = locale->categories[LC_MESSAGES].messages;

  switch (tok)
    {
    case tok_yesexpr:
      if (code->val.str.start == NULL)
	{
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),
		    "yesexpr", "LC_MESSAGES");
	  messages->yesexpr = "";
	}
      else
	messages->yesexpr = code->val.str.start;
      break;

    case tok_noexpr:
      if (code->val.str.start == NULL)
	{
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),
		    "noexpr", "LC_MESSAGES");
	  messages->noexpr = "";
	}
      else
	messages->noexpr = code->val.str.start;
      break;

    case tok_yesstr:
      if (code->val.str.start == NULL)
	{
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),
		    "yesstr", "LC_MESSAGES");
	  messages->yesstr = "";
	}
      else
	messages->yesstr = code->val.str.start;
      break;

    case tok_nostr:
      if (code->val.str.start == NULL)
	{
	  lr_error (lr, _("unknown character in field `%s' of category `%s'"),
		    "nostr", "LC_MESSAGES");
	  messages->nostr = "";
	}
      else
	messages->nostr = code->val.str.start;
      break;

    default:
      assert (! "unknown token in category `LC_MESSAGES': should not happen");
    }
}
