/* Convert double to 64-bit int.
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
typedef         float SFtype	__attribute__ ((mode (SF)));
typedef         float DFtype	__attribute__ ((mode (DF)));
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

#define WORD_SIZE __WORDSIZE
#define HIGH_WORD_COEFF (((UDWtype) 1) << WORD_SIZE)

DWtype
___fixunsdfdi (DFtype a)
{
  DFtype b;
  UDWtype v;

  if (a < 0)
    return 0;

  /* Compute high word of result, as a flonum.  */
  b = (a / HIGH_WORD_COEFF);
  /* Convert that to fixed (but not to DWtype!),
     and shift it into the high word.  */
  v = (UWtype) b;
  v <<= WORD_SIZE;
  /* Remove high part from the DFtype, leaving the low part as flonum.  */
  a -= (DFtype)v;
  /* Convert that to fixed (but not to DWtype!) and add it in.
     Sometimes A comes out negative.  This is significant, since
     A has more bits than a long int does.  */
  if (a < 0)
    v -= (UWtype) (- a);
  else
    v += (UWtype) a;
  return v;
}
symbol_version (___fixunsdfdi, __fixunsdfdi, GLIBC_2.0);


DWtype
___fixdfdi (DFtype a)
{
  if (a < 0)
    return - __fixunsdfdi (-a);
  return __fixunsdfdi (a);
}
symbol_version (___fixdfdi, __fixdfdi, GLIBC_2.0);

#endif
