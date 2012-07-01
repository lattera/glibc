/* Macros to swap the order of bytes in integer values.  m68k version.
   Copyright (C) 1997, 2002, 2008, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H && !defined _ENDIAN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

/* Swap bytes in 16 bit value.  We don't provide an assembler version
   because GCC is smart enough to generate optimal assembler output, and
   this allows for better cse.  */
#define __bswap_constant_16(x) \
  ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8))

static __inline unsigned short int
__bswap_16 (unsigned short int __bsx)
{
  return __bswap_constant_16 (__bsx);
}

/* Swap bytes in 32 bit value.  */
#define __bswap_constant_32(x) \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >>  8) |		      \
   (((x) & 0x0000ff00u) <<  8) | (((x) & 0x000000ffu) << 24))

#if !defined(__mcoldfire__)
static __inline unsigned int
__bswap_32 (unsigned int __bsx)
{
  if (__builtin_constant_p (__bsx))
    return __bswap_constant_32 (__bsx);
  __asm__ __volatile__ ("ror%.w %#8, %0;"
			"swap %0;"
			"ror%.w %#8, %0"
			: "+d" (__bsx));
  return __bsx;
}
#else
static __inline unsigned int
__bswap_32 (unsigned int __bsx)
{
  return __bswap_constant_32 (__bsx);
}
#endif

#if defined __GNUC__ && __GNUC__ >= 2
/* Swap bytes in 64 bit value.  */
# define __bswap_constant_64(x) \
  __extension__								      \
  ((((x) & 0xff00000000000000ull) >> 56)				      \
   | (((x) & 0x00ff000000000000ull) >> 40)				      \
   | (((x) & 0x0000ff0000000000ull) >> 24)				      \
   | (((x) & 0x000000ff00000000ull) >> 8)				      \
   | (((x) & 0x00000000ff000000ull) << 8)				      \
   | (((x) & 0x0000000000ff0000ull) << 24)				      \
   | (((x) & 0x000000000000ff00ull) << 40)				      \
   | (((x) & 0x00000000000000ffull) << 56))

/* Swap bytes in 64 bit value.  */
static __inline unsigned long long
__bswap_64 (unsigned long long __bsx)
{
  if (__builtin_constant_p (__bsx))
    return __bswap_constant_64 (__bsx);
  return (__bswap_32 (__bsx >> 32)
	  | ((unsigned long long) __bswap_32 (__bsx) << 32));
}
#endif

#endif /* _BITS_BYTESWAP_H */
