/* ffs -- find first set bit in a word, counted from least significant end.
   For i960 Core architecture
   This file is part of the GNU C Library.
   Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
   On-Line Applications Research Corporation.

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

#include <string.h>

#undef	ffs

#if	defined (__GNUC__) && defined (__i960__)

int
__ffs (x)
     int x;
{
  int cnt;

  asm ("scanbit %1,%0" : "=d" (cnt) : "rm" (x & -x));

  return cnt;
}
weak_alias (__ffs, ffs)

#else

#include <sysdeps/generic/ffs.c>

#endif
