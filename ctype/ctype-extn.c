/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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

#define	__NO_CTYPE
#include <ctype.h>

/* Real function versions of the non-ANSI ctype functions.  */

int
isblank (int c)
{
  return __isctype (c, _ISblank);
}

int
_tolower (int c)
{
  return __tolower (c);
}
int
_toupper (int c)
{
  return __toupper (c);
}

int
toascii (int c)
{
  return __toascii (c);
}
int
isascii (int c)
{
  return __isascii (c);
}


int
__isblank_l (int c, __locale_t l)
{
  return __isctype_l (c, _ISblank, l);
}

int
__toascii_l (int c, __locale_t l)
{
  return __toascii (c);
}
int
__isascii_l (int c, __locale_t l)
{
  return __isascii (c);
}
