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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/* Transform SRC into a form such that the result of strcmp
   on two strings that have been transformed by strxfrm is
   the same as the result of strcoll on the two strings before
   their transformation.  The transformed string is put in at
   most N characters of DEST and its length is returned.  */
size_t
strxfrm (dest, src, n)
     char *dest;
     const char *src;
     size_t n;
{
  if (n == 0)
    return strlen (src);

  return __stpncpy (dest, src, n) - dest;
}
