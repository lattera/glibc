/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <sysdeps/generic/sysdep.h>
#include <bits/wordsize.h>
#include <arch/abi.h>

#ifndef HAVE_ELF
# error "ELF is assumed."
#endif

#ifndef NO_UNDERSCORES
# error "User-label prefix (underscore) assumed absent."
#endif

#if defined __ASSEMBLER__ || defined REQUEST_ASSEMBLER_MACROS

#include <feedback-asm.h>

/* Make use of .type and .size directives.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),@function)			      \
  .align 8;								      \
  C_LABEL(name)								      \
  cfi_startproc;							      \
  CALL_MCOUNT

#undef	END
#define END(name)							      \
  cfi_endproc;								      \
  ASM_SIZE_DIRECTIVE(name)

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#define mcount		__mcount

/* If compiled for profiling, call `mcount' at the start of each function.
   The mcount code requires the caller PC in r10.  The `mcount' function
   sets lr back to the value r10 had on entry when it returns.  */
#ifdef	PROF
#define CALL_MCOUNT { move r10, lr; jal mcount }
#else
#define CALL_MCOUNT             /* Do nothing.  */
#endif

/* Local label name for asm code. */
#define L(name)		.L##name

/* Specify the size in bytes of a machine register.  */
#ifdef __tilegx__
#define REGSIZE		8
#else
#define REGSIZE		4
#endif

/* Support a limited form of shared assembly between tile and tilegx.
   The presumption is that LD/ST are used for manipulating registers.
   Since opcode parsing is case-insensitive, we don't need to provide
   definitions for these on tilegx.  */
#ifndef __tilegx__
#define LD		lw
#define LD4U		lw
#define ST		sw
#define ST4		sw
#define BNEZ		bnz
#define BEQZ		bz
#define BEQZT		bzt
#define BGTZ		bgz
#define CMPEQI		seqi
#define CMPEQ		seq
#define CMOVEQZ		mvz
#define CMOVNEZ		mvnz
#endif

/* Provide "pointer-oriented" instruction variants.  These differ not
   just for tile vs tilegx, but also for tilegx -m64 vs -m32.  */
#if defined __tilegx__ && __WORDSIZE == 32
#define ADD_PTR		addx
#define ADDI_PTR	addxi
#define ADDLI_PTR	addxli
#define LD_PTR		ld4s
#define ST_PTR		st4
#define SHL_PTR_ADD	shl2add
#else
#define ADD_PTR		add
#define ADDI_PTR	addi
#define ADDLI_PTR	addli
#define LD_PTR		LD
#define ST_PTR		ST
#ifdef __tilegx__
#define SHL_PTR_ADD	shl3add
#else
#define SHL_PTR_ADD	s2a
#endif
#endif

#endif /* __ASSEMBLER__ */
