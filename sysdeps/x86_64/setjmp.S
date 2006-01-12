/* setjmp for x86-64.
   Copyright (C) 2001, 2003, 2005, 2006 Free Software Foundation, Inc.
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

ENTRY (__sigsetjmp)
	/* Save registers.  */
	movq %rbx, (JB_RBX*8)(%rdi)
#ifdef PTR_MANGLE
	movq %rbp, %rax
	PTR_MANGLE (%rax)
	movq %rax, (JB_RBP*8)(%rdi)
#else
	movq %rbp, (JB_RBP*8)(%rdi)
#endif
	movq %r12, (JB_R12*8)(%rdi)
	movq %r13, (JB_R13*8)(%rdi)
	movq %r14, (JB_R14*8)(%rdi)
	movq %r15, (JB_R15*8)(%rdi)
	leaq 8(%rsp), %rdx	/* Save SP as it will be after we return.  */
#ifdef PTR_MANGLE
	PTR_MANGLE (%rdx)
#endif
	movq %rdx, (JB_RSP*8)(%rdi)
	movq (%rsp), %rax	/* Save PC we are returning to now.  */
#ifdef PTR_MANGLE
	PTR_MANGLE (%rax)
#endif
	movq %rax, (JB_PC*8)(%rdi)

#if defined NOT_IN_libc && defined IS_IN_rtld
	/* In ld.so we never save the signal mask.  */
	xorl %eax, %eax
	retq
#else
	/* Make a tail call to __sigjmp_save; it takes the same args.  */
# ifdef	PIC
	jmp C_SYMBOL_NAME (BP_SYM (__sigjmp_save))@PLT
# else
	jmp BP_SYM (__sigjmp_save)
# endif
#endif
END (BP_SYM (__sigsetjmp))
hidden_def (__sigsetjmp)
