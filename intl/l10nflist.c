/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

This file is part of the GNU C Library.  Its master source is NOT part of
the C library, however.  The master source lives in /gd/gnu/lib.

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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#if defined _LIBC && (defined __ARGZ_COUNT || defined __ARGZ_STRINGIFY)
# include <argz.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "loadinfo.h"

/* Define function which are usually not available.  */

#if !defined _LIBC && !defined __ARGZ_COUNT
/* Returns the number of strings in ARGZ.  */
static size_t __argz_count __P ((const char *argz, size_t len));

static size_t
__argz_count (argz, len)
     const char *argz;
     size_t len;
{
  size_t count = 0;
  while (len > 0)
    {
      size_t part_len = strlen(argz);
      argz += part_len + 1;
      len -= part_len + 1;
      count++;
    }
  return count;
}
#endif	/* !_LIBC && !__ARGZ_COUNT */

#if !defined _LIBC && !defined __ARGZ_STRINGIFY
/* Make '\0' separated arg vector ARGZ printable by converting all the '\0's
   except the last into the character SEP.  */
static void __argz_stringify __P ((char *argz, size_t len, int sep));

static void
__argz_stringify (argz, len, sep)
     char *argz;
     size_t len;
     int sep;
{
  while (len > 0)
    {
      size_t part_len = strlen(argz);
      argz += part_len;
      len -= part_len + 1;
      if (len > 0)
	*argz++ = sep;
    }
}
#endif	/* !_LIBC && !__ARGZ_COUNT */

#if !defined _LIBC && !defined __ARGZ_NEXT
static char *
__argz_next (char *argz, size_t argz_len, const char *entry)
{
  if (entry)
    {
      if (entry < argz + argz_len)
        entry = strchr (entry, '\0') + 1;

      return entry >= argz + argz_len ? NULL : (char *) entry;
    }
  else
    if (argz_len > 0)
      return argz;
    else
      return 0;
}
#endif


/* Return number of bits set in X.  */
static int pop __P ((int x));

static inline int
pop (x)
     int x;
{
  /* We assume that no more than 16 bits are used.  */
  x = ((x & ~0x5555) >> 1) + (x & 0x5555);
  x = ((x & ~0x3333) >> 2) + (x & 0x3333);
  x = ((x >> 4) + x) & 0x0f0f;
  x = ((x >> 8) + x) & 0xff;

  return x;
}


struct loaded_l10nfile *
_nl_make_l10nflist (l10nfile_list, dirlist, dirlist_len, mask, language,
		    territory, codeset, normalized_codeset, modifier, special,
		    sponsor, revision, filename, do_allocate)
     struct loaded_l10nfile **l10nfile_list;
     const char *dirlist;
     size_t dirlist_len;
     int mask;
     const char *language;
     const char *territory;
     const char *codeset;
     const char *normalized_codeset;
     const char *modifier;
     const char *special;
     const char *sponsor;
     const char *revision;
     const char *filename;
     int do_allocate;
{
  char *abs_filename;
  struct loaded_l10nfile *last = NULL;
  struct loaded_l10nfile *retval;
  char *cp;
  size_t entries;
  int cnt;

  /* Allocate room for the full file name.  */
  abs_filename = (char *) malloc (dirlist_len
				  + strlen (language)
				  + ((mask & TERRITORY) != 0
				     ? strlen (territory) + 1 : 0)
				  + ((mask & XPG_CODESET) != 0
				     ? strlen (codeset) + 1 : 0)
				  + ((mask & XPG_NORM_CODESET) != 0
				     ? strlen (normalized_codeset) + 1 : 0)
				  + (((mask & XPG_MODIFIER) != 0
				      || (mask & CEN_AUDIENCE) != 0) ?
				     strlen (modifier) + 1 : 0)
				  + ((mask & CEN_SPECIAL) != 0
				     ? strlen (special) + 1 : 0)
				  + ((mask & CEN_SPONSOR) != 0
				     ? strlen (sponsor) + 1 : 0)
				  + ((mask & CEN_REVISION) != 0
				     ? strlen (revision) + 1 : 0)
				  + 1 + strlen (filename) + 1);

  if (abs_filename == NULL)
    return NULL;

  retval = NULL;
  last = NULL;

  /* Construct file name.  */
  memcpy (abs_filename, dirlist, dirlist_len);
  __argz_stringify (abs_filename, dirlist_len, ':');
  cp = abs_filename + (dirlist_len - 1);
  *cp++ = '/';
  cp = stpcpy (cp, language);

  if ((mask & TERRITORY) != 0)
    {
      *cp++ = '_';
      cp = stpcpy (cp, territory);
    }
  if ((mask & XPG_CODESET) != 0)
    {
      *cp++ = '.';
      cp = stpcpy (cp, codeset);
    }
  if ((mask & XPG_NORM_CODESET) != 0)
    {
      *cp++ = '.';
      cp = stpcpy (cp, normalized_codeset);
    }
  if ((mask & (XPG_MODIFIER | CEN_AUDIENCE)) != 0)
    {
      /* This component can be part of both syntaces but has different
	 leading characters.  For CEN we use `+', else `@'.  */
      *cp++ = (mask & CEN_AUDIENCE) != 0 ? '+' : '@';
      cp = stpcpy (cp, modifier);
    }
  if ((mask & CEN_SPECIAL) != 0)
    {
      *cp++ = '+';
      cp = stpcpy (cp, special);
    }
  if ((mask & CEN_SPONSOR) != 0)
    {
      *cp++ = ',';
      cp = stpcpy (cp, sponsor);
    }
  if ((mask & CEN_REVISION) != 0)
    {
      *cp++ = '_';
      cp = stpcpy (cp, revision);
    }

  *cp++ = '/';
  stpcpy (cp, filename);

  /* Look in list of already loaded domains whether it is already
     available.  */
  last = NULL;
  for (retval = *l10nfile_list; retval != NULL; retval = retval->next)
    if (retval->filename != NULL)
      {
	int compare = strcmp (retval->filename, abs_filename);
	if (compare == 0)
	  /* We found it!  */
	  break;
	if (compare < 0)
	  {
	    /* It's not in the list.  */
	    retval = NULL;
	    break;
	  }

	last = retval;
      }

  if (retval != NULL || do_allocate == 0)
    {
      free (abs_filename);
      return retval;
    }

  retval = (struct loaded_l10nfile *)
    malloc (sizeof (*retval) + (__argz_count (dirlist, dirlist_len)
				* (1 << pop (mask))
				* sizeof (struct loaded_l10nfile *)));
  if (retval == NULL)
    return NULL;

  retval->filename = abs_filename;
  retval->decided = (__argz_count (dirlist, dirlist_len) != 1
		     || ((mask & XPG_CODESET) != 0
			 && (mask & XPG_NORM_CODESET) != 0));
  retval->data = NULL;

  if (last == NULL)
    {
      retval->next = *l10nfile_list;
      *l10nfile_list = retval;
    }
  else
    {
      retval->next = last->next;
      last->next = retval;
    }

  entries = 0;
  /* If the DIRLIST is a real list the RETVAL entry correcponds not to
     a real file.  So we have to use the DIRLIST separation machanism
     of the inner loop.  */
  cnt = __argz_count (dirlist, dirlist_len) == 1 ? mask - 1 : mask;
  for (; cnt >= 0; --cnt)
    if ((cnt & ~mask) == 0
	&& ((cnt & CEN_SPECIFIC) == 0 || (cnt & XPG_SPECIFIC) == 0)
	&& ((cnt & XPG_CODESET) == 0 || (cnt & XPG_NORM_CODESET) == 0))
      {
	/* Iterate over all elements of the DIRLIST.  */
	char *dir = NULL;

	while ((dir = __argz_next ((char *) dirlist, dirlist_len, dir))
	       != NULL)
	  retval->successor[entries++]
	    = _nl_make_l10nflist (l10nfile_list, dir, strlen (dir) + 1, cnt,
				  language, territory, codeset,
				  normalized_codeset, modifier, special,
				  sponsor, revision, filename, 1);
      }
  retval->successor[entries] = NULL;

  return retval;
}

/* Normalize codeset name.  There is no standard for the codeset
   names.  Normalization allows the user to use any of the common
   names.  */
const char *
_nl_normalize_codeset (codeset, name_len)
     const char *codeset;
     size_t name_len;
{
  int len = 0;
  int only_digit = 1;
  char *retval;
  char *wp;
  size_t cnt;

  for (cnt = 0; cnt < name_len; ++cnt)
    if (isalnum (codeset[cnt]))
      {
	++len;

	if (isalpha (codeset[cnt]))
	  only_digit = 0;
      }

  retval = (char *) malloc ((only_digit ? 3 : 0) + len + 1);

  if (retval != NULL)
    {
      if (only_digit)
	wp = stpcpy (retval, "ISO");
      else
	wp = retval;

      for (cnt = 0; cnt < name_len; ++cnt)
	if (isalpha (codeset[cnt]))
	  *wp++ = tolower (codeset[cnt]);
	else if (isdigit (codeset[cnt]))
	  *wp++ = codeset[cnt];

      *wp = '\0';
    }

  return (const char *) retval;
}
