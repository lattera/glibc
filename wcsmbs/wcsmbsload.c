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
static struct gconv_step to_wc =
{
  shlib_handle: NULL,
  modname: NULL,
  counter: INT_MAX,
  from_name: "ANSI_X3.4-1968//",
  to_name: "INTERNAL",
  fct: __gconv_transform_ascii_internal,
  init_fct: NULL,
  end_fct: NULL,
  data: NULL
};

static struct gconv_step to_mb =
{
  shlib_handle: NULL,
  modname: NULL,
  counter: INT_MAX,
  from_name: "INTERNAL",
  to_name: "ANSI_X3.4-1968//",
  fct: __gconv_transform_internal_ascii,
  init_fct: NULL,
  end_fct: NULL,
  data: NULL
};


/* For the default locale we only have to handle ANSI_X3.4-1968.  */
struct gconv_fcts __wcsmbs_gconv_fcts =
{
  towc: &to_wc,
  tomb: &to_mb
};


static inline struct gconv_step *
getfct (const char *to, const char *from)
{
  size_t nsteps;
  struct gconv_step *result;

  if (__gconv_find_transform (to, from, &result, &nsteps) != GCONV_OK)
    /* Loading the conversion step is not possible.  */
    return NULL;

  /* We must only have one step in this conversion.  */
  if (nsteps != 1)
    return NULL;

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
    while (strchr ("@.+,", *cp) == NULL)				      \
      ++cp;								      \
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


/* The gconv functions expects the name to be complete, including the
   trailing shashes if necessary.  */
#define add_slashes(str) \
  ({									      \
    const char *cp = str;						      \
    char *result;							      \
    char *tmp;								      \
    size_t cnt = 0;							      \
									      \
    while (*cp != '\0')							      \
      if (*cp++ == '/')							      \
	++cnt;								      \
									      \
    result = alloca (cp - str + 3);					      \
    tmp = __mempcpy (result, str, cp - str);				      \
    if (cnt < 2)							      \
      {									      \
	*tmp++ = '/';							      \
	if (cnt < 1)							      \
	  *tmp++ = '/';							      \
      }									      \
    *tmp = '\0';							      \
    result;								      \
  })


/* Load conversion functions for the currently selected locale.  */
void
__wcsmbs_load_conv (const struct locale_data *new_category)
{
  /* We must modify global data.  */
  __libc_lock_define_initialized (static, lock)

  /* Acquire the lock.  */
  __libc_lock_lock (lock);

  /* We should repest the test since while we waited some other thread
     might have run this function.  */
  if (__wcsmbs_last_locale != new_category)
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

	  /* Get name of charset of the locale.  We first examine
	     whether we have a character set mentioned in the locale
	     name.  If this isn't the case we use the information from
	     the locale files.  */
	  charset_name = extract_charset_name (setlocale (LC_CTYPE, NULL));
	  if (charset_name == NULL)
	    charset_name =
	      new_category->values[_NL_ITEM_INDEX(CODESET)].string;

	  /* Add the slashes necessary for a complete lookup.  */
	  complete_name = add_slashes (charset_name);

	  __wcsmbs_gconv_fcts.tomb = getfct (complete_name, "INTERNAL");
	  __wcsmbs_gconv_fcts.towc = getfct ("INTERNAL", complete_name);

	  /* If any of the conversion functions is not available we don't
	     use any since this would mean we cannot convert back and
	     forth.*/
	  if (__wcsmbs_gconv_fcts.towc == NULL
	      || __wcsmbs_gconv_fcts.tomb == NULL)
	    goto failed;
	}

      /* Set last-used variable for current locale.  */
      __wcsmbs_last_locale = new_category;
    }

  __libc_lock_unlock (lock);
}
