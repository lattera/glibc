/* setjmp for 64 bit S/390, ELF version.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
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

#include <sysdep.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

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
END (setjmp)
libc_hidden_def (_setjmp)

ENTRY(__setjmp)
        slgr   %r3,%r3              /* Second argument of zero.  */
        j      .Linternal_sigsetjmp /* Branch relativ to __sigsetjmp.  */
END (setjmp)

ENTRY(__sigsetjmp)
.Linternal_sigsetjmp:
        stmg   %r6,%r15,0(%r2)      /* Store registers in jmp_buf.  */
	std    %f1,80(%r2)
	std    %f3,88(%r2)
	std    %f5,96(%r2)
	std    %f7,104(%r2)
#ifdef PIC
        jg     __sigjmp_save@PLT    /* Branch to PLT of __sigsetjmp.  */
#else
	jg     __sigjmp_save
#endif
END (__sigsetjmp)
