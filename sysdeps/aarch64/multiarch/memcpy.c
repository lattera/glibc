/* Multiple versions of memcpy. AARCH64 version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in libc.  */

#if IS_IN (libc)
/* Redefine memcpy so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef memcpy
# define memcpy __redirect_memcpy
# include <string.h>
# include <init-arch.h>

extern __typeof (__redirect_memcpy) __libc_memcpy;

extern __typeof (__redirect_memcpy) __memcpy_generic attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_thunderx attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_falkor attribute_hidden;

libc_ifunc (__libc_memcpy,
            (IS_THUNDERX (midr)
	     ? __memcpy_thunderx
	     : (IS_FALKOR (midr)
		? __memcpy_falkor
		: __memcpy_generic)));

# undef memcpy
strong_alias (__libc_memcpy, memcpy);
#endif
