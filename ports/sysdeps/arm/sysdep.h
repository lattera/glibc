/* Assembler macros for ARM.
   Copyright (C) 1997-2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdeps/generic/sysdep.h>
#include <features.h>

#if (!defined (__ARM_ARCH_2__) && !defined (__ARM_ARCH_3__) \
     && !defined (__ARM_ARCH_3M__) && !defined (__ARM_ARCH_4__))
# define __USE_BX__
#endif

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

#define ALIGNARG(log2) log2
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

#define PLTJMP(_x)	_x##(PLT)

/* APCS-32 doesn't preserve the condition codes across function call. */
#ifdef __APCS_32__
#ifdef __USE_BX__
#define RETINSTR(cond, reg)	\
	bx##cond	reg
#define DO_RET(_reg)		\
	bx _reg
#else
#define RETINSTR(cond, reg)	\
	mov##cond	pc, reg
#define DO_RET(_reg)		\
	mov pc, _reg
#endif
#else  /* APCS-26 */
#define RETINSTR(cond, reg)	\
	mov##cond##s	pc, reg
#define DO_RET(_reg)		\
	movs pc, _reg
#endif

/* Define an entry point visible from C.  */
#define	ENTRY(name)					\
	.globl	C_SYMBOL_NAME(name);			\
	.type	C_SYMBOL_NAME(name),%function;		\
	.align	ALIGNARG(4);				\
  C_LABEL(name)						\
	CFI_SECTIONS;					\
	cfi_startproc;					\
	CALL_MCOUNT

#define CFI_SECTIONS					\
	.cfi_sections .debug_frame

#undef	END
#define END(name)					\
	cfi_endproc;					\
	ASM_SIZE_DIRECTIVE(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef	PROF
/* Call __gnu_mcount_nc if GCC >= 4.4.  */
#if __GNUC_PREREQ(4,4)
#define CALL_MCOUNT					\
	push	{lr};					\
	cfi_adjust_cfa_offset (4);			\
	cfi_rel_offset (lr, 0);				\
	bl	PLTJMP(mcount);				\
	cfi_adjust_cfa_offset (-4);			\
	cfi_restore (lr)
#else /* else call _mcount */
#define CALL_MCOUNT					\
	push	{lr};					\
	cfi_adjust_cfa_offset (4);			\
	cfi_rel_offset (lr, 0);				\
	bl	PLTJMP(mcount);				\
	pops	{lr};					\
	cfi_adjust_cfa_offset (-4);			\
	cfi_restore (lr)
#endif
#else
#define CALL_MCOUNT		/* Do nothing.  */
#endif

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#if __GNUC_PREREQ(4,4)
#define mcount		__gnu_mcount_nc
#else
#define mcount		_mcount
#endif

/* Tag_ABI_align8_preserved: This code preserves 8-byte
   alignment in any callee.  */
	.eabi_attribute 25, 1
/* Tag_ABI_align8_needed: This code may require 8-byte alignment from
   the caller.  */
	.eabi_attribute 24, 1

/* The thumb2 encoding is reasonably complete.  Unless suppressed, use it.  */
	.syntax unified
# if defined(__thumb2__) && !defined(NO_THUMB)
	.thumb
#else
#  undef __thumb__
#  undef __thumb2__
	.arm
# endif

/* Load or store to/from a pc-relative EXPR into/from R, using T.  */
# ifdef __thumb2__
#  define LDST_PCREL(OP, R, T, EXPR) \
	ldr	T, 98f;					\
	.subsection 2;					\
98:	.word	EXPR - 99f - PC_OFS;			\
	.previous;					\
99:	add	T, T, pc;				\
	OP	R, [T]
# else
#  define LDST_PCREL(OP, R, T, EXPR) \
	ldr	T, 98f;					\
	.subsection 2;					\
98:	.word	EXPR - 99f - PC_OFS;			\
	.previous;					\
99:	OP	R, [pc, T]
# endif

/* Cope with negative memory offsets, which thumb can't encode.
   Use NEGOFF_ADJ_BASE to (conditionally) alter the base register,
   and then NEGOFF_OFF1 to use 0 for thumb and the offset for arm,
   or NEGOFF_OFF2 to use A-B for thumb and A for arm.  */
# ifdef __thumb2__
#  define NEGOFF_ADJ_BASE(R, OFF)	add R, R, $OFF
#  define NEGOFF_ADJ_BASE2(D, S, OFF)	add D, S, $OFF
#  define NEGOFF_OFF1(R, OFF)		[R]
#  define NEGOFF_OFF2(R, OFFA, OFFB)	[R, $((OFFA) - (OFFB))]
# else
#  define NEGOFF_ADJ_BASE(R, OFF)
#  define NEGOFF_ADJ_BASE2(D, S, OFF)	mov D, S
#  define NEGOFF_OFF1(R, OFF)		[R, $OFF]
#  define NEGOFF_OFF2(R, OFFA, OFFB)	[R, $OFFA]
# endif

/* Helper to get the TLS base pointer.  The interface is that TMP is a
   register that may be used to hold the LR, if necessary.  TMP may be
   LR itself to indicate that LR need not be saved.  The base pointer
   is returned in R0.  Only R0 and TMP are modified.

   At this generic level we have no tricks to pull.  Call the ABI routine.  */
# define GET_TLS(TMP)					\
	push	{ r1, r2, r3, lr };			\
	cfi_remember_state;				\
	cfi_adjust_cfa_offset (16);			\
	cfi_rel_offset (r1, 0);				\
	cfi_rel_offset (r2, 4);				\
	cfi_rel_offset (r3, 8);				\
	cfi_rel_offset (lr, 12);			\
	bl	__aeabi_read_tp;			\
	pop	{ r1, r2, r3, lr };			\
	cfi_restore_state

#endif	/* __ASSEMBLER__ */

/* This number is the offset from the pc at the current location.  */
#ifdef __thumb__
# define PC_OFS  4
#else
# define PC_OFS  8
#endif
