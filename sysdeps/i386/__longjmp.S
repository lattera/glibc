/* longjmp for i386.
   Copyright (C) 1995,1996,1997,1998,2000,2002 Free Software Foundation, Inc.
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
#include <asm-syntax.h>
#include <bp-sym.h>
#include <bp-asm.h>

#define PARMS	LINKAGE		/* no space for saved regs */
#define JBUF	PARMS
#define VAL	JBUF+PTR_SIZE

ENTRY (BP_SYM (__longjmp))
	ENTER

	movl JBUF(%esp), %ecx	/* User's jmp_buf in %ecx.  */
	CHECK_BOUNDS_BOTH_WIDE (%ecx, JBUF(%esp), $JB_SIZE)

	movl VAL(%esp), %eax	/* Second argument is return value.  */
	/* Save the return address now.  */
	movl (JB_PC*4)(%ecx), %edx
     	/* Restore registers.  */
	movl (JB_BX*4)(%ecx), %ebx
	movl (JB_SI*4)(%ecx), %esi
	movl (JB_DI*4)(%ecx), %edi
	movl (JB_BP*4)(%ecx), %ebp
	movl (JB_SP*4)(%ecx), %esp
	/* Jump to saved PC.  */
     	jmp *%edx
END (BP_SYM (__longjmp))
