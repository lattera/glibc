/* Copyright (C) 1991, 1992, 1996 Free Software Foundation, Inc.
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

/* Convert the string of multibyte characters in S to `wchar_t's in
   PWCS, writing no more than N.  Return the number written,
   or (size_t) -1 if an invalid multibyte character is encountered.

   Attention: this function should NEVER be intentionally used.
   The interface is completely stupid.  The state is shared between
   all conversion functions.  You should use instead the restartable
   version `mbsrtowcs'.  */
size_t
mbstowcs (wchar_t *pwcs, const char *s, size_t n)
{
  mbstate_t save_shift = __no_r_state;
  size_t written;

  written = mbsrtowcs (pwcs, s, n, &__no_r_state);

  /* Restore the old shift state.  */
  __no_r_state = save_shift;

  /* Return how many we wrote (or maybe an error).  */
  return written;
}
