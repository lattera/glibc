/* ISO C extended string formatting.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <time.h>

/* The ISO C 9X standard extended the `struct tm' structure to contain some
   more information necessary for the new formats.  But the struct format
   we used so far already contains the information and since the `struct tm'
   and `struct tmx' structures match exactly in the first part.  So we can
   simply use `strftime' to implement `strfxtime'.  */
size_t
strfxtime (char *s, size_t maxsize, const char *format,
	   const struct tmx *timeptr)
{
  return strftime (s, maxsize, format, (const struct tm *) timeptr);
}
