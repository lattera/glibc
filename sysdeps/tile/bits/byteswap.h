/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H && !defined _ENDIAN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

/* gcc __builtin_bswap64() can constant-fold, etc, so always use it. */
#define __bswap_16(x) ((unsigned short)(__builtin_bswap32(x) >> 16))
#define __bswap_32(x) ((unsigned int)__builtin_bswap32(x))
#define __bswap_64(x) ((unsigned long long)__builtin_bswap64(x))

#define __bswap_constant_16(x) __bswap_16(x)
#define __bswap_constant_32(x) __bswap_32(x)
#define __bswap_constant_64(x) __bswap_64(x)

#endif /* _BITS_BYTESWAP_H */
