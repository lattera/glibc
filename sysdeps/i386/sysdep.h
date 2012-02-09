/* Assembler macros for i386.
   Copyright (C) 1991-93,95,96,98,2002,2003,2005,2006,2011,2012
   Free Software Foundation, Inc.
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

#include <sysdeps/generic/sysdep.h>

#include <features.h> /* For __GNUC_PREREQ.  */

/* It is desirable that the names of PIC thunks match those used by
   GCC so that multiple copies are eliminated by the linker.  Because
   GCC 4.6 and earlier use __i686 in the names, it is necessary to
   override that predefined macro.  */
#if defined __i686 && defined __ASSEMBLER__
#undef __i686
#define __i686 __i686
#endif

#ifdef	__ASSEMBLER__
# if __GNUC_PREREQ (4, 7)
#  define GET_PC_THUNK(reg) __x86.get_pc_thunk.reg
# else
#  define GET_PC_THUNK(reg) __i686.get_pc_thunk.reg
# endif
#else
# if __GNUC_PREREQ (4, 7)
#  define GET_PC_THUNK_STR(reg) "__x86.get_pc_thunk." #reg
# else
#  define GET_PC_THUNK_STR(reg) "__i686.get_pc_thunk." #reg
# endif
#endif

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

/* ELF uses byte-counts for .align, most others use log2 of count of bytes.  */
#define ALIGNARG(log2) 1<<log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;


/* Define an entry point visible from C.

   There is currently a bug in gdb which prevents us from specifying
   incomplete stabs information.  Fake some entries here which specify
   the current source file.  */
#define	ENTRY(name)							      \
  STABS_CURRENT_FILE1("")						      \
  STABS_CURRENT_FILE(name)						      \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),@function)			      \
  .align ALIGNARG(4);							      \
  STABS_FUN(name)							      \
  C_LABEL(name)								      \
  cfi_startproc;							      \
  CALL_MCOUNT

#undef	END
#define END(name)							      \
  cfi_endproc;								      \
  ASM_SIZE_DIRECTIVE(name)						      \
  STABS_FUN_END(name)

#define ENTRY_CHK(name) ENTRY (name)
#define END_CHK(name) END (name)

#ifdef HAVE_CPP_ASM_DEBUGINFO
/* Disable that goop, because we just pass -g through to the assembler
   and it generates proper line number information directly.  */
# define STABS_CURRENT_FILE1(name)
# define STABS_CURRENT_FILE(name)
# define STABS_FUN(name)
# define STABS_FUN_END(name)
#else
/* Remove the following two lines once the gdb bug is fixed.  */
#define STABS_CURRENT_FILE(name)					      \
  STABS_CURRENT_FILE1 (#name)
#define STABS_CURRENT_FILE1(name)					      \
  1: .stabs name,100,0,0,1b;
/* Emit stabs definition lines.  We use F(0,1) and define t(0,1) as `int',
   the same way gcc does it.  */
#define STABS_FUN(name) STABS_FUN2(name, name##:F(0,1))
#define STABS_FUN2(name, namestr)					      \
  .stabs "int:t(0,1)=r(0,1);-2147483648;2147483647;",128,0,0,0;		      \
  .stabs #namestr,36,0,0,name;
#define STABS_FUN_END(name)						      \
  1: .stabs "",36,0,0,1b-name;
#endif

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef	PROF
/* The mcount code relies on a normal frame pointer being on the stack
   to locate our caller, so push one just for its benefit.  */
#define CALL_MCOUNT \
  pushl %ebp; cfi_adjust_cfa_offset (4); movl %esp, %ebp; \
  cfi_def_cfa_register (ebp); call JUMPTARGET(mcount); \
  popl %ebp; cfi_def_cfa (esp, 4);
#else
#define CALL_MCOUNT		/* Do nothing.  */
#endif

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#define mcount		_mcount

#define	PSEUDO(name, syscall_name, args)				      \
  .globl syscall_error;							      \
lose: SYSCALL_PIC_SETUP							      \
  jmp JUMPTARGET(syscall_error);					      \
  ENTRY (name)								      \
  DO_CALL (syscall_name, args);						      \
  jb lose

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  END (name)

# define SETUP_PIC_REG(reg) \
  .ifndef GET_PC_THUNK(reg);						      \
  .section .gnu.linkonce.t.GET_PC_THUNK(reg),"ax",@progbits;		      \
  .globl GET_PC_THUNK(reg);						      \
  .hidden GET_PC_THUNK(reg);						      \
  .p2align 4;								      \
  .type GET_PC_THUNK(reg),@function;					      \
GET_PC_THUNK(reg):							      \
  movl (%esp), %e##reg;							      \
  ret;									      \
  .size GET_PC_THUNK(reg), . - GET_PC_THUNK(reg);			      \
  .previous;								      \
  .endif;								      \
  call GET_PC_THUNK(reg)

# define LOAD_PIC_REG(reg) \
  SETUP_PIC_REG(reg); addl $_GLOBAL_OFFSET_TABLE_, %e##reg

#undef JUMPTARGET
#ifdef PIC
#define JUMPTARGET(name)	name##@PLT
#define SYSCALL_PIC_SETUP \
    pushl %ebx;								      \
    cfi_adjust_cfa_offset (4);						      \
    call 0f;								      \
0:  popl %ebx;								      \
    cfi_adjust_cfa_offset (-4);						      \
    addl $_GLOBAL_OFFSET_TABLE+[.-0b], %ebx;

#else
#define JUMPTARGET(name)	name
#define SYSCALL_PIC_SETUP	/* Nothing.  */
#endif

/* Local label name for asm code. */
#ifndef L
#define L(name)		.L##name
#endif

#define atom_text_section .section ".text.atom", "ax"

#else /* __ASSEMBLER__ */

# define SETUP_PIC_REG_STR(reg)						\
  ".ifndef " GET_PC_THUNK_STR (reg) "\n"				\
  ".section .gnu.linkonce.t." GET_PC_THUNK_STR (reg) ",\"ax\",@progbits\n" \
  ".globl " GET_PC_THUNK_STR (reg) "\n"					\
  ".hidden " GET_PC_THUNK_STR (reg) "\n"				\
  ".p2align 4\n"							\
  ".type " GET_PC_THUNK_STR (reg) ",@function\n"			\
GET_PC_THUNK_STR (reg) ":"						\
  "movl (%%esp), %%e" #reg "\n"						\
  "ret\n"								\
  ".size " GET_PC_THUNK_STR (reg) ", . - " GET_PC_THUNK_STR (reg) "\n"	\
  ".previous\n"								\
  ".endif\n"								\
  "call " GET_PC_THUNK_STR (reg)

# define LOAD_PIC_REG_STR(reg) \
  SETUP_PIC_REG_STR (reg) "\naddl $_GLOBAL_OFFSET_TABLE_, %%e" #reg

#endif	/* __ASSEMBLER__ */
