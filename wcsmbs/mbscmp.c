/* Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.	 If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <mbstr.h>
#include <stdlib.h>

#define __need_wchar_t
/* FIXME: should be defined in stddef.h.
!!! #define __need_uwchar_t  */
typedef unsigned int uwchar_t;
#include <stddef.h>


/* Compare MBS1 and MBS2.  */
int
mbscmp (mbs1, mbs2)
    const char *mbs1;
    const char *mbs2;
{
  int len1 = 0;
  int len2 = 0;
  uwchar_t c1;
  uwchar_t c2;

  /* Reset multibyte characters to their initial state.	 */
  (void) mblen ((char *) NULL, 0);

  do
    {
      len1 = mbtowc ((wchar_t *) &c1, mbs1, MB_CUR_MAX);
      len2 = mbtowc ((wchar_t *) &c2, mbs2, MB_CUR_MAX);

      if (len1 == 0)
	return len2 == 0 ? 0 : -1;
      if (len2 == 0)
	return 1;
      if (len1 < 0 || len2 < 0)
	/* FIXME: an illegal character appears.	 What to do?  */
	return c1 - c2;

      mbs1 += len1;
      mbs2 += len2;
    }
  while (c1 == c2);

  return c1 - c2;
}

