/* Copyright (C) 1993, 1994, 1995, 1997, 2003 Free Software Foundation, Inc.
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

#include <sysdeps/unix/sysdep.h>

#ifdef	__ASSEMBLER__

#ifdef	NO_UNDERSCORES
/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	C_SYMBOL_NAME(__syscall_error)
#endif

#ifdef PIC
#define SETUP_PIC_REG(reg, tmp)						\
	.ifndef __sparc_get_pc_thunk.reg;				\
	.section .text.__sparc_get_pc_thunk.reg,"axG",@progbits,__sparc_get_pc_thunk.reg,comdat; \
	.align	 32;							\
	.weak	 __sparc_get_pc_thunk.reg;				\
	.hidden	 __sparc_get_pc_thunk.reg;				\
	.type	 __sparc_get_pc_thunk.reg, #function;			\
__sparc_get_pc_thunk.reg:		   				\
	jmp	%o7 + 8;						\
	 add	%o7, %reg, %##reg;					\
	.previous;							\
	.endif;								\
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %##reg;			\
	mov	%o7, %##tmp;		      				\
	call	__sparc_get_pc_thunk.reg;				\
	 or	%##reg, %lo(_GLOBAL_OFFSET_TABLE_+4), %##reg;		\
	mov	%##tmp, %o7;
#endif

#ifdef HAVE_ELF
#define	ENTRY(name)		\
  .global C_SYMBOL_NAME(name);	\
  .type name,@function;		\
  .align 4;			\
  C_LABEL(name)

#else
#define	ENTRY(name)		\
  .global C_SYMBOL_NAME(name);	\
  .align 4;			\
  C_LABEL(name)

#endif /* HAVE_ELF */

#define	PSEUDO(name, syscall_name, args)	\
  .global syscall_error;			\
  ENTRY (name)					\
  mov SYS_ify(syscall_name), %g1;		\
  ta 0;						\
  bcc 1f;					\
  sethi %hi(syscall_error), %g1;		\
  jmp %g1 + %lo(syscall_error);	nop;		\
1:

#define	PSEUDO_NOERRNO(name, syscall_name, args) \
  .global syscall_error;			\
  ENTRY (name)					\
  mov SYS_ify(syscall_name), %g1;		\
  ta 0

#define	PSEUDO_ERRVAL(name, syscall_name, args) \
  .global syscall_error;			\
  ENTRY (name)					\
  mov SYS_ify(syscall_name), %g1;		\
  ta 0

#define	ret		retl; nop
#define	ret_NOERRNO	retl; nop
#define	ret_ERRVAL	retl; nop
#define	r0		%o0
#define	r1		%o1
#define	MOVE(x,y)	mov x, y

#endif	/* __ASSEMBLER__ */
