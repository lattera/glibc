/* Copyright (C) 2000, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 2000.

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

#include <wchar.h>
#include <wctype.h>

#include "../locale/outdigits.h"
#include "../locale/outdigitswc.h"

static CHAR_T *
_i18n_number_rewrite (CHAR_T *w, CHAR_T *rear_ptr)
{
#ifdef COMPILE_WPRINTF
# define decimal NULL
# define thousands NULL
#else
  char decimal[MB_LEN_MAX];
  char thousands[MB_LEN_MAX];
#endif

  /* "to_outpunct" is a map from ASCII decimal point and thousands-sep
     to their equivalent in locale. This is defined for locales which
     use extra decimal point and thousands-sep.  */
  wctrans_t map = __wctrans ("to_outpunct");
  wint_t wdecimal = __towctrans (L'.', map);
  wint_t wthousands = __towctrans (L',', map);

#ifndef COMPILE_WPRINTF
  if (__builtin_expect (map != NULL, 0))
    {
      mbstate_t state;
      memset (&state, '\0', sizeof (state));

      if (__wcrtomb (decimal, wdecimal, &state) == (size_t) -1)
	memcpy (decimal, ".", 2);

      memset (&state, '\0', sizeof (state));

      if (__wcrtomb (thousands, wthousands, &state) == (size_t) -1)
	memcpy (thousands, ",", 2);
    }
#endif

  /* Copy existing string so that nothing gets overwritten.  */
  CHAR_T *src = (CHAR_T *) alloca ((rear_ptr - w) * sizeof (CHAR_T));
  CHAR_T *s = (CHAR_T *) __mempcpy (src, w,
				    (rear_ptr - w) * sizeof (CHAR_T));
  w = rear_ptr;

  /* Process all characters in the string.  */
  while (--s >= src)
    {
      if (*s >= '0' && *s <= '9')
	{
	  if (sizeof (CHAR_T) == 1)
	    w = (CHAR_T *) outdigit_value ((char *) w, *s - '0');
	  else
	    *--w = (CHAR_T) outdigitwc_value (*s - '0');
	}
      else if (__builtin_expect (map == NULL, 1) || (*s != '.' && *s != ','))
	*--w = *s;
      else
	{
	  if (sizeof (CHAR_T) == 1)
	    {
	      const char *outpunct = *s == '.' ? decimal : thousands;
	      size_t dlen = strlen (outpunct);

	      w -= dlen;
	      while (dlen-- > 0)
		w[dlen] = outpunct[dlen];
	    }
	  else
	    *--w = *s == '.' ? (CHAR_T) wdecimal : (CHAR_T) wthousands;
	}
    }

  return w;
}
