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
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>


extern int _mb_shift;	/* Defined in mbtowc.c.  */

/* Convert the string of multibyte characters in S to `wchar_t's in
   PWCS, writing no more than N.  Return the number written,
   or (size_t) -1 if an invalid multibyte character is encountered.  */
size_t
DEFUN(mbstowcs, (pwcs, s, n),
      register wchar_t *pwcs AND register CONST char *s AND register size_t n)
{
  int save_shift;
  register size_t written = 0;

  /* Save the shift state.  */
  save_shift = _mb_shift;
  /* Reset the shift state.  */
  _mb_shift = 0;

  while (*s != '\0')
    {
      int len;
      if (isascii (*s))
	{
	  *pwcs = (wchar_t) *s;
	  len = 1;
	}
      else
	len = mbtowc (pwcs, s, n);

      if (len < 1)
	{
	  /* Return an error.  */
	  written = (size_t) -1;
	  break;
	}
      else
	{
	  /* Multibyte character converted.  */
	  ++pwcs;
	  ++written;
	  s += len;
	  n -= len;
	}
    }

  /* Terminate the string if it has space.  */
  if (n > 0)
    *pwcs = (wchar_t) 0;

  /* Restore the old shift state.  */
  _mb_shift = save_shift;

  /* Return how many we wrote (or maybe an error).  */
  return written;
}
