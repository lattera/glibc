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

	.section .rodata.str1.1,"aMS",@progbits,1
	.type	longjmp_msg,@object
longjmp_msg:
	.string "longjmp causes uninitialized stack frame"
	.size	longjmp_msg, .-longjmp_msg


#define __longjmp ____longjmp_chk

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

#include "__longjmp.S"
