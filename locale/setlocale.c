/* Copyright (C) 1991, 1992, 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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

#include <alloca.h>
#include <argz.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "localeinfo.h"

/* For each category declare two external variables (with weak references):
     extern const struct locale_data *_nl_current_CATEGORY;
   This points to the current locale's in-core data for CATEGORY.
     extern const struct locale_data _nl_C_CATEGORY;
   This contains the built-in "C"/"POSIX" locale's data for CATEGORY.
   Both are weak references; if &_nl_current_CATEGORY is zero,
   then nothing is using the locale data.  */
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
extern const struct locale_data *_nl_current_##category;		      \
extern const struct locale_data _nl_C_##category;			      \
weak_symbol (_nl_current_##category) weak_symbol (_nl_C_##category)
#include "categories.def"
#undef	DEFINE_CATEGORY

/* Array indexed by category of pointers to _nl_current_CATEGORY slots.
   Elements are zero for categories whose data is never used.  */
static const struct locale_data * *const _nl_current[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
    [category] = &_nl_current_##category,
#include "categories.def"
#undef	DEFINE_CATEGORY
  };

/* Array indexed by category of pointers to _nl_C_CATEGORY slots.
   Elements are zero for categories whose data is never used.  */
const struct locale_data *const _nl_C[] =
{
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
  [category] = &_nl_C_##category,
#include "categories.def"
#undef	DEFINE_CATEGORY
};


/* Define an array of category names (also the environment variable names),
   indexed by integral category.  */
const char *const _nl_category_names[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
    [category] = category_name,
#include "categories.def"
#undef	DEFINE_CATEGORY
    [LC_ALL] = "LC_ALL"
  };
/* An array of their lengths, for convenience.  */
const size_t _nl_category_name_sizes[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
    [category] = sizeof (category_name) - 1,
#include "categories.def"
#undef	DEFINE_CATEGORY
    [LC_ALL] = sizeof ("LC_ALL") - 1
  };


/* Declare the postload functions used below.  */
#undef	NO_POSTLOAD
#define NO_POSTLOAD _nl_postload_ctype /* Harmless thing known to exist.  */
#define DEFINE_CATEGORY(category, category_name, items, postload, b, c, d) \
extern void postload (void);
#include "categories.def"
#undef	DEFINE_CATEGORY
#undef	NO_POSTLOAD

/* Define an array indexed by category of postload functions to call after
   loading and installing that category's data.  */
void (*const _nl_category_postload[]) (void) =
  {
#define DEFINE_CATEGORY(category, category_name, items, postload, b, c, d) \
    [category] = postload,
#include "categories.def"
#undef	DEFINE_CATEGORY
  };


/* Name of current locale for each individual category.
   Each is malloc'd unless it is nl_C_name.  */
static const char *_nl_current_names[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
    [category] = _nl_C_name,
#include "categories.def"
#undef	DEFINE_CATEGORY
    [LC_ALL] = _nl_C_name		/* For LC_ALL.  */
  };



/* Use this when we come along an error.  */
#define ERROR_RETURN							      \
  do {									      \
    errno = EINVAL;							      \
    return NULL;							      \
  } while (0)


static inline char *
clever_copy (const char *string)
{
  size_t len;
  char *new;

  if (strcmp (string, "C") == 0 || strcmp (string, "POSIX") == 0)
    /* This return is dangerous because the returned string might be
       placed in read-only memory.  But everything should be set up to
       handle this case.  */
    return (char *) _nl_C_name;

  len = strlen (string) + 1;
  new = (char *) malloc (len);
  return new != NULL ? memcpy (new, string, len) : NULL;
}


/* Construct a new composite name.  */
static inline char *
new_composite_name (int category, char *newnames[LC_ALL])
{
  size_t last_len;
  size_t cumlen = 0;
  int i;
  char *new, *p;
  int same = 1;

  for (i = 0; i < LC_ALL; ++i)
    {
      char *name = (category == LC_ALL ? newnames[i] :
		    category == i ? newnames[0] :
		    (char *) _nl_current_names[i]);
      last_len = strlen (name);
      cumlen += _nl_category_name_sizes[i] + 1 + last_len + 1;
      if (i > 0 && same && strcmp (name, newnames[0]) != 0)
	same = 0;
    }

  if (same)
    {
      /* All the categories use the same name.  */
      if (strcmp (newnames[0], "C") == 0 || strcmp (newnames[0], "POSIX") == 0)
	return (char *) _nl_C_name;

      new = malloc (last_len + 1);
      if (new == NULL)
	return NULL;

      memcpy (new, newnames[0], last_len + 1);
      return new;
    }

  new = malloc (cumlen);
  if (new == NULL)
    return NULL;
  p = new;
  for (i = 0; i < LC_ALL; ++i)
    {
      /* Add "CATEGORY=NAME;" to the string.  */
      char *name = (category == LC_ALL ? newnames[i] :
		    category == i ? newnames[0] :
		    (char *) _nl_current_names[i]);
      p = __stpcpy (p, _nl_category_names[i]);
      *p++ = '=';
      p = __stpcpy (p, name);
      *p++ = ';';
    }
  p[-1] = '\0';		/* Clobber the last ';'.  */
  return new;
}


/* Put NAME in _nl_current_names.  */
static inline void
setname (int category, const char *name)
{
  if (_nl_current[category] == NULL
      && _nl_current_names[category] != _nl_C_name)
    free ((void *) _nl_current_names[category]);

  _nl_current_names[category] = name;
}


/* Put DATA in *_nl_current[CATEGORY].  */
static inline void
setdata (int category, const struct locale_data *data)
{
  if (_nl_current[category] != NULL)
    {
      *_nl_current[category] = data;
      if (_nl_category_postload[category])
	(*_nl_category_postload[category]) ();
    }
}


char *
setlocale (int category, const char *locale)
{
  char *locpath_var;
  char *locale_path;
  size_t locale_path_len;
  char *composite;

  /* Sanity check for CATEGORY argument.  */
  if (category < 0 || category > LC_ALL)
    ERROR_RETURN;

  /* Does user want name of current locale?  */
  if (locale == NULL)
    return (char *) _nl_current_names[category];

  if (strcmp (locale, _nl_current_names[category]) == 0)
    /* Changing to the same thing.  */
    return (char *) _nl_current_names[category];

  /* We perhaps really have to load some data.  So we determine the
     path in which to look for the data now.  But this environment
     variable must only be used when the binary has no SUID or SGID
     bit set.  */
  locale_path = NULL;
  locale_path_len = 0;

  locpath_var = getenv ("LOCPATH");
  if (locpath_var != NULL && locpath_var[0] != '\0'
      && __getuid () == __geteuid () && __getgid () == __getegid ())
    if (__argz_create_sep (locpath_var, ':',
			   &locale_path, &locale_path_len) != 0)
      return NULL;

  if (__argz_append (&locale_path, &locale_path_len,
		     LOCALE_PATH, sizeof (LOCALE_PATH)) != 0)
    return NULL;

  if (category == LC_ALL)
    {
      /* The user wants to set all categories.  The desired locales
	 for the individual categories can be selected by using a
	 composite locale name.  This is a semi-colon separated list
	 of entries of the form `CATEGORY=VALUE'.  */
      char *newnames[LC_ALL];
      const struct locale_data *newdata[LC_ALL];

      /* Set all name pointers to the argument name.  */
      for (category = 0; category < LC_ALL; ++category)
	newnames[category] = (char *) locale;

      if (strchr (locale, ';') != NULL)
	{
	  /* This is a composite name.  Make a copy and split it up.  */
	  char *np = strdupa (locale);
	  char *cp;
	  int cnt;

	  while ((cp = strchr (np, '=')) != NULL)
	    {
	      for (cnt = 0; cnt < LC_ALL; ++cnt)
		if (cp - np == _nl_category_name_sizes[cnt]
		    && memcmp (np, _nl_category_names[cnt], cp - np) == 0)
		  break;

	      if (cnt == LC_ALL)
		/* Bogus category name.  */
		ERROR_RETURN;

	      /* Found the category this clause sets.  */
	      newnames[cnt] = ++cp;
	      cp = strchr (cp, ';');
	      if (cp != NULL)
		{
		  /* Examine the next clause.  */
		  *cp = '\0';
		  np = cp + 1;
		}
	      else
		/* This was the last clause.  We are done.  */
		break;
	    }

	  for (cnt = 0; cnt < LC_ALL; ++cnt)
	    if (newnames[cnt] == locale)
	      /* The composite name did not specify all categories.  */
	      ERROR_RETURN;
	}

      /* Load the new data for each category.  */
      while (category-- > 0)
	/* Only actually load the data if anything will use it.  */
	if (_nl_current[category] != NULL)
	  {
	    newdata[category] = _nl_find_locale (locale_path, locale_path_len,
						 category,
						 &newnames[category]);

	    if (newdata[category] == NULL)
	      {
		/* Loading this part of the locale failed.  Abort the
		   composite load.  */
		int save_errno;
	      abort_composite:
		save_errno = errno;

		while (++category < LC_ALL)
		  if (_nl_current[category] != NULL)
		    _nl_free_locale (newdata[category]);
		  else
		    if (_nl_current[category] == NULL
			&& newnames[category] != _nl_C_name)
		      free (newnames[category]);

		errno = save_errno;
		return NULL;
	      }
	  }
	else
	  {
	    /* The data is never used; just change the name.  */
	    newnames[category] = clever_copy (newnames[category]);
	    if (newnames[category] == NULL)
	      goto abort_composite;
	  }

      /* Create new composite name.  */
      composite = new_composite_name (LC_ALL, newnames);
      if (composite == NULL)
	{
	  category = -1;
	  goto abort_composite;
	}

      /* Now we have loaded all the new data.  Put it in place.  */
      for (category = 0; category < LC_ALL; ++category)
	{
	  setdata (category, newdata[category]);
	  setname (category, newnames[category]);
	}
      setname (LC_ALL, composite);

      return composite;
    }
  else
    {
      const struct locale_data *newdata;
      char *newname;

      if (_nl_current[category] != NULL)
	{
	  /* Only actually load the data if anything will use it.  */
	  newname = (char *) locale;
	  newdata = _nl_find_locale (locale_path, locale_path_len, category,
				     (char **) &newname);
	  if (newdata == NULL)
	    return NULL;
	}

      /* Create new composite name.  */
      composite = new_composite_name (category, &newname);
      if (composite == NULL)
	{
	  /* If anything went wrong free what we managed to allocate
	     so far.  */
	  int save_errno = errno;

	  if (_nl_current[category] != NULL)
	    _nl_free_locale (newdata);

	  errno = save_errno;
	  return NULL;
	}

      if (_nl_current[category] != NULL)
	setdata (category, newdata);

      setname (category, newname);
      setname (LC_ALL, composite);

      return newname;
    }
}
