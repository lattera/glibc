/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

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

#include <math.h>

double
__fabs (double x)
{
#if __GNUC_PREREQ (2, 8)
  return __builtin_fabs (x);
#else
  __asm ("cpys $f31, %1, %0" : "=f" (x) : "f" (x));
  return x;
#endif
}

weak_alias (__fabs, fabs)
#ifdef NO_LONG_DOUBLE
strong_alias (__fabs, __fabsl)
weak_alias (__fabs, fabsl)
#endif
