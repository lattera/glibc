/* Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 1996.

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
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "localedef.h"
#include "locfile.h"

#include "locfile-kw.h"


int
locfile_read (struct localedef_t *result, struct charmap_t *charmap)
{
  const char *filename = result->name;
  const char *repertoire_name = result->repertoire_name;
  int locale_mask = result->needed ^ result->avail;
  struct linereader *ldfile;
  int not_here = ALL_LOCALES;

  /* If no repertoire name was specified use the global one.  */
  if (repertoire_name == NULL)
    repertoire_name = repertoire_global;

  /* Open the locale definition file.  */
  ldfile = lr_open (filename, locfile_hash);
  if (ldfile == NULL)
    {
      if (filename != NULL && filename[0] != '/')
	{
	  char *i18npath = getenv ("I18NPATH");
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

		  if (ldfile == NULL)
		    {
		      stpcpy (stpcpy (path, next), filename);

		      ldfile = lr_open (path, locfile_hash);
		    }
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
	return 1;
    }

    /* Parse locale definition file and store result in RESULT.  */
  while (1)
    {
      struct token *now = lr_token (ldfile, charmap, NULL, verbose);
      enum token_t nowtok = now->tok;
      struct token *arg;

      if (nowtok == tok_eof)
	break;

      if (nowtok == tok_eol)
	/* Ignore empty lines.  */
	continue;

      switch (nowtok)
	{
	case tok_escape_char:
	case tok_comment_char:
	  /* We need an argument.  */
	  arg = lr_token (ldfile, charmap, NULL, verbose);

	  if (arg->tok != tok_ident)
	    {
	      SYNTAX_ERROR (_("bad argument"));
	      continue;
	    }

	  if (arg->val.str.lenmb != 1)
	    {
	      lr_error (ldfile, _("\
argument to `%s' must be a single character"),
			nowtok == tok_escape_char
			? "escape_char" : "comment_char");

	      lr_ignore_rest (ldfile, 0);
	      continue;
	    }

	  if (nowtok == tok_escape_char)
	    ldfile->escape_char = *arg->val.str.startmb;
	  else
	    ldfile->comment_char = *arg->val.str.startmb;
	  break;

	case tok_repertoiremap:
	  /* We need an argument.  */
	  arg = lr_token (ldfile, charmap, NULL, verbose);

	  if (arg->tok != tok_ident)
	    {
	      SYNTAX_ERROR (_("bad argument"));
	      continue;
	    }

	  if (repertoire_name == NULL)
	    {
	      repertoire_name = memcpy (xmalloc (arg->val.str.lenmb + 1),
					arg->val.str.startmb,
					arg->val.str.lenmb);
	      ((char *) repertoire_name)[arg->val.str.lenmb] = '\0';
	    }
	  break;

	case tok_lc_ctype:
	  ctype_read (ldfile, result, charmap, repertoire_name,
		      (locale_mask & CTYPE_LOCALE) == 0);
	  result->avail |= locale_mask & CTYPE_LOCALE;
	  not_here ^= CTYPE_LOCALE;
	  continue;

	case tok_lc_collate:
	  collate_read (ldfile, result, charmap, repertoire_name,
			(locale_mask & COLLATE_LOCALE) == 0);
	  result->avail |= locale_mask & COLLATE_LOCALE;
	  not_here ^= COLLATE_LOCALE;
	  continue;

	case tok_lc_monetary:
	  monetary_read (ldfile, result, charmap, repertoire_name,
			 (locale_mask & MONETARY_LOCALE) == 0);
	  result->avail |= locale_mask & MONETARY_LOCALE;
	  not_here ^= MONETARY_LOCALE;
	  continue;

	case tok_lc_numeric:
	  numeric_read (ldfile, result, charmap, repertoire_name,
			(locale_mask & NUMERIC_LOCALE) == 0);
	  result->avail |= locale_mask & NUMERIC_LOCALE;
	  not_here ^= NUMERIC_LOCALE;
	  continue;

	case tok_lc_time:
	  time_read (ldfile, result, charmap, repertoire_name,
		     (locale_mask & TIME_LOCALE) == 0);
	  result->avail |= locale_mask & TIME_LOCALE;
	  not_here ^= TIME_LOCALE;
	  continue;

	case tok_lc_messages:
	  messages_read (ldfile, result, charmap, repertoire_name,
			 (locale_mask & MESSAGES_LOCALE) == 0);
	  result->avail |= locale_mask & MESSAGES_LOCALE;
	  not_here ^= MESSAGES_LOCALE;
	  continue;

	case tok_lc_paper:
	  paper_read (ldfile, result, charmap, repertoire_name,
		      (locale_mask & PAPER_LOCALE) == 0);
	  result->avail |= locale_mask & PAPER_LOCALE;
	  not_here ^= PAPER_LOCALE;
	  continue;

	case tok_lc_name:
	  name_read (ldfile, result, charmap, repertoire_name,
		     (locale_mask & NAME_LOCALE) == 0);
	  result->avail |= locale_mask & NAME_LOCALE;
	  not_here ^= NAME_LOCALE;
	  continue;

	case tok_lc_address:
	  address_read (ldfile, result, charmap, repertoire_name,
			(locale_mask & ADDRESS_LOCALE) == 0);
	  result->avail |= locale_mask & ADDRESS_LOCALE;
	  not_here ^= ADDRESS_LOCALE;
	  continue;

	case tok_lc_telephone:
	  telephone_read (ldfile, result, charmap, repertoire_name,
			  (locale_mask & TELEPHONE_LOCALE) == 0);
	  result->avail |= locale_mask & TELEPHONE_LOCALE;
	  not_here ^= TELEPHONE_LOCALE;
	  continue;

	case tok_lc_measurement:
	  measurement_read (ldfile, result, charmap, repertoire_name,
			    (locale_mask & MEASUREMENT_LOCALE) == 0);
	  result->avail |= locale_mask & MEASUREMENT_LOCALE;
	  not_here ^= MEASUREMENT_LOCALE;
	  continue;

	case tok_lc_identification:
	  identification_read (ldfile, result, charmap, repertoire_name,
			       (locale_mask & IDENTIFICATION_LOCALE) == 0);
	  result->avail |= locale_mask & IDENTIFICATION_LOCALE;
	  not_here ^= IDENTIFICATION_LOCALE;
	  continue;

	default:
	  SYNTAX_ERROR (_("\
syntax error: not inside a locale definition section"));
	  continue;
	}

      /* The rest of the line must be empty.  */
      lr_ignore_rest (ldfile, 1);
    }

  /* We read all of the file.  */
  lr_close (ldfile);

  /* Mark the categories which are not contained in the file.  We assume
     them to be available and the default data will be used.  */
  result->avail |= not_here;

  return 0;
}


static void (*const check_funcs[]) (struct localedef_t *,
				    struct charmap_t *) =
{
  [LC_CTYPE] = ctype_finish,
  [LC_COLLATE] = collate_finish,
  [LC_MESSAGES] = messages_finish,
  [LC_MONETARY] = monetary_finish,
  [LC_NUMERIC] = numeric_finish,
  [LC_TIME] = time_finish,
  [LC_PAPER] = paper_finish,
  [LC_NAME] = name_finish,
  [LC_ADDRESS] = address_finish,
  [LC_TELEPHONE] = telephone_finish,
  [LC_MEASUREMENT] = measurement_finish,
  [LC_IDENTIFICATION] = identification_finish
};


void
check_all_categories (struct localedef_t *definitions,
		      struct charmap_t *charmap)
{
  int cnt;

  for (cnt = 0; cnt < sizeof (check_funcs) / sizeof (check_funcs[0]); ++cnt)
    if (check_funcs[cnt] != NULL)
      check_funcs[cnt] (definitions, charmap);
}


static void (*const write_funcs[]) (struct localedef_t *, struct charmap_t *,
				    const char *) =
{
  [LC_CTYPE] = ctype_output,
  [LC_COLLATE] = collate_output,
  [LC_MESSAGES] = messages_output,
  [LC_MONETARY] = monetary_output,
  [LC_NUMERIC] = numeric_output,
  [LC_TIME] = time_output,
  [LC_PAPER] = paper_output,
  [LC_NAME] = name_output,
  [LC_ADDRESS] = address_output,
  [LC_TELEPHONE] = telephone_output,
  [LC_MEASUREMENT] = measurement_output,
  [LC_IDENTIFICATION] = identification_output
};


void
write_all_categories (struct localedef_t *definitions,
		      struct charmap_t *charmap,
		      const char *output_path)
{
  int cnt;

  for (cnt = 0; cnt < sizeof (write_funcs) / sizeof (write_funcs[0]); ++cnt)
    if (write_funcs[cnt] != NULL)
      write_funcs[cnt] (definitions, charmap, output_path);
}


void
write_locale_data (const char *output_path, const char *category,
		   size_t n_elem, struct iovec *vec)
{
  size_t cnt, step, maxiov;
  int fd;
  char *fname;

  fname = malloc (strlen (output_path) + 2 * strlen (category) + 7);
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
	  sprintf (fname, "%1$s/%2$s/SYS_%2$s", output_path, category);
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
