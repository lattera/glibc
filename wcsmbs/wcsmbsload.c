/* Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <ctype.h>
#include <langinfo.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <locale/localeinfo.h>
#include <wcsmbsload.h>
#include <bits/libc-lock.h>
#include <iconv/gconv_int.h>


/* Last loaded locale for LC_CTYPE.  We initialize for the C locale
   which is enabled at startup.  */
extern const struct locale_data _nl_C_LC_CTYPE;
const struct locale_data *__wcsmbs_last_locale = &_nl_C_LC_CTYPE;


/* These are the descriptions for the default conversion functions.  */
static struct __gconv_step to_wc =
{
  .__shlib_handle = NULL,
  .__modname = NULL,
  .__counter = INT_MAX,
  .__from_name = "ANSI_X3.4-1968//TRANSLIT",
  .__to_name = "INTERNAL",
  .__fct = __gconv_transform_ascii_internal,
  .__init_fct = NULL,
  .__end_fct = NULL,
  .__min_needed_from = 1,
  .__max_needed_from = 1,
  .__min_needed_to = 4,
  .__max_needed_to = 4,
  .__stateful = 0,
  .__data = NULL
};

static struct __gconv_step to_mb =
{
  .__shlib_handle = NULL,
  .__modname = NULL,
  .__counter = INT_MAX,
  .__from_name = "INTERNAL",
  .__to_name = "ANSI_X3.4-1968//TRANSLIT",
  .__fct = __gconv_transform_internal_ascii,
  .__init_fct = NULL,
  .__end_fct = NULL,
  .__min_needed_from = 4,
  .__max_needed_from = 4,
  .__min_needed_to = 1,
  .__max_needed_to = 1,
  .__stateful = 0,
  .__data = NULL
};


/* For the default locale we only have to handle ANSI_X3.4-1968.  */
struct gconv_fcts __wcsmbs_gconv_fcts =
{
  .towc = &to_wc,
  .tomb = &to_mb
};


static inline struct __gconv_step *
getfct (const char *to, const char *from)
{
  size_t nsteps;
  struct __gconv_step *result;
  size_t nstateful;
  size_t cnt;

  if (__gconv_find_transform (to, from, &result, &nsteps, 0) != __GCONV_OK)
    /* Loading the conversion step is not possible.  */
    return NULL;

  /* Count the number of stateful conversions.  Since we will only
     have one 'mbstate_t' object available we can only deal with one
     stateful conversion.  */
  nstateful = 0;
  for (cnt = 0; cnt < nsteps; ++cnt)
    if (result[cnt].__stateful)
      ++nstateful;
  if (nstateful > 1)
    {
      /* We cannot handle this case.  */
      __gconv_close_transform (result, nsteps);
      result = NULL;
    }

  return result;
}


/* Extract from the given locale name the character set portion.  Since
   only the XPG form of the name includes this information we don't have
   to take care for the CEN form.  */
#define extract_charset_name(str) \
  ({									      \
    const char *cp = str;						      \
    char *result = NULL;						      \
									      \
    cp += strcspn (cp, "@.+,");						      \
    if (*cp == '.')							      \
      {									      \
	const char *endp = ++cp;					      \
	while (*endp != '\0' && *endp != '@')				      \
	  ++endp;							      \
	if (endp != cp)							      \
	  result = strndupa (cp, endp - cp);				      \
      }									      \
    result;								      \
  })


/* We must modify global data.  */
__libc_lock_define_initialized (static, lock)


/* Load conversion functions for the currently selected locale.  */
void
internal_function
__wcsmbs_load_conv (const struct locale_data *new_category)
{
  /* Acquire the lock.  */
  __libc_lock_lock (lock);

  /* We should repeat the test since while we waited some other thread
     might have run this function.  */
  if (__builtin_expect (__wcsmbs_last_locale != new_category, 1))
    {
      if (new_category->name == _nl_C_name)	/* Yes, pointer comparison.  */
	{
	failed:
	  __wcsmbs_gconv_fcts.towc = &to_wc;
	  __wcsmbs_gconv_fcts.tomb = &to_mb;
	}
      else
	{
	  /* We must find the real functions.  */
	  const char *charset_name;
	  const char *complete_name;
	  struct __gconv_step *new_towc;
	  struct __gconv_step *new_tomb;
	  int use_translit;

	  /* Free the old conversions.  */
	  __gconv_close_transform (__wcsmbs_gconv_fcts.tomb, 1);
	  __gconv_close_transform (__wcsmbs_gconv_fcts.towc, 1);

	  /* Get name of charset of the locale.  */
	  charset_name = new_category->values[_NL_ITEM_INDEX(CODESET)].string;

	  /* Does the user want transliteration?  */
	  use_translit = new_category->use_translit;

	  /* Normalize the name and add the slashes necessary for a
             complete lookup.  */
	  complete_name = norm_add_slashes (charset_name,
					    use_translit ? "TRANSLIT" : NULL);

	  /* It is not necessary to use transliteration in this direction
	     since the internal character set is supposed to be able to
	     represent all others.  */
	  new_towc = getfct ("INTERNAL", complete_name);
	  new_tomb = (new_towc != NULL
		      ? getfct (complete_name, "INTERNAL") : NULL);

	  /* If any of the conversion functions is not available we don't
	     use any since this would mean we cannot convert back and
	     forth.*/
	  if (new_towc == NULL || new_tomb == NULL)
	    {
	      if (new_towc != NULL)
		__gconv_close_transform (new_towc, 1);

	      goto failed;
	    }

	  __wcsmbs_gconv_fcts.tomb = new_tomb;
	  __wcsmbs_gconv_fcts.towc = new_towc;
	}

      /* Set last-used variable for current locale.  */
      __wcsmbs_last_locale = new_category;
    }

  __libc_lock_unlock (lock);
}


/* Clone the current conversion function set.  */
void
internal_function
__wcsmbs_clone_conv (struct gconv_fcts *copy)
{
  /* First make sure the function table is up-to-date.  */
  update_conversion_ptrs ();

  /* Make sure the data structures remain the same until we are finished.  */
  __libc_lock_lock (lock);

  /* Copy the data.  */
  *copy = __wcsmbs_gconv_fcts;

  /* Now increment the usage counters.  */
  if (copy->towc->__shlib_handle != NULL)
    ++copy->towc->__counter;
  if (copy->tomb->__shlib_handle != NULL)
    ++copy->tomb->__counter;

  __libc_lock_unlock (lock);
}


/* Clone the current conversion function set.  */
int
internal_function
__wcsmbs_named_conv (struct gconv_fcts *copy, const char *name)
{
  copy->towc = getfct ("INTERNAL", name);
  if (copy->towc != NULL)
    {
      copy->tomb = getfct (name, "INTERNAL");
      if (copy->tomb == NULL)
	__gconv_close_transform (copy->towc, 1);
    }

  if (copy->towc == NULL || copy->tomb == NULL)
    return 1;

  /* Now increment the usage counters.  */
  if (copy->towc->__shlib_handle != NULL)
    ++copy->towc->__counter;
  if (copy->tomb->__shlib_handle != NULL)
    ++copy->tomb->__counter;

  return 0;
}
