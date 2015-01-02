/* setjmp for 64 bit S/390, ELF version.
   Copyright (C) 2001-2015 Free Software Foundation, Inc.
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

#include <sysdep.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>
#include <shlib-compat.h>
#include <stap-probe.h>

#if !IS_IN (rtld)
# if defined SHARED &&  SHLIB_COMPAT (libc, GLIBC_2_19, GLIBC_2_20)
	/* we need a unique name in case of symbol versioning.  */
#  define __sigsetjmp __v1__sigsetjmp
# endif /* if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_19, GLIBC_2_20)  */
#endif /* !IS_IN (rtld)  */

	/* We include the BSD entry points here as well but we make
	   them weak.  */
ENTRY (setjmp)
	.weak C_SYMBOL_NAME (setjmp)
        lghi   %r3,1                /* Second argument of one.  */
        j      .Linternal_sigsetjmp /* Branch relativ to __sigsetjmp.  */
END (setjmp)

	/* Binary compatibility entry point.  */
ENTRY(_setjmp)
        .weak  C_SYMBOL_NAME (_setjmp)
        slgr   %r3,%r3              /* Second argument of zero.  */
        j      .Linternal_sigsetjmp /* Branch relativ to __sigsetjmp.  */
END (_setjmp)
libc_hidden_def (_setjmp)

ENTRY(__setjmp)
        slgr   %r3,%r3              /* Second argument of zero.  */
        j      .Linternal_sigsetjmp /* Branch relativ to __sigsetjmp.  */
END (__setjmp)

ENTRY(__sigsetjmp)
.Linternal_sigsetjmp:
	/* setjmp probe expects sig/setjmp first argument (8@%r2), second
	   argument (-8@%r3) and target address (8@%r14).  */
	LIBC_PROBE (setjmp, 3, 8@%r2, -4@%r3, 8@%r14)
#ifdef PTR_MANGLE
	stmg   %r6,%r13,0(%r2)      /* Store registers in jmp_buf.  */
	lgr    %r4,%r14
	lgr    %r5,%r15
	PTR_MANGLE (%r4, %r1)
	PTR_MANGLE2 (%r5, %r1)
	stmg   %r4,%r5,64(%r2)
#else
        stmg   %r6,%r15,0(%r2)      /* Store registers in jmp_buf.  */
#endif
	std    %f8,80(%r2)
	std    %f9,88(%r2)
	std    %f10,96(%r2)
	std    %f11,104(%r2)
	std    %f12,112(%r2)
	std    %f13,120(%r2)
	std    %f14,128(%r2)
	std    %f15,136(%r2)
#if IS_IN (rtld)
	/* In ld.so we never save the signal mask.  */
	lghi   %r2,0
	br     %r14
#elif defined PIC
        jg     __sigjmp_save@PLT    /* Branch to PLT of __sigsetjmp.  */
#else
	jg     __sigjmp_save
#endif
END (__sigsetjmp)

#if !IS_IN (rtld)
# if defined SHARED &&  SHLIB_COMPAT (libc, GLIBC_2_19, GLIBC_2_20)
/* In glibc release 2.19 new versions of setjmp-functions were introduced,
   but were reverted before 2.20. Thus both versions are the same function.  */

#  undef __sigsetjmp

weak_alias (setjmp, __v1setjmp);
weak_alias (setjmp, __v2setjmp);
versioned_symbol (libc, __v1setjmp, setjmp, GLIBC_2_0);
compat_symbol (libc, __v2setjmp, setjmp, GLIBC_2_19);

weak_alias (_setjmp, __v1_setjmp);
weak_alias (_setjmp, __v2_setjmp);
versioned_symbol (libc, __v1_setjmp, _setjmp, GLIBC_2_0);
compat_symbol (libc, __v2_setjmp, _setjmp, GLIBC_2_19);

strong_alias (__v1__sigsetjmp, __v2__sigsetjmp);
versioned_symbol (libc, __v1__sigsetjmp, __sigsetjmp, GLIBC_2_0);
compat_symbol (libc, __v2__sigsetjmp, __sigsetjmp, GLIBC_2_19);
# endif /* if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_19, GLIBC_2_20)  */
#endif /* if !IS_IN (rtld)  */
