/* Round floating-point to integer.  PowerPC version.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#include "math.h"

#ifdef NO_LONG_DOUBLE

long int
__lrint (long double x)
{
  return (long int) __rintl(x);
}

#else

long int
__lrint (long double x)
{
  union {
    double d;
    long int ll[2];
  } u;
  asm ("fctiw %0,%1" : "=f"(u.d) : "f"(x));
  return d.ll[1];
}

#endif
weak_alias (__lrint, lrint)
