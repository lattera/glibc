/* Multiple versions of memchr
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2013-2017 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <rtld-global-offsets.h>

#if IS_IN (libc)
/* Under __ARM_NEON__, memchr_neon.S defines the name memchr.  */
# ifndef __ARM_NEON__
	.text
	.arm
ENTRY(memchr)
	.type	memchr, %gnu_indirect_function
	ldr	r1, .Lmemchr_noneon
	tst	r0, #HWCAP_ARM_NEON
	ldrne	r1, .Lmemchr_neon
1:
	add	r0, r1, pc
	DO_RET(lr)

.Lmemchr_noneon:
	.long	C_SYMBOL_NAME(__memchr_noneon) - 1b - 8
.Lmemchr_neon:
	.long	C_SYMBOL_NAME(__memchr_neon) - 1b - 8

END(memchr)

libc_hidden_builtin_def (memchr)
# endif  /* Not __ARM_NEON__.  */
libc_hidden_def (__memchr_noneon)

# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# undef weak_alias
# define weak_alias(x, y)
# undef libc_hidden_def
# define libc_hidden_def(name)

# define memchr __memchr_noneon

#endif

#include "memchr_impl.S"
