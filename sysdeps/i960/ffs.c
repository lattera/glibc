/* ffs -- find first set bit in a word, counted from least significant end.
   For i960 Core architecture
   Copyright (C) 1994 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.

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
#include <string.h>

#undef	ffs

#if	defined (__GNUC__) && defined (__i960__)

int
DEFUN(ffs, (x), int x)
{
  int cnt;

  asm("scanbit %1,%0" : "=d" (cnt) : "rm" (x & -x));

  return cnt;
}

#else

#include <sysdeps/generic/ffs.c>

#endif
