/* ffs -- find first set bit in a word, counted from least significant end.
   For IBM rs6000.
   Copyright (C) 1991, 1992, 1997, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Torbjorn Granlund (tege@sics.se).

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

#ifdef	__GNUC__

int
__ffs (x)
     int x;
{
  int cnt;

  asm ("cntlz %0,%1" : "=r" (cnt) : "r" (x & -x));
  return 32 - cnt;
}
weak_alias (__ffs, ffs)
libc_hidden_builtin_def (ffs)

#else
#include <sysdeps/generic/ffs.c>
#endif
