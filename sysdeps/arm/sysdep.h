/* Assembler macros for ARM.
   Copyright (C) 1997, 1998, 2003, 2009, 2010 Free Software Foundation, Inc.
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

#include <sysdeps/generic/sysdep.h>
#include <features.h>

#if (!defined (__ARM_ARCH_2__) && !defined (__ARM_ARCH_3__) \
     && !defined (__ARM_ARCH_3M__) && !defined (__ARM_ARCH_4__))
# define __USE_BX__
#endif

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

#ifdef HAVE_ELF

#define ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,%##typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* In ELF C symbols are asm symbols.  */
#undef	NO_UNDERSCORES
#define NO_UNDERSCORES

#define PLTJMP(_x)	_x##(PLT)

#else

#define ALIGNARG(log2) log2
#define ASM_TYPE_DIRECTIVE(name,type)	/* Nothing is specified.  */
#define ASM_SIZE_DIRECTIVE(name)	/* Nothing is specified.  */

#define PLTJMP(_x)	_x

#endif

/* APCS-32 doesn't preserve the condition codes across function call. */
#ifdef __APCS_32__
#define LOADREGS(cond, base, reglist...)\
	ldm##cond	base,reglist
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
#define LOADREGS(cond, base, reglist...)\
	ldm##cond	base,reglist^
#define RETINSTR(cond, reg)	\
	mov##cond##s	pc, reg
#define DO_RET(_reg)		\
	movs pc, _reg
#endif

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),function)			      \
  .align ALIGNARG(4);							      \
  C_LABEL(name)								      \
  .cfi_sections .debug_frame;						      \
  cfi_startproc;							      \
  CALL_MCOUNT

#undef	END
#define END(name)							      \
  cfi_endproc;								      \
  ASM_SIZE_DIRECTIVE(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef	PROF
/* Call __gnu_mcount_nc if GCC >= 4.4 and abi = EABI.  */
#if __GNUC_PREREQ(4,4) && defined(__ARM_EABI__)
#define CALL_MCOUNT \
  str	lr,[sp, #-4]!; \
  cfi_adjust_cfa_offset (4); \
  cfi_rel_offset (lr, 0); \
  bl PLTJMP(mcount); \
  cfi_adjust_cfa_offset (-4); \
  cfi_restore (lr)
#else /* else call _mcount */
#define CALL_MCOUNT \
  str	lr,[sp, #-4]!; \
  cfi_adjust_cfa_offset (4); \
  cfi_rel_offset (lr, 0); \
  bl PLTJMP(mcount); \
  ldr lr, [sp], #4; \
  cfi_adjust_cfa_offset (-4); \
  cfi_restore (lr)
#endif
#else
#define CALL_MCOUNT		/* Do nothing.  */
#endif

#ifdef	NO_UNDERSCORES
/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#if __GNUC_PREREQ(4,4) && defined(__ARM_EABI__)
#define mcount		__gnu_mcount_nc
#else
#define mcount		_mcount
#endif
#endif

#if defined(__ARM_EABI__)
/* Tag_ABI_align8_preserved: This code preserves 8-byte
   alignment in any callee.  */
	.eabi_attribute 25, 1
/* Tag_ABI_align8_needed: This code may require 8-byte alignment from
   the caller.  */
	.eabi_attribute 24, 1
#endif

#endif	/* __ASSEMBLER__ */
