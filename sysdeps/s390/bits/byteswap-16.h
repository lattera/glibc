/* Macros to swap the order of bytes in 16-bit integer values.  s390 version
   Copyright (C) 2012 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_BYTESWAP_H
# error "Never use <bits/byteswap-16.h> directly; include <byteswap.h> instead."
#endif

#include <bits/wordsize.h>

/* Swap bytes in 16 bit value. */
#if defined __GNUC__ && __GNUC__ >= 2
# if __WORDSIZE == 64
#  define __bswap_16(x) \
     (__extension__							      \
      ({ unsigned short int __v, __x = (x);				      \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_16 (__x);				      \
	 else {								      \
	   unsigned short int __tmp = (unsigned short int) (__x);             \
	   __asm__ __volatile__ (                                             \
	      "lrvh %0,%1"                                                    \
	      : "=&d" (__v) : "m" (__tmp) );                                  \
	 }                                                                    \
	 __v; }))
# else
#  define __bswap_16(x) \
     (__extension__							      \
      ({ unsigned short int __v, __x = (x);				      \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_16 (__x);				      \
	 else {								      \
	   unsigned short int __tmp = (unsigned short int) (__x);             \
	   __asm__ __volatile__ (                                             \
	      "sr   %0,%0\n"                                                  \
	      "la   1,%1\n"                                                   \
	      "icm  %0,2,1(1)\n"                                              \
	      "ic   %0,0(1)"                                                  \
	      : "=&d" (__v) : "m" (__tmp) : "1");                             \
	 }                                                                    \
	 __v; }))
# endif
#else
/* This is better than nothing.  */
static __inline unsigned short int
__bswap_16 (unsigned short int __bsx)
{
  return __bswap_constant_16 (__bsx);
}
#endif
