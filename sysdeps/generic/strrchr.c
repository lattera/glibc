/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
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
#include <string.h>


/* Find the last ocurrence of C in S.  */
char *
DEFUN(strrchr, (s, c), CONST char *s AND int c)
{
  register CONST char *found, *p;

  c = (unsigned char) c;

  /* Since strchr is fast, we use it rather than the obvious loop.  */
  
  if (c == '\0')
    return strchr(s, '\0');

  found = NULL;
  while ((p = strchr(s, c)) != NULL)
    {
      found = p;
      s = p + 1;
    }

  return (char *) found;
}

#ifdef weak_alias
#undef rindex
weak_alias (strrchr, rindex)
#endif
