/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Convert the `wchar_t' string in PWCS to a multibyte character string
   in S, writing no more than N characters.  Return the number of bytes
   written, or (size_t) -1 if an invalid `wchar_t' was found.  */
size_t
DEFUN(wcstombs, (s, pwcs, n),
      register char *s AND register CONST wchar_t *pwcs AND register size_t n)
{
  register CONST mb_char *mb;
  register int shift = 0;

  register size_t written = 0;
  register wchar_t w;

  while ((w = *pwcs++) != (wchar_t) '\0')
    {
      if (isascii (w))
	{
	  /* A normal character.  */
	  *s++ = (unsigned char) w;
	  --n;
	  ++written;
	}
      else
	{
	  mb = &_ctype_info->mbchar->mb_chars[w + shift];
	  if (mb->string == NULL || mb->len == 0)
	    {
	      written = (size_t) -1;
	      break;
	    }
	  else if (mb->len > n)
	    break;
	  else
	    {
	      memcpy ((PTR) s, (CONST PTR) mb->string, mb->len);
	      s += mb->len;
	      n -= mb->len;
	      written += mb->len;
	      shift += mb->shift;
	    }
	}
    }

  /* Terminate the string if it has space.  */
  if (n > 0)
    *s = '\0';

  /* Return the number of characters written (or maybe an error).  */
  return written;
}
