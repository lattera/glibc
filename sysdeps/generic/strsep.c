/* Copyright (C) 1992, 1993, 1996 Free Software Foundation, Inc.
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

#include <string.h>

char *
strsep (char **stringp, const char *delim)
{
  char *begin, *end;

  begin = *stringp;
  if (! begin || *begin == '\0')
    return NULL;

  /* Find the end of the token.  */
  end = strpbrk (begin, delim);
  if (end)
    {
      /* Terminate the token and advance *STRINGP past the delimiters.  */
      *end++ = '\0';
      *stringp = end + strspn (end, delim);
    }
  else
    /* No more delimiters; this is the last token.  */
    *stringp = NULL;

  return begin;
}
