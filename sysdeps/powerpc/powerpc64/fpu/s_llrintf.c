/* Round a float value to a long long in the current rounding mode.
   Copyright (C) 1997, 2004 Free Software Foundation, Inc.
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
#define __lrintl XXX__lrintf
#define lrintl XXXlrintf

#include "math.h"

#undef __lrintf
#undef lrintf

long long int
__llrintf (float x)
{
  return (long long int) __rintf (x);
}
strong_alias (__llrintf, __lrintf)
weak_alias (__llrintf, llrintf)
weak_alias (__lrintf, lrintf)
