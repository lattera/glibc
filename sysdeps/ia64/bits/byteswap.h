/* Macros to swap the order of bytes in integer values.
   Copyright (C) 1997, 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

/* Swap bytes in 16 bit value.  */
#define __bswap_constant_16(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

#if defined __GNUC__ && __GNUC__ >= 2
# define __bswap_16(x) \
     (__extension__							      \
      ({ register unsigned short int __v, __x = (x);			      \
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


/* Swap bytes in 32 bit value.  */
#define __bswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |		      \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#if defined __GNUC__ && __GNUC__ >= 2
# define __bswap_32(x) \
     (__extension__							      \
      ({ register unsigned int __v, __x = (x);				      \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_32 (__x);				      \
	 else								      \
	   __asm__ __volatile__ ("shl %0 = %1, 32 ;;"			      \
				 "mux1 %0 = %0, @rev ;;"		      \
				 : "=r" (__v)				      \
				 : "r" ((unsigned int) (__x)));		      \
	 __v; }))
#else
static __inline unsigned int
__bswap_32 (unsigned int __bsx)
{
  return __bswap_constant_32 (__bsx);
}
#endif


/* Swap bytes in 64 bit value.  */
#define __bswap_constant_64(x) \
     ((((x) & 0xff00000000000000ul) >> 56)				      \
      | (((x) & 0x00ff000000000000ul) >>  40)				      \
      | (((x) & 0x0000ff0000000000ul) >> 24)				      \
      | (((x) & 0x000000ff00000000ul) >> 8)				      \
      | (((x) & 0x00000000ff000000ul) << 8)				      \
      | (((x) & 0x0000000000ff0000ul) << 24)				      \
      | (((x) & 0x000000000000ff00ul) << 40)				      \
      | (((x) & 0x00000000000000fful) << 56))

#if defined __GNUC__ && __GNUC__ >= 2
# define __bswap_64(x) \
     (__extension__							      \
      ({ register unsigned long int __v, __x = (x);			      \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_64 (__x);				      \
	 else								      \
	   __asm__ __volatile__ ("mux1 %0 = %1, @rev ;;"		      \
				 : "=r" (__v)				      \
				 : "r" ((unsigned long int) (__x)));	      \
         __v; }))

#else
static __inline unsigned long int
__bswap_64 (unsigned long int __bsx)
{
  return __bswap_constant_64 (__bsx);
}
#endif

#endif /* _BITS_BYTESWAP_H */
