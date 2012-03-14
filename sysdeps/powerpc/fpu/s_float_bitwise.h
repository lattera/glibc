/* Bitwise manipulation over float. Function prototypes.
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Adhemerval Zanella <azanella@br.ibm.com>, 2011

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _FLOAT_BITWISE_
#define _FLOAT_BITWISE_ 1

#include <math_private.h>

/* Returns (int)(num & 0x7FFFFFF0 == value) */
static inline
int __float_and_test28 (float num, float value)
{
  float ret;
#ifdef _ARCH_PWR7
  vector int mask = (vector int) {
    0x7ffffffe, 0x00000000, 0x00000000, 0x0000000
  };
  __asm__ (
  /* the 'f' constrain is use on mask because we just need
   * to compare floats, not full vector */
    "xxland %x0,%x1,%x2" : "=f" (ret) : "f" (num), "f" (mask)
  );
#else
  int32_t inum;
  GET_FLOAT_WORD(inum, num);
  inum = (inum & 0x7ffffff0);
  SET_FLOAT_WORD(ret, inum);
#endif
  return (ret == value);
}

/* Returns (int)(num & 0x7FFFFF00 == value) */
static inline
int __float_and_test24 (float num, float value)
{
  float ret;
#ifdef _ARCH_PWR7
  vector int mask = (vector int) {
    0x7fffffe0, 0x00000000, 0x00000000, 0x0000000
  };
  __asm__ (
    "xxland %x0,%x1,%x2" : "=f" (ret) : "f" (num), "f" (mask)
  );
#else
  int32_t inum;
  GET_FLOAT_WORD(inum, num);
  inum = (inum & 0x7fffff00);
  SET_FLOAT_WORD(ret, inum);
#endif
  return (ret == value);
}

/* Returns (float)(num & 0x7F800000) */
static inline
float __float_and8 (float num)
{
  float ret;
#ifdef _ARCH_PWR7
  vector int mask = (vector int) {
    0x7ff00000, 0x00000000, 0x00000000, 0x00000000
  };
  __asm__ (
    "xxland %x0,%x1,%x2" : "=f" (ret) : "f" (num), "f" (mask)
  );
#else
  int32_t inum;
  GET_FLOAT_WORD(inum, num);
  inum = (inum & 0x7f800000);
  SET_FLOAT_WORD(ret, inum);
#endif
  return ret;
}

/* Returns ((int32_t)(num & 0x7F800000) >> 23) */
static inline
int32_t __float_get_exp (float num)
{
  int32_t inum;
#ifdef _ARCH_PWR7
  float ret;
  vector int mask = (vector int) {
    0x7ff00000, 0x00000000, 0x00000000, 0x00000000
  };
  __asm__ (
    "xxland %x0,%x1,%x2" : "=f" (ret) : "f" (num), "f" (mask)
  );
  GET_FLOAT_WORD(inum, ret);
#else
  GET_FLOAT_WORD(inum, num);
  inum = inum & 0x7f800000;
#endif
  return inum >> 23;
}

#endif /* s_float_bitwise.h */
