/* idn-stub.c --- Stub to dlopen libcidn.so and invoke idna_to_ascii_lz.
 * Copyright (C) 2003, 2004  Simon Josefsson
 *
 * This file is part of GNU Libidn.
 *
 * GNU Libidn is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GNU Libidn is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GNU Libidn; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>
#include <bits/libc-lock.h>

/* Get specification for idna_to_ascii_lz. */
#include "idna.h"

/* Handle of the libidn  DSO.  */
static void *h;


static int (*to_ascii_lz) (const char *input, char **output, int flags);
static int (*to_unicode_lzlz) (const char *input, char **output, int flags);


static void
load_dso (void)
{
  /* Lock protecting the DSO loading.  */
  __libc_lock_define_initialized (static, lock);

  __libc_lock_lock (lock);

  /* Retest in case some other thread arrived here at the same time.  */
  if (h == NULL)
    {
      h = __libc_dlopen (LIBCIDN_SO);

      if (h == NULL)
	h = (void *) 1l;
      else
	{
	  /* Get the function we are interested in.  */
	  to_ascii_lz = __libc_dlsym (h, "idna_to_ascii_lz");
	  to_unicode_lzlz = __libc_dlsym (h, "idna_to_unicode_lzlz");
	  if (to_ascii_lz == NULL || to_unicode_lzlz == NULL)
	    {
	      __libc_dlclose (h);
	      h = (void *) 1l;
	    }
	}
    }

  __libc_lock_unlock (lock);
}


/* Stub to dlopen libcidn.so and invoke the real idna_to_ascii_lz, or
   return IDNA_DLOPEN_ERROR on failure.  */
int
__idna_to_unicode_lzlz (const char *input, char **output, int flags)
{
  /* If the input string contains no "xn--" prefix for a component of
     the name we can pass it up right away.  */
  const char *cp = input;
  while (*cp != '\0')
    {
      if (strncmp (cp, IDNA_ACE_PREFIX, strlen (IDNA_ACE_PREFIX)) == 0)
	break;

      /* On to the next part of the name.  */
      cp = __strchrnul (cp, '.');
      if (*cp == '.')
	++cp;
    }

  if (*cp == '\0')
    {
      *output = (char *) input;
      return IDNA_SUCCESS;
    }

  if (h == NULL)
    load_dso ();

  if (h == (void *) 1l)
    return IDNA_DLOPEN_ERROR;

  return to_unicode_lzlz (input, output, flags);
}


/* Stub to dlopen libcidn.so and invoke the real idna_to_ascii_lz, or
   return IDNA_DLOPEN_ERROR on failure.  */
int
__idna_to_ascii_lz (const char *input, char **output, int flags)
{
  /* If the input string contains no non-ASCII character the output
     string will be the same.  No valid locale encoding does not have
     this property.  */
  const char *cp = input;
  while (*cp != '\0' && isascii (*cp))
    ++cp;

  if (*cp == '\0')
    {
      *output = (char *) input;
      return IDNA_SUCCESS;
    }

  if (h == NULL)
    load_dso ();

  if (h == (void *) 1l)
    return IDNA_DLOPEN_ERROR;

  return to_ascii_lz (input, output, flags);
}


#ifndef NOT_IN_libc
libc_freeres_fn (unload_libidn)
{
  if (h != NULL && h != (void *) 1l)
    {
      __libc_dlclose (h);
      h = (void *) 1l;
    }
}
#endif
