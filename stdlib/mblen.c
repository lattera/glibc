/* Copyright (C) 1991, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stdlib.h>
#include <wchar.h>


/* Return the length of the multibyte character (if there is one)
   at S which is no longer than N characters.
   The ISO C standard says that the `mblen' function must not change
   the global state.  */
int
mblen (const char *s, size_t n)
{
  mbstate_t state;
  int result;

  /* If S is NULL the function has to return null or not null
     depending on the encoding having a state depending encoding or
     not.  This is nonsense because any multibyte encoding has a
     state.  The ISO C amendment 1 corrects this while introducing the
     restartable functions.  We simply say here all encodings have a
     state.  */
  if (s == NULL)
    result = 1;
  else if (*s == '\0')
    /* According to the ISO C 89 standard this is the expected behaviour.
       Idiotic, but true.  */
    result = 0;
  else
    {
      state.count = 0;
      state.value = 0;

      result = __mbrtowc (NULL, s, n, &state);

      /* The `mbrtowc' functions tell us more than we need.  Fold the -1
	 and -2 result into -1.  */
      if (result < 0)
	result = -1;
    }

  return result;
}
