/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <langinfo.h>
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
const struct locale_data * *const _nl_current[] =
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
  };
/* An array of their lengths, for convenience.  */
const size_t _nl_category_name_sizes[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
    [category] = sizeof (category_name) - 1,
#include "categories.def"
#undef	DEFINE_CATEGORY
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


const char _nl_C_name[] = "C";

/* Name of current locale for each individual category.
   Each is malloc'd unless it is nl_C_name.  */
const char *_nl_current_names[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
    _nl_C_name,
#include "categories.def"
#undef	DEFINE_CATEGORY
  };

/* Composite LC_ALL name for current locale.
   This is malloc'd unless it's _nl_C_name.  */
char *_nl_current_composite_name = (char *) _nl_C_name;


/* Switch to the locale called NAME in CATEGORY.  Return a string
   describing the locale.  This string can be used as the NAME argument in
   a later call.  If NAME is NULL, don't switch locales, but return the
   current one.  If NAME is "", switch to a locale based on the environment
   variables, as per POSIX.  Return NULL on error.  */

char *
setlocale (int category, const char *name)
{
  /* Return a malloc'd copy of STRING.  */
  char *copy (const char *string)
    {
      size_t len = strlen (string) + 1;
      char *new = malloc (len);
      return new ? memcpy (new, string, len) : NULL;
    }

  /* Construct a new composite name.  */
  char *new_composite_name (int category, char *newnames[LC_ALL])
  {
    size_t lens[LC_ALL], cumlen = 0;
    int i;
    char *new, *p;
    int same = 1;

    for (i = 0; i < LC_ALL; ++i)
      {
	char *name = (category == LC_ALL ? newnames[i] :
		      category == i ? newnames[0] :
		      (char *) _nl_current_names[i]);
	lens[i] = strlen (name);
	cumlen += _nl_category_name_sizes[i] + 1 + lens[i] + 1;
	if (i > 0 && same && strcmp (name, newnames[0]))
	  same = 0;
      }

    if (same)
      {
	/* All the categories use the same name.  */
	new = malloc (lens[0] + 1);
	if (! new)
	  {
	    if (!strcmp (newnames[0], "C") || !strcmp (newnames[0], "POSIX"))
	      return (char *) _nl_C_name;
	    return NULL;
	  }
	memcpy (new, newnames[0], lens[0] + 1);
	return new;
      }

    new = malloc (cumlen);
    if (! new)
      return NULL;
    p = new;
    for (i = 0; i < LC_ALL; ++i)
      {
	/* Add "CATEGORY=NAME;" to the string.  */
	char *name = (category == LC_ALL ? newnames[i] :
		      category == i ? newnames[0] :
		      (char *) _nl_current_names[i]);
	memcpy (p, _nl_category_names[i], _nl_category_name_sizes[i]);
	p += _nl_category_name_sizes[i];
	*p++ = '=';
	memcpy (p, name, lens[i]);
	p += lens[i];
	*p++ = ';';
      }
    p[-1] = '\0';		/* Clobber the last ';'.  */
    return new;
  }
  /* Put COMPOSITE in _nl_current_composite_name and free the old value.  */
  void setcomposite (char *composite)
    {
      char *old = _nl_current_composite_name;
      _nl_current_composite_name = composite;
      if (old != _nl_C_name)
	free (old);
    }
  /* Put NAME in _nl_current_names and free the old value.  */
  void setname (int category, const char *name)
    {
      const char *oldname = _nl_current_names[category];
      _nl_current_names[category] = name;
      if (oldname != _nl_C_name)
	free ((char *) oldname);
    }
  /* Put DATA in *_nl_current[CATEGORY] and free the old value.  */
  void setdata (int category, struct locale_data *data)
    {
      if (_nl_current[category])
	{
	  const struct locale_data *olddata = *_nl_current[category];
	  *_nl_current[category] = data;
	  if (_nl_category_postload[category])
	    (*_nl_category_postload[category]) ();
	  if (olddata != _nl_C[category])
	    _nl_free_locale ((struct locale_data *) olddata);
	}
    }

  const char *current_name;
  char *composite;

  if (category < 0 || category > LC_ALL)
    {
      errno = EINVAL;
      return NULL;
    }

  if (category == LC_ALL)
    current_name = _nl_current_composite_name;
  else
    current_name = _nl_current_names[category];

  if (name == NULL)
    /* Return the name of the current locale.  */
    return (char *) current_name;

  if (name == current_name)
    /* Changing to the same thing.  */
    return (char *) current_name;

  if (category == LC_ALL)
    {
      const size_t len = strlen (name) + 1;
      char *newnames[LC_ALL];
      char *p;
      struct locale_data *newdata[LC_ALL];

      /* Set all name pointers to the argument name.  */
      for (category = 0; category < LC_ALL; ++category)
	newnames[category] = (char *) name;

      p = strchr (name, ';');
      if (p)
	{
	  /* This is a composite name.  Make a local copy and split it up.  */
	  int i;
	  char *n = alloca (len);
	  memcpy (n, name, len);

	  while ((p = strchr (n, '=')) != NULL)
	    {
	      for (i = 0; i < LC_ALL; ++i)
		if (_nl_category_name_sizes[i] == p - n &&
		    !memcmp (_nl_category_names[i], n, p - n))
		  break;
	      if (i == LC_ALL)
		{
		  /* Bogus category name.  */
		  errno = EINVAL;
		  return NULL;
		}
	      if (i < LC_ALL)
		{
		  /* Found the category this clause sets.  */
		  char *end = strchr (++p, ';');
		  newnames[i] = p;
		  if (end)
		    {
		      /* Examine the next clause.  */
		      *end = '\0';
		      n = end + 1;
		    }
		  else
		    /* This was the last clause.  We are done.  */
		    break;
		}
	    }

	  for (i = 0; i < LC_ALL; ++i)
	    if (newnames[i] == name)
	      /* The composite name did not specify all categories.  */
	      return NULL;
	}
	
      /* Load the new data for each category.  */
      while (category-- > 0)
	/* Only actually load the data if anything will use it.  */
	if (_nl_current[category])
	  {
	    newdata[category] = _nl_load_locale (category,
						 &newnames[category]);
	    if (newdata[category])
	      newnames[category] = copy (newnames[category]);
	    if (! newdata[category] || ! newnames[category])
	      {
		if (!strcmp (newnames[category], "C") ||
		    !strcmp (newnames[category], "POSIX"))
		  {
		    /* Loading from a file failed, but this is a request
		       for the default locale.  Use the built-in data.  */
		    if (! newdata[category])
		      newdata[category]
			= (struct locale_data *) _nl_C[category];
		    newnames[category] = (char *) _nl_C_name;
		  }
		else
		  {
		    /* Loading this part of the locale failed.
		       Abort the composite load.  */
		  abort_composite:
		    while (++category < LC_ALL)
		      {
			if (_nl_current[category])
			  _nl_free_locale (newdata[category]);
			if (newnames[category] != _nl_C_name)
			  free (newnames[category]);
		      }
		    return NULL;
		  }
	      }
	  }
	else
	  {
	    /* The data is never used; just change the name.  */
	    newnames[category] = copy (newnames[category]);
	    if (! newnames[category])
	      {
		if (!strcmp (newnames[category], "C") ||
		    !strcmp (newnames[category], "POSIX"))
		  newnames[category] = (char *) _nl_C_name;
		else
		  {
		    while (++category < LC_ALL)
		      if (newnames[category] != _nl_C_name)
			free (newnames[category]);
		  }
	      }
	  }

      composite = new_composite_name (LC_ALL, newnames);
      if (! composite)
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
      setcomposite (composite);

      return composite;
    }
  else
    {
      char *newname = copy (name);
      if (! newname)
	{
	  if (!strcmp (name, "C") || !strcmp (name, "POSIX"))
	    newname = (char *) _nl_C_name;
	  else
	    return NULL;
	}

      composite = new_composite_name (category, &newname);
      if (! composite)
	{
	  if (newname != _nl_C_name)
	    free (newname);
	  return NULL;
	}

      /* Only actually load the data if anything will use it.  */
      if (_nl_current[category])
	{
	  struct locale_data *newdata = _nl_load_locale (category,
							 (char **) &name);
	  if (! newdata)
	    {
	      if (!strcmp (name, "C") || !strcmp (name, "POSIX"))
		newdata = (struct locale_data *) _nl_C[category];
	      else
		return NULL;
	    }
	  setdata (category, newdata);
	}

      setname (category, newname);
      setcomposite (composite);

      return newname;
    }
}
