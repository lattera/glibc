/* setjmp for i386.
   Copyright (C) 1995,1996,1997,2000,2001,2005,2006
	Free Software Foundation, Inc.
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
#include <jmpbuf-offsets.h>
#include <asm-syntax.h>
#include "bp-sym.h"
#include "bp-asm.h"

#define PARMS	LINKAGE		/* no space for saved regs */
#define JMPBUF	PARMS
#define SIGMSK	JMPBUF+PTR_SIZE

ENTRY (BP_SYM (__sigsetjmp))
	ENTER

	movl JMPBUF(%esp), %eax
	CHECK_BOUNDS_BOTH_WIDE (%eax, JMPBUF(%esp), $JB_SIZE)

     	/* Save registers.  */
	movl %ebx, (JB_BX*4)(%eax)
	movl %esi, (JB_SI*4)(%eax)
	movl %edi, (JB_DI*4)(%eax)
	leal JMPBUF(%esp), %ecx	/* Save SP as it will be after we return.  */
#ifdef PTR_MANGLE
	PTR_MANGLE (%ecx)
#endif
     	movl %ecx, (JB_SP*4)(%eax)
	movl PCOFF(%esp), %ecx	/* Save PC we are returning to now.  */
#ifdef PTR_MANGLE
	PTR_MANGLE (%ecx)
#endif
     	movl %ecx, (JB_PC*4)(%eax)
	LEAVE /* pop frame pointer to prepare for tail-call.  */
	movl %ebp, (JB_BP*4)(%eax) /* Save caller's frame pointer.  */

#if defined NOT_IN_libc && defined IS_IN_rtld
	/* In ld.so we never save the signal mask.  */
	xorl %eax, %eax
	ret
#else
	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	jmp __sigjmp_save
#endif
END (BP_SYM (__sigsetjmp))
