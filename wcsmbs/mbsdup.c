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


/* Duplicate MBS, returning an identical malloc'd string.  */
char *
mbsdup (mbs)
    const char *mbs;
{
  size_t len = 0;
  int clen = 0;
  char *retval;

  /* Reset multibyte characters to their initial state.	 */
  (void) mblen ((char *) NULL, 0);

  do
    {
      len += clen;
      clen = mblen (&mbs[len], MB_CUR_MAX);
    }
  while (clen > 0);

  retval = (char *) malloc (len + 1);
  if (retval != NULL)
    {
      (void) memcpy ((void *) retval, (void *) mbs, len);
      retval[len] = '\0';   /* '\0' is the multibyte representation of L'\0' */
    }

  return retval;
}

