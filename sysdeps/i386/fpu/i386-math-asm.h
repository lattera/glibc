/* Helper macros for x86 libm functions.
   Copyright (C) 2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _I386_MATH_ASM_H
#define _I386_MATH_ASM_H 1

/* Remove excess range and precision by storing a value on the stack
   and loading it back.  */
#define FLT_NARROW_EVAL				\
	subl	$4, %esp;			\
	cfi_adjust_cfa_offset (4);		\
	fstps	(%esp);				\
	flds	(%esp);				\
	addl	$4, %esp;			\
	cfi_adjust_cfa_offset (-4);
#define DBL_NARROW_EVAL				\
	subl	$8, %esp;			\
	cfi_adjust_cfa_offset (8);		\
	fstpl	(%esp);				\
	fldl	(%esp);				\
	addl	$8, %esp;			\
	cfi_adjust_cfa_offset (-8);

#endif /* i386-math-asm.h.  */
