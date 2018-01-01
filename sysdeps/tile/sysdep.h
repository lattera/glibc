/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdeps/generic/sysdep.h>
#include <bits/wordsize.h>
#include <arch/abi.h>

#if defined __ASSEMBLER__ || defined REQUEST_ASSEMBLER_MACROS

#include <feedback.h>

/* Make use of .size directive.  */
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  .globl C_SYMBOL_NAME(name);						      \
  .type C_SYMBOL_NAME(name),@function;					      \
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
#define REGSIZE		8

/* Provide "pointer-oriented" instruction variants.  These differ not
   just for tilepro vs tilegx, but also for tilegx -m64 vs -m32.  */
#if __WORDSIZE == 32
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
#define SHL_PTR_ADD	shl3add
#endif

#endif /* __ASSEMBLER__ */
