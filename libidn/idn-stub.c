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

/* Get specification for idna_to_ascii_lz. */
#include "idna.h"

/* Handle of the libidn  DSO.  */
static void *h;


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
      *output = strdup (input);
      return *output == NULL ? IDNA_MALLOC_ERROR : IDNA_SUCCESS;
    }

  static int (*to_ascii_lz) (const char *input, char **output, int flags);

  if (h == NULL)
    {
      h = __libc_dlopen ("libcidn.so");

      if (h == NULL)
	h = (void *) 1l;
      else
	{
	  /* Get the function we are interested in.  */
	  to_ascii_lz = __libc_dlsym (h, "idna_to_ascii_lz");
	  if (to_ascii_lz == NULL)
	    {
	      __libc_dlclose (h);
	      h = (void *) 1l;
	    }
	}
    }

  if (h == (void *) 1l)
    return IDNA_DLOPEN_ERROR;

  return to_ascii_lz (input, output, flags);
}


libc_freeres_fn (unload_libidn)
{
  if (h != NULL && h != (void *) 1l)
    {
      __libc_dlclose (h);
      h = (void *) 1l;
    }
}
