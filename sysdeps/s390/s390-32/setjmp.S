/*
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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
/* R2 = pointer to jmp_buf, R3 = savemask */

ENTRY(__sigsetjmp)
	stm    %r6,%r15,0(%r2)      /* store registers in jmp_buf */
	std    %f4,40(%r2)
	std    %f6,48(%r2)
#ifdef PIC
	/* We cannot use the PLT, because it requires that %r12 be set, but
	   we can't save and restore our caller's value.  Instead, we do an
	   indirect jump through the GOT. */
	basr   %r1,0
.L0:    al     %r1,.L1 - .L0(0,%r1) /* get address of global offset table */
				    /* get address of __sigjmp_save from got */
	l      %r1,__sigjmp_save@GOT12(0,%r1)
	br     %r1
.L1:    .long  _GLOBAL_OFFSET_TABLE_ - .L0
#else
	basr   %r1,0
.L0:    l      %r1,.L1-.L0(0,%r1)   /* load address of __sigjmp_save */
	br     %r1                  /* tail-call __sigjmp_save */
.L1:	.long  __sigjmp_save
#endif
END (__sigsetjmp)
