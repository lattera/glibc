/* Initialization code run first thing by the ELF startup code.
   For i386/Linux.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

	.text

	/* Called from start.S.  Not: there is no value in %ebx.  */

ENTRY (__libc_init_first)

	/* Make sure we are not using iBSC2 personality.  */
	movl $SYS_ify (personality), %eax
	xorl %ebx, %ebx
	int $0x80

#ifdef PIC
	/* Set control work of FPU.  */
	call .L0
.L0:	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L0], %ebx
	movzwl C_SYMBOL_NAME(___fpu_control@GOT)(%ebx), %eax
#else
	movzwl ___fpu_control, %eax
#endif
	pushl %eax
	call JUMPTARGET(__setfpucw)
	addl $4, %esp

	/* That is all for now.  */
	ret


	/* This is only a dummy function for the list below.  */
	.type _dummy_exit, @function
C_LABEL(_dummy_exit)
	ret


	.section .init,"ax",@progbits
	movl 16(%ebp), %edx	/* envp */
	movl 12(%ebp), %ecx	/* argv */
	movl 8(%ebp), %eax	/* argc */
	pushl %edx
	pushl %ecx
	pushl %eax

	call JUMPTARGET(__libc_init)

	addl $12, %esp


	/* Make sure __libc_subinit section is always present.  */
	.section __libc_subinit, "a", @progbits
	.align 4
	.type __elf_set___libc_subinit_element__dummy_exit__, @object
	.size __elf_set___libc_subinit_element__dummy_exit__, 4
__elf_set___libc_subinit_element__dummy_exit__:
	.long _dummy_exit

	/* Make sure __libc_atexit section is always present.  */
	.section __libc_atexit, "a", @progbits
	.align 4
	.type __elf_set___libc_atexit_element__dummy_exit__, @object
	.size __elf_set___libc_atexit_element__dummy_exit__, 4
__elf_set___libc_atexit_element__dummy_exit__:
	.long _dummy_exit
