/* Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "localeinfo.h"


/* Constant data defined in setlocale.c.  */
extern const struct locale_data *const _nl_C[];


static inline char *
copy (const char *string)
{
  size_t len;
  char *new;
  len = strlen (string) + 1;
  new = (char *) malloc (len);
  return new != NULL ? memcpy (new, string, len) : NULL;
}


/* For each category we keep a list of records for the locale files
   which are somehow addressed.  */
static struct loaded_l10nfile *locale_file_list[LC_ALL];


const struct locale_data *
_nl_find_locale (const char *locale_path, size_t locale_path_len,
		 int category, char **name)
{
  int mask;
  /* Name of the locale for this category.  */
  char *loc_name;
  const char *language;
  const char *modifier;
  const char *territory;
  const char *codeset;
  const char *normalized_codeset;
  const char *special;
  const char *sponsor;
  const char *revision;
  struct loaded_l10nfile *locale_file;

  if ((*name)[0] == '\0')
    {
      /* The user decides which locale to use by setting environment
	 variables.  */
      *name = getenv ("LC_ALL");
      if (*name == NULL || (*name)[0] == '\0')
	*name = getenv (_nl_category_names[category]);
      if (*name == NULL || (*name)[0] == '\0')
	*name = getenv ("LANG");
      if (*name == NULL || (*name)[0] == '\0')
	*name = (char *) _nl_C_name;
    }

  if (strcmp (*name, _nl_C_name) == 0 || strcmp (*name, "POSIX") == 0)
    {
      /* We need not load anything.  The needed data is contained in
	 the library itself.  */
      *name = (char *) _nl_C_name;
      return _nl_C[category];
    }

  /* We really have to load some data.  First see whether the name is
     an alias.  Please note that this makes it impossible to have "C"
     or "POSIX" as aliases.  */
  loc_name = _nl_expand_alias (*name);
  if (loc_name == NULL)
    /* It is no alias.  */
    loc_name = *name;

  /* Make a writable copy of the locale name.  */
  loc_name = copy (loc_name);

  /* LOCALE can consist of up to four recognized parts for the XPG syntax:

		language[_territory[.codeset]][@modifier]

     and six parts for the CEN syntax:

	language[_territory][+audience][+special][,sponsor][_revision]

     Beside the first all of them are allowed to be missing.  If the
     full specified locale is not found, the less specific one are
     looked for.  The various part will be stripped of according to
     the following order:
		(1) revision
		(2) sponsor
		(3) special
		(4) codeset
		(5) normalized codeset
		(6) territory
		(7) audience/modifier
   */
  mask = _nl_explode_name (loc_name, &language, &modifier, &territory,
			   &codeset, &normalized_codeset, &special,
			   &sponsor, &revision);

  /* If exactly this locale was already asked for we have an entry with
     the complete name.  */
  locale_file = _nl_make_l10nflist (&locale_file_list[category],
				    locale_path, locale_path_len, mask,
				    language, territory, codeset,
				    normalized_codeset, modifier, special,
				    sponsor, revision,
				    _nl_category_names[category], 0);

  if (locale_file == NULL)
    {
      /* Find status record for addressed locale file.  We have to search
	 through all directories in the locale path.  */
      locale_file = _nl_make_l10nflist (&locale_file_list[category],
					locale_path, locale_path_len, mask,
					language, territory, codeset,
					normalized_codeset, modifier, special,
					sponsor, revision,
					_nl_category_names[category], 1);
      if (locale_file == NULL)
	/* This means we are out of core.  */
	return NULL;
    }
  else
    /* If the addressed locale is already available it should be freed.
       If we would not do this switching back and force between two
       locales would slowly eat up all memory.*/
    free (loc_name);

  if (locale_file->decided == 0)
    _nl_load_locale (locale_file, category);

  if (locale_file->data == NULL)
    {
      int cnt;
      for (cnt = 0; locale_file->successor[cnt] != NULL; ++cnt)
	{
	  if (locale_file->successor[cnt]->decided == 0)
	    _nl_load_locale (locale_file->successor[cnt], category);
	  if (locale_file->successor[cnt]->data != NULL)
	    break;
	}
      /* Move the entry we found (or NULL) to the first place of
	 successors.  */
      locale_file->successor[0] = locale_file->successor[cnt];
      locale_file = locale_file->successor[cnt];
    }

  if (locale_file == NULL)
    return NULL;

  /* Determine the locale name for which loading succeeded.  This
     information comes from the file name.  The form is
     <path>/<locale>/LC_foo.  We must extract this <locale> part.  */
  if (((struct locale_data *) locale_file->data)->name == NULL)
    {
      char *newp, *cp, *endp;

      endp = strrchr (locale_file->filename, '/');
      cp = endp - 1;
      while (cp[-1] != '/')
	--cp;
      newp = (char *) malloc (endp - cp + 1);
      if (newp == NULL)
	return NULL;
      memcpy (newp, cp, endp - cp);
      newp[endp - cp] = '\0';
      ((struct locale_data *) locale_file->data)->name = newp;
    }
  *name = (char *) ((struct locale_data *) locale_file->data)->name;

  return (struct locale_data *) locale_file->data;
}
