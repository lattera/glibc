/* Assembly macros for 64-bit PowerPC.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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

#include <sysdeps/powerpc/sysdep.h>

#ifdef __ASSEMBLER__

#ifdef __ELF__
/* If compiled for profiling, call `_mcount' at the start of each function.
   see ppc-mcount.S for more details.  */
#ifdef	PROF
/* The mcount code relies on a the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#ifdef SYSV_ELF_PROFILING
#define CALL_MCOUNT      \
  .pushsection;          \
  .section ".data";      \
  .align ALIGNARG(2);    \
__mcount:            \
  .long  0;            \
  .previous;              \
	.section	".toc","aw";  \
.LC__mcount:; \
	.tc __mcount[TC],__mcount; \
  .previous;              \
  mflr  r0;              \
  std   r0,16(r1);        \
  ld    r0,.LC__mcount@toc(r2);    \
  bl    JUMPTARGET(_mcount);
#else /* SYSV_ELF_PROFILING */
#define CALL_MCOUNT      \
  mflr  r0;              \
  std   r0,16(r1);       \
  bl    JUMPTARGET(_mcount);
#endif /* SYSV_ELF_PROFILING */
#else  /* PROF */
#define CALL_MCOUNT		/* Do nothing.  */
#endif /* PROF */

#define DOT_LABEL(X) .##X

#define ENTRY(name)	\
	.section	".text"; \
	.align ALIGNARG(2); \
	.globl DOT_LABEL(name); \
	.type DOT_LABEL(name),@function ; \
	.globl name; \
	.section	".opd","aw"; \
	.align 3; \
	.size name,24; \
name##: ; \
	.quad DOT_LABEL(name) ; \
	.quad .TOC.@tocbase, 0; \
	.previous; \
DOT_LABEL(name):


#define EALIGN_W_0  /* No words to insert.  */
#define EALIGN_W_1  nop
#define EALIGN_W_2  nop;nop
#define EALIGN_W_3  nop;nop;nop
#define EALIGN_W_4  EALIGN_W_3;nop
#define EALIGN_W_5  EALIGN_W_4;nop
#define EALIGN_W_6  EALIGN_W_5;nop
#define EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#ifdef PROF
#define EALIGN(name, alignt, words) \
	.section	".text"; \
	.globl DOT_LABEL(name); \
	.type DOT_LABEL(name),@function ; \
	.globl name; \
	.section	".opd","aw"; \
	.align 3; \
	.size name,24; \
name##: ; \
	.quad DOT_LABEL(name) ; \
	.quad .TOC.@tocbase, 0; \
	.previous; \
	.align ALIGNARG(alignt); \
  EALIGN_W_##words;	\
DOT_LABEL(name):		\
  CALL_MCOUNT  \
  b 0f; \
  .align ALIGNARG(alignt); \
  EALIGN_W_##words; \
  0:
#else /* PROF */
#define EALIGN(name, alignt, words) \
	.section	".text"; \
	.globl DOT_LABEL(name); \
	.type DOT_LABEL(name),@function ; \
	.globl name; \
	.section	".opd","aw"; \
	.align 3; \
	.size name,24; \
name##: ; \
	.quad DOT_LABEL(name) ; \
	.quad .TOC.@tocbase, 0; \
	.previous; \
	.align ALIGNARG(alignt); \
  EALIGN_W_##words; \
DOT_LABEL(name):
#endif

/* Local labels stripped out by the linker.  */
#undef L
#define L(x) .L##x

#define tostring(s) #s
#define stringify(s) tostring(s)
#define XGLUE(a,b) a##b
#define GLUE(a,b) XGLUE(a,b)
#define LT_LABEL(name) GLUE(.LT,name)
#define LT_LABELSUFFIX(name,suffix) GLUE(GLUE(.LT,name),suffix)

/* Support Traceback tables */
#define TB_ASM			0x000c000000000000
#define TB_GLOBALLINK	0x0000800000000000
#define TB_IS_EPROL		0x0000400000000000
#define TB_HAS_TBOFF	0x0000200000000000
#define TB_INT_PROC		0x0000100000000000
#define TB_HAS_CTL		0x0000080000000000
#define TB_TOCLESS		0x0000040000000000
#define TB_FP_PRESENT	0x0000020000000000
#define TB_LOG_ABORT	0x0000010000000000
#define TB_INT_HANDL	0x0000008000000000
#define TB_NAME_PRESENT	0x0000004000000000
#define TB_USES_ALLOCA	0x0000002000000000
#define TB_SAVES_CR		0x0000000200000000
#define TB_SAVES_LR		0x0000000100000000
#define TB_STORES_BC	0x0000000080000000
#define TB_FIXUP		0x0000000040000000
#define TB_FP_SAVED(fprs)	(((fprs) & 0x3f) << 24)
#define TB_GPR_SAVED(gprs)	(((fprs) & 0x3f) << 16)
#define TB_FIXEDPARMS(parms)	(((parms) & 0xff) << 8)
#define TB_FLOATPARMS(parms)	(((parms) & 0x7f) << 1)
#define TB_PARMSONSTK	0x0000000000000001

#define PPC_HIGHER(v) (((v) >> 32) & 0xffff)
#define TB_DEFAULT	TB_ASM | TB_HAS_TBOFF | TB_NAME_PRESENT

#define TRACEBACK(name) \
LT_LABEL(name): ; \
	.long	0 ; \
	.quad	TB_DEFAULT ; \
	.long	LT_LABEL(name)-DOT_LABEL(name) ; \
	.short	LT_LABELSUFFIX(name,_name_end)-LT_LABELSUFFIX(name,_name_start) ; \
LT_LABELSUFFIX(name,_name_start): ;\
	.ascii	stringify(name) ; \
LT_LABELSUFFIX(name,_name_end): ; \
	.align	2 ;

#define TRACEBACK_MASK(name,mask) \
LT_LABEL(name): ; \
	.long	0 ; \
	.quad	TB_DEFAULT | mask ; \
	.long	LT_LABEL(name)-DOT_LABEL(name) ; \
	.short	LT_LABELSUFFIX(name,_name_end)-LT_LABELSUFFIX(name,_name_start) ; \
LT_LABELSUFFIX(name,_name_start): ;\
	.ascii	stringify(name) ; \
LT_LABELSUFFIX(name,_name_end): ; \
	.align	2 ;

/* END generates Traceback tables */
#undef	END
#define END(name) \
  TRACEBACK(name)	\
  ASM_SIZE_DIRECTIVE(DOT_LABEL(name))

/* This form supports more informative traceback tables */
#define END_GEN_TB(name,mask)	\
  TRACEBACK_MASK(name,mask)	\
  ASM_SIZE_DIRECTIVE(DOT_LABEL(name))


#define DO_CALL(syscall) \
    li 0,syscall; \
    sc

/* ppc64 is always PIC */
#undef JUMPTARGET
#define JUMPTARGET(name) DOT_LABEL(name)

#define PSEUDO(name, syscall_name, args) \
  .section ".text";	\
  ENTRY (name) \
  DO_CALL (SYS_ify (syscall_name));

#define PSEUDO_RET \
    bnslr+; \
    b JUMPTARGET(__syscall_error)

#define ret PSEUDO_RET

#undef	PSEUDO_END
#define	PSEUDO_END(name) \
  END (name)

#define PSEUDO_NOERRNO(name, syscall_name, args) \
  .section ".text";	\
  ENTRY (name) \
  DO_CALL (SYS_ify (syscall_name));

#define PSEUDO_RET_NOERRNO \
    blr

#define ret_NOERRNO PSEUDO_RET_NOERRNO

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name) \
  END (name)

#define PSEUDO_ERRVAL(name, syscall_name, args) \
  .section ".text";	\
  ENTRY (name) \
  DO_CALL (SYS_ify (syscall_name));

#define PSEUDO_RET_ERRVAL \
    blr

#define ret_ERRVAL PSEUDO_RET_ERRVAL

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

/* Label in text section.  */
/* ppc64 function descriptors which requires . notation */
#define C_TEXT(name) .##name

#endif /* __ELF__ */

#endif	/* __ASSEMBLER__ */
