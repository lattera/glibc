/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  i386 version.
   Copyright (C) 1995-1997,2000,2001,2002,2003 Free Software Foundation, Inc.
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
#include "bp-sym.h"
#include "bp-asm.h"

#define PARMS	LINKAGE		/* no space for saved regs */
#define JMPBUF	PARMS
#define SIGMSK	JMPBUF+PTR_SIZE

ENTRY (BP_SYM (setjmp))
	/* Note that we have to use a non-exported symbol in the next
	   jump since otherwise gas will emit it as a jump through the
	   PLT which is what we cannot use here.  */
	ENTER

	movl JMPBUF(%esp), %eax
	CHECK_BOUNDS_BOTH_WIDE (%eax, JMPBUF(%esp), $JB_SIZE)

     	/* Save registers.  */
	movl %ebx, (JB_BX*4)(%eax)
	movl %esi, (JB_SI*4)(%eax)
	movl %edi, (JB_DI*4)(%eax)
	leal JMPBUF(%esp), %ecx	/* Save SP as it will be after we return.  */
     	movl %ecx, (JB_SP*4)(%eax)
	movl PCOFF(%esp), %ecx	/* Save PC we are returning to now.  */
     	movl %ecx, (JB_PC*4)(%eax)
	LEAVE /* pop frame pointer to prepare for tail-call.  */
	movl %ebp, (JB_BP*4)(%eax) /* Save caller's frame pointer.  */

	/* Call __sigjmp_save.  */
	pushl $1
	pushl 8(%esp)
#ifdef	PIC
	/* We cannot use the PLT, because it requires that %ebx be set, but
           we can't save and restore our caller's value.  Instead, we do an
           indirect jump through the GOT, using for the temporary register
           %ecx, which is call-clobbered.  */
	call __i686.get_pc_thunk.cx
	addl $_GLOBAL_OFFSET_TABLE_, %ecx
	leal C_SYMBOL_NAME (BP_SYM (__sigjmp_save)@GOTOFF)(%ecx), %ecx
	call *%ecx
#else
	call BP_SYM (__sigjmp_save)
#endif
	popl %ecx
	popl %edx
	ret
END (BP_SYM (setjmp))

	.section .gnu.linkonce.t.__i686.get_pc_thunk.cx,"ax",@progbits
	.globl __i686.get_pc_thunk.cx
	.hidden __i686.get_pc_thunk.cx
	.type __i686.get_pc_thunk.cx,@function
__i686.get_pc_thunk.cx:
	movl (%esp), %ecx
	ret
	.size __i686.get_pc_thunk.cx, . - __i686.get_pc_thunk.cx
