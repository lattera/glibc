/* Macros to swap the order of bytes in integer values.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

#define __bswap_constant_16(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

/* Swap bytes in 16 bit value. */
#if defined __GNUC__ && __GNUC__ >= 2
# define __bswap_16(x) \
     (__extension__							      \
      ({ unsigned short int __v; 		                              \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_16 (x);				      \
	 else {								      \
           unsigned short int __tmp = (unsigned short int) (x);               \
           __asm__ __volatile__ (                                             \
              "lrvh %0,%1"                                                    \
              : "=&d" (__v) : "m" (__tmp) );                                  \
         }                                                                    \
	 __v; }))
#else
/* This is better than nothing.  */
#define __bswap_16(x) __bswap_constant_16 (x)
#endif

/* Swap bytes in 32 bit value.  */
#define __bswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |		      \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#if defined __GNUC__ && __GNUC__ >= 2
#  define __bswap_32(x) \
     (__extension__							      \
      ({ unsigned int __v;					              \
	 if (__builtin_constant_p (x))					      \
	   __v = __bswap_constant_32 (x);				      \
	 else {								      \
           unsigned int __tmp = (unsigned int) (x);                           \
           __asm__ __volatile__ (                                             \
              "lrv   %0,%1"                                                   \
              : "=&d" (__v) : "m" (__tmp));                                   \
         }                                                                    \
	 __v; }))
#else
# define __bswap_32(x) __bswap_constant_32 (x)
#endif

/* Swap bytes in 64 bit value.  */
#define __bswap_constant_64(x) \
     ((((x)&0xff00000000000000) >> 56) | (((x)&0x00ff000000000000) >> 40) |  \
      (((x)&0x0000ff0000000000) >> 24) | (((x)&0x000000ff00000000) >>  8) |  \
      (((x)&0x00000000ff000000) <<  8) | (((x)&0x0000000000ff0000) << 24) |  \
      (((x)&0x000000000000ff00) << 40) | (((x)&0x00000000000000ff) << 56))

#if defined __GNUC__ && __GNUC__ >= 2
#  define __bswap_64(x) \
     (__extension__							      \
      ({ unsigned long __w;					              \
	 if (__builtin_constant_p (x))					      \
	   __w = __bswap_constant_64 (x);				      \
	 else {								      \
           unsigned long __tmp = (unsigned long) (x);                         \
           __asm__ __volatile__ (                                             \
              "lrvg  %0,%1"                                                   \
              : "=&d" (__w) : "m" (__tmp));                                   \
         }                                                                    \
	 __w; }))
#else
# define __bswap_64(x) __bswap_constant_64 (x)
#endif


