/* 64-bit integer shifting.
   Copyright (C) 1989, 1992-2001, 2002 Free Software Foundation, Inc.
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

#include <endian.h>
#include <stdlib.h>
#include <bits/wordsize.h>
#include <shlib-compat.h>

#if __WORDSIZE != 32
# error This is for 32-bit targets only
#endif

#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_2_6)

typedef unsigned int UQItype	__attribute__ ((mode (QI)));
typedef          int SItype	__attribute__ ((mode (SI)));
typedef unsigned int USItype	__attribute__ ((mode (SI)));
typedef          int DItype	__attribute__ ((mode (DI)));
typedef unsigned int UDItype	__attribute__ ((mode (DI)));
#define Wtype SItype
#define HWtype SItype
#define DWtype DItype
#define UWtype USItype
#define UHWtype USItype
#define UDWtype UDItype
#define W_TYPE_SIZE 32
#define BITS_PER_UNIT 8

#include <stdlib/longlong.h>

#if __BYTE_ORDER == __BIG_ENDIAN
struct DWstruct { Wtype high, low;};
#elif __BYTE_ORDER == __LITTLE_ENDIAN
struct DWstruct { Wtype low, high;};
#else
#error Unhandled endianity
#endif
typedef union { struct DWstruct s; DWtype ll; } DWunion;


DWtype
___lshrdi3 (DWtype u, Wtype b)
{
  DWunion w;
  Wtype bm;
  DWunion uu;

  if (b == 0)
    return u;

  uu.ll = u;

  bm = (sizeof (Wtype) * BITS_PER_UNIT) - b;
  if (bm <= 0)
    {
      w.s.high = 0;
      w.s.low = (UWtype) uu.s.high >> -bm;
    }
  else
    {
      UWtype carries = (UWtype) uu.s.high << bm;

      w.s.high = (UWtype) uu.s.high >> b;
      w.s.low = ((UWtype) uu.s.low >> b) | carries;
    }

  return w.ll;
}
symbol_version (___lshrdi3, __lshrdi3, GLIBC_2.0);


DWtype
___ashldi3 (DWtype u, Wtype b)
{
  DWunion w;
  Wtype bm;
  DWunion uu;

  if (b == 0)
    return u;

  uu.ll = u;

  bm = (sizeof (Wtype) * BITS_PER_UNIT) - b;
  if (bm <= 0)
    {
      w.s.low = 0;
      w.s.high = (UWtype) uu.s.low << -bm;
    }
  else
    {
      UWtype carries = (UWtype) uu.s.low >> bm;

      w.s.low = (UWtype) uu.s.low << b;
      w.s.high = ((UWtype) uu.s.high << b) | carries;
    }

  return w.ll;
}
symbol_version (___ashldi3, __ashldi3, GLIBC_2.0);


DWtype
___ashrdi3 (DWtype u, Wtype b)
{
  DWunion w;
  Wtype bm;
  DWunion uu;

  if (b == 0)
    return u;

  uu.ll = u;

  bm = (sizeof (Wtype) * BITS_PER_UNIT) - b;
  if (bm <= 0)
    {
      /* w.s.high = 1..1 or 0..0 */
      w.s.high = uu.s.high >> (sizeof (Wtype) * BITS_PER_UNIT - 1);
      w.s.low = uu.s.high >> -bm;
    }
  else
    {
      UWtype carries = (UWtype) uu.s.high << bm;

      w.s.high = uu.s.high >> b;
      w.s.low = ((UWtype) uu.s.low >> b) | carries;
    }

  return w.ll;
}
symbol_version (___ashrdi3, __ashrdi3, GLIBC_2.0);

#endif
