/* Copyright (C) 1991, 1992, 1995, 1996 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <wchar.h>


extern mbstate_t __no_r_state;	/* Defined in mbtowc.c.  */

/* Convert the `wchar_t' string in PWCS to a multibyte character string
   in S, writing no more than N characters.  Return the number of bytes
   written, or (size_t) -1 if an invalid `wchar_t' was found.

   Attention: this function should NEVER be intentionally used.
   The interface is completely stupid.  The state is shared between
   all conversion functions.  You should use instead the restartable
   version `wcsrtombs'.  */
size_t
wcstombs (char *s, const wchar_t *pwcs, size_t n)
{
  mbstate_t save_shift = __no_r_state;
  size_t written;

  written = __wcsrtombs (s, &pwcs, n, &__no_r_state);

  /* Restore the old shift state.  */
  __no_r_state = save_shift;

  /* Return how many we wrote (or maybe an error).  */
  return written;
}
