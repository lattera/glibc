/* 64-bit integer comparison.
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

#include <stdlib/longlong.h>

#if __BYTE_ORDER == __BIG_ENDIAN
struct DWstruct { Wtype high, low;};
#elif __BYTE_ORDER == __LITTLE_ENDIAN
struct DWstruct { Wtype low, high;};
#else
#error Unhandled endianity
#endif
typedef union { struct DWstruct s; DWtype ll; } DWunion;


Wtype
___cmpdi2 (DWtype a, DWtype b)
{
  DWunion au, bu;

  au.ll = a, bu.ll = b;

  if (au.s.high < bu.s.high)
    return 0;
  else if (au.s.high > bu.s.high)
    return 2;
  if ((UWtype) au.s.low < (UWtype) bu.s.low)
    return 0;
  else if ((UWtype) au.s.low > (UWtype) bu.s.low)
    return 2;
  return 1;
}
symbol_version (___cmpdi2, __cmpdi2, GLIBC_2.0);


Wtype
___ucmpdi2 (DWtype a, DWtype b)
{
  DWunion au, bu;

  au.ll = a, bu.ll = b;

  if ((UWtype) au.s.high < (UWtype) bu.s.high)
    return 0;
  else if ((UWtype) au.s.high > (UWtype) bu.s.high)
    return 2;
  if ((UWtype) au.s.low < (UWtype) bu.s.low)
    return 0;
  else if ((UWtype) au.s.low > (UWtype) bu.s.low)
    return 2;
  return 1;
}
symbol_version (___ucmpdi2, __ucmpdi2, GLIBC_2.0);

#endif
