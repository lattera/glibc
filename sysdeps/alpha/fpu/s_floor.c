/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

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

#ifndef __USE_EXTERN_INLINES
#define __USE_EXTERN_INLINES
#endif
#define __floor __i_floor

#include <math.h>

#undef __floor

double
__floor (double x)
{
  return __i_floor(x);
}

weak_alias (__floor, floor)
#ifdef NO_LONG_DOUBLE
strong_alias (__floor, __floorl)
weak_alias (__floor, floorl)
#endif
