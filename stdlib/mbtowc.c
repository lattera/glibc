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

#include <ansidecl.h>
#include "../locale/localeinfo.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


long int _mb_shift = 0;

/* Convert the multibyte character at S, which is no longer
   than N characters, to its `wchar_t' representation, placing
   this n *PWC and returning its length.  */
int
DEFUN(mbtowc, (pwc, s, n), wchar_t *pwc AND CONST char *s AND size_t n)
{
#if 0
  register CONST mb_char *mb;
  register wchar_t i;
#endif

  if (s == NULL)
    return _mb_shift != 0;

  if (*s == '\0')
    /* ANSI 4.10.7.2, line 19.  */
    return 0;

  if (isascii (*s))
    {
      /* A normal ASCII character translates to itself.  */
      if (pwc != NULL)
	*pwc = (wchar_t) *s;
      return 1;
    }

#if 0
  if (_ctype_info->mbchar == NULL ||
      _ctype_info->mbchar->mb_chars == NULL)
    return -1;

  if (n > MB_CUR_MAX)
    n = MB_CUR_MAX;

  for (i = 0; i < WCHAR_MAX; ++i)
    {
      mb = &_ctype_info->mbchar->mb_chars[i];
      /* EOF and NUL aren't MB chars.  */
      if (i == (wchar_t) EOF || i == (wchar_t) '\0')
	continue;
      /* Normal ASCII values can't start MB chars.  */
      else if (isascii(i))
	continue;
      else if (mb->string == NULL || mb->len == 0)
	continue;
      else if (mb->len > n)
	continue;
      else if (!strncmp(mb->string, s, mb->len))
	{
	  _mb_shift += mb->shift;
	  if (pwc != NULL)
	    *pwc = i;
	  return mb->len;
	}
    }
#endif

  return -1;
}
