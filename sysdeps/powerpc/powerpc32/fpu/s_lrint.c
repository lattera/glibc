/* Round floating-point to integer.  PowerPC version.
   Copyright (C) 1997,2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Kludge to avoid decls that will confuse strong_alias below.  */
#define __lrintf XXX__lrintf
#define __lrintl XXX__lrintl
#define lrintf XXXlrintf
#define lrintl XXXlrintl

#include "math.h"

#undef __lrintf
#undef __lrintl
#undef lrintf
#undef lrintl


long int
__lrint (double x)
{
  union
  {
    double d;
    int ll[2];
  } u;
  asm ("fctiw %0,%1" : "=f"(u.d) : "f"(x));
  return u.ll[1];
}
weak_alias (__lrint, lrint)

strong_alias (__lrint, __lrintf)
weak_alias (__lrint, lrintf)

#ifdef NO_LONG_DOUBLE
strong_alias (__lrint, __lrintl)
weak_alias (__lrint, lrintl)
#endif
