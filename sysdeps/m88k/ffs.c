/* ffs -- find first set bit in a word, counted from least significant end.
   For Motorola 88000.
   Copyright (C) 1991, 1992 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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
#include <bstring.h>

#undef	ffs

#ifdef	__GNUC__

int
DEFUN(ffs, (x), int x)
{
  int cnt;

  if (x == 0)
    return 0;

  asm ("ff1 %0,%1" : "=r" (cnt) : "r" (x & -x));
  return cnt + 1;
}

#else
#include <sysdeps/generic/ffs.c>
#endif
