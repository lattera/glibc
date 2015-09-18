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

/* Define constants for the minimum value of a floating-point
   type.  */
#define DEFINE_FLT_MIN				\
	.section .rodata.cst4,"aM",@progbits,4;	\
	.p2align 2;				\
	.type flt_min,@object;			\
flt_min:					\
	.byte 0, 0, 0x80, 0;			\
	.size flt_min, .-flt_min;
#define DEFINE_DBL_MIN				\
	.section .rodata.cst8,"aM",@progbits,8;	\
	.p2align 3;				\
	.type dbl_min,@object;			\
dbl_min:					\
	.byte 0, 0, 0, 0, 0, 0, 0x10, 0;	\
	.size dbl_min, .-dbl_min;

/* Remove excess range and precision by storing a value on the stack
   and loading it back.  The value is given to be nonnegative or NaN;
   if it is subnormal, also force an underflow exception.  The
   relevant constant for the minimum of the type must have been
   defined, the MO macro must have been defined for access to memory
   operands, and, if PIC, the PIC register must have been loaded.  */
#define FLT_NARROW_EVAL_UFLOW_NONNEG_NAN	\
	subl	$4, %esp;			\
	cfi_adjust_cfa_offset (4);		\
	flds	MO(flt_min);			\
	fld	%st(1);				\
	fucompp;				\
	fnstsw;					\
	sahf;					\
	jnc 6424f;				\
	fld	%st(0);				\
	fmul	%st(0);				\
	fstps	(%esp);				\
6424:	fstps	(%esp);				\
	flds	(%esp);				\
	addl	$4, %esp;			\
	cfi_adjust_cfa_offset (-4);
#define DBL_NARROW_EVAL_UFLOW_NONNEG_NAN	\
	subl	$8, %esp;			\
	cfi_adjust_cfa_offset (8);		\
	fldl	MO(dbl_min);			\
	fld	%st(1);				\
	fucompp;				\
	fnstsw;					\
	sahf;					\
	jnc 6453f;				\
	fld	%st(0);				\
	fmul	%st(0);				\
	fstpl	(%esp);				\
6453:	fstpl	(%esp);				\
	fldl	(%esp);				\
	addl	$8, %esp;			\
	cfi_adjust_cfa_offset (-8);

/* Likewise, but the argument is not a NaN (so fcom instructions,
   which support memory operands, can be used).  */
#define FLT_NARROW_EVAL_UFLOW_NONNEG		\
	subl	$4, %esp;			\
	cfi_adjust_cfa_offset (4);		\
	fcoms	MO(flt_min);			\
	fnstsw;					\
	sahf;					\
	jnc 6424f;				\
	fld	%st(0);				\
	fmul	%st(0);				\
	fstps	(%esp);				\
6424:	fstps	(%esp);				\
	flds	(%esp);				\
	addl	$4, %esp;			\
	cfi_adjust_cfa_offset (-4);
#define DBL_NARROW_EVAL_UFLOW_NONNEG		\
	subl	$8, %esp;			\
	cfi_adjust_cfa_offset (8);		\
	fcoml	MO(dbl_min);			\
	fnstsw;					\
	sahf;					\
	jnc 6453f;				\
	fld	%st(0);				\
	fmul	%st(0);				\
	fstpl	(%esp);				\
6453:	fstpl	(%esp);				\
	fldl	(%esp);				\
	addl	$8, %esp;			\
	cfi_adjust_cfa_offset (-8);

#endif /* i386-math-asm.h.  */
