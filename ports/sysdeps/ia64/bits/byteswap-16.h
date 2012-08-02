/* Macros to swap the order of bytes in 16-bit integer values.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
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

#if defined __GNUC__ && __GNUC__ >= 2
# define __bswap_16(x) \
     (__extension__							      \
      ({ register unsigned short int __v, __x = (unsigned short int) (x);     \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_16 (__x);				      \
	 else								      \
	   __asm__ __volatile__ ("shl %0 = %1, 48 ;;"			      \
				 "mux1 %0 = %0, @rev ;;"		      \
				 : "=r" (__v)				      \
				 : "r" ((unsigned short int) (__x)));	      \
	 __v; }))
#else
/* This is better than nothing.  */
static __inline unsigned short int
__bswap_16 (unsigned short int __bsx)
{
  return __bswap_constant_16 (__bsx);
}
#endif
