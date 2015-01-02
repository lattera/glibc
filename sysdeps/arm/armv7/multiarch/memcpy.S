/* Multiple versions of memcpy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

/* Thumb requires excess IT instructions here.  */
#define NO_THUMB
#include <sysdep.h>
#include <rtld-global-offsets.h>

#if IS_IN (libc)
/* Under __ARM_NEON__, memcpy_neon.S defines the name memcpy.  */
# ifndef __ARM_NEON__
	.text
ENTRY(memcpy)
	.type	memcpy, %gnu_indirect_function
# ifdef __SOFTFP__
	ldr	r1, .Lmemcpy_arm
	tst	r0, #HWCAP_ARM_VFP
	ldrne	r1, .Lmemcpy_vfp
# else
	ldr	r1, .Lmemcpy_vfp
# endif
	tst	r0, #HWCAP_ARM_NEON
	ldrne	r1, .Lmemcpy_neon
1:
	add	r0, r1, pc
	DO_RET(lr)

# ifdef __SOFTFP__
.Lmemcpy_arm:
	.long	C_SYMBOL_NAME(__memcpy_arm) - 1b - PC_OFS
# endif
.Lmemcpy_neon:
	.long	C_SYMBOL_NAME(__memcpy_neon) - 1b - PC_OFS
.Lmemcpy_vfp:
	.long	C_SYMBOL_NAME(__memcpy_vfp) - 1b - PC_OFS

END(memcpy)

libc_hidden_builtin_def (memcpy)
#endif  /* Not __ARM_NEON__.  */

/* These versions of memcpy are defined not to clobber any VFP or NEON
   registers so they must always call the ARM variant of the memcpy code.  */
strong_alias (__memcpy_arm, __aeabi_memcpy)
strong_alias (__memcpy_arm, __aeabi_memcpy4)
strong_alias (__memcpy_arm, __aeabi_memcpy8)
libc_hidden_def (__memcpy_arm)

#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#undef weak_alias
#define weak_alias(x, y)
#undef libc_hidden_def
#define libc_hidden_def(name)

#define memcpy __memcpy_arm

#endif

#include "memcpy_impl.S"
