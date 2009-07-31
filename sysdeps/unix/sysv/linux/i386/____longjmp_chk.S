/* Copyright (C) 2001,2004,2005,2006,2009 Free Software Foundation, Inc.
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


	.section .rodata.str1.1,"aMS",@progbits,1
	.type	longjmp_msg,@object
longjmp_msg:
	.string "longjmp causes uninitialized stack frame"
	.size	longjmp_msg, .-longjmp_msg


#ifdef PIC
# define CALL_FAIL	movl	%ebx, %ecx;				      \
			cfi_register(%ebx,%ecx);			      \
			LOAD_PIC_REG (bx);				      \
			leal	longjmp_msg@GOTOFF(%ebx), %eax;		      \
			call	__GI___fortify_fail@PLT
#else
# define CALL_FAIL	movl	$longjmp_msg, %eax;			      \
			call	__fortify_fail
#endif

#define CHECK_ESP(reg) \
	cmpl	reg, %esp;						      \
	jbe	.Lok;							      \
	CALL_FAIL;							      \
.Lok:


	.text
ENTRY (____longjmp_chk)
	movl	4(%esp), %ecx	/* User's jmp_buf in %ecx.  */

	/* Save the return address now.  */
	movl	(JB_PC*4)(%ecx), %edx
	/* Get the stack pointer.  */
	movl	(JB_SP*4)(%ecx), %edi
	cfi_undefined(%edi)
	PTR_DEMANGLE (%edx)
	PTR_DEMANGLE (%edi)

	pushl	$0
	cfi_adjust_cfa_offset(4)
	cmpl	%edi, %esp
	jbe	.Lok

	subl	$12, %esp
	cfi_adjust_cfa_offset(12)
	xorl	%ebx, %ebx
	movl	%esp, %ecx
	movl	$__NR_sigaltstack, %eax
	ENTER_KERNEL
	movl	4(%esp), %ebx
	addl	$12, %esp
	cfi_adjust_cfa_offset(-12)
	movl	8(%esp), %ecx
	testl	%eax, %eax
	jne	.Lok
	andl	$1, %ebx
	movl	%ebx, (%esp)
.Lok:
	cfi_def_cfa(%ecx, 0)
	cfi_register(%eip, %edx)
	cfi_register(%esp, %edi)
	cfi_offset(%ebx, JB_BX*4)
	cfi_offset(%esi, JB_SI*4)
	cfi_offset(%edi, JB_DI*4)
	cfi_offset(%ebp, JB_BP*4)

	movl	12(%esp), %eax	/* Second argument is return value.  */
	xchgl	%edi, %esp
	cfi_restore(%edi)

	cmpl	%esp, %edi
	jnbe	.Lcheck

     	/* Restore registers.  */
.Lout:	movl	(JB_BX*4)(%ecx), %ebx
	movl	(JB_SI*4)(%ecx), %esi
	movl	(JB_DI*4)(%ecx), %edi
	movl	(JB_BP*4)(%ecx), %ebp
	cfi_restore(%ebx)
	cfi_restore(%esi)
	cfi_restore(%edi)
	cfi_restore(%ebp)

	/* Jump to saved PC.  */
     	jmp	*%edx

	cfi_def_cfa(%ecx, 0)
	cfi_register(%eip, %edx)
	cfi_offset(%ebx, JB_BX*4)
	cfi_offset(%esi, JB_SI*4)
	cfi_offset(%edi, JB_DI*4)
	cfi_offset(%ebp, JB_BP*4)

.Lcheck:
	cmpl	$0, (%edi)
	je	.Lfail

	subl	$12, %esp
	cfi_adjust_cfa_offset(12)
	xorl	%ebx, %ebx
	movl	%esp, %ecx
	movl	$__NR_sigaltstack, %eax
	ENTER_KERNEL
	testl	$1, 4(%esp)
	leal	12(%esp), %esp
	movl	8(%edi), %ecx
	movl	12(%edi), %eax
	cfi_adjust_cfa_offset(-12)
	je	.Lout

.Lfail:	xchgl	%edi, %esp
	cfi_def_cfa(%esp, 8)
	cfi_restore(%esp)
	cfi_restore(%ebx)
	cfi_restore(%esi)
	cfi_undefined(%edi)
	cfi_restore(%ebp)

	CALL_FAIL
	hlt
END (____longjmp_chk)
