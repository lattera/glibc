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
#include <localeinfo.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


extern long int _mb_shift;	/* Defined in mbtowc.c.  */

/* Convert WCHAR into its multibyte character representation,
   putting this in S and returning its length.  */
int
DEFUN(wctomb, (s, wchar), register char *s AND wchar_t wchar)
{
  register CONST mb_char *mb;

  if (_ctype_info->mbchar == NULL)
    mb = NULL;
  else
    mb = _ctype_info->mbchar->mb_chars;

  /* If S is NULL, just say if we're shifted or not.  */
  if (s == NULL)
    return _mb_shift != 0;

  if (wchar == (wchar_t) '\0')
    {
      _mb_shift = 0;
      /* See ANSI 4.4.1.1, line 21.  */
      if (s != NULL)
	*s = '\0';
      return 1;
    }
  else if (mb == NULL)
    {
      if ((wchar_t) (char) wchar == wchar && isascii ((char) wchar))
	{
	  /* A normal ASCII character translates to itself.  */
	  if (s != NULL)
	    *s = (char) wchar;
	  return 1;
	}
      return -1;
    }

  mb += wchar + _mb_shift;
  if (mb->string == NULL || mb->len == 0)
    return -1;
  memcpy((PTR) s, (CONST PTR) mb->string, mb->len + 1);
  _mb_shift += mb->shift;
  return mb->len;
}
