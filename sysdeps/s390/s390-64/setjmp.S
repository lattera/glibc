/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#ifndef _ASM
#define _ASM
#endif

#include <sysdep.h>
#include <bits/setjmp.h>

/* Save the current program position in ENV and return 0.  */
/* R2 = pointer to jmp_buf, R3 = savemask.  */

ENTRY(__sigsetjmp)
        stmg   %r6,%r15,0(%r2)      /* Store registers in jmp_buf.  */
	std    %f1,80(%r2)
	std    %f3,88(%r2)
	std    %f5,96(%r2)
	std    %f7,104(%r2)
#ifdef PIC
        jg     __sigjmp_save@PLT    /* Tail-call __sigjmp_save.  */
#else
        jg     __sigjmp_save        /* Tail-call __sigjmp_save.  */
#endif
END (__sigsetjmp)


