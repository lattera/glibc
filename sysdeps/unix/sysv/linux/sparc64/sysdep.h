/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _LINUX_SPARC64_SYSDEP_H
#define _LINUX_SPARC64_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>

#undef SYS_ify
#define SYS_ify(syscall_name) __NR_##syscall_name

#ifdef ASSEMBLER

#ifdef DONT_LOAD_G1
# define LOADSYSCALL(x)
#else
# define LOADSYSCALL(x) mov __NR_##x, %g1
#endif

/* Linux/SPARC uses a different trap number */
#undef PSEUDO
#undef ENTRY

#define ENTRY(name)							\
	.global C_SYMBOL_NAME(name);					\
	.align 2;							\
	C_LABEL(name);							\
	.type name,@function;

#ifdef PIC
#define SYSCALL_ERROR_HANDLER1(R)					\
	.global C_SYMBOL_NAME(errno);					\
	.type C_SYMBOL_NAME(errno),@object;				\
0:	rd %pc,%g1;							\
	sethi %hi(_GLOBAL_OFFSET_TABLE_-(0b-.)),%g2;			\
	add %g2,%lo(_GLOBAL_OFFSET_TABLE_-(0b-.)),%g2;			\
	add %g2,%g1,%g1;						\
	sethi %hi(errno),%g2;						\
	add %g2,%lo(errno),%g2;						\
	ldx [%g1+%g2],%g2;						\
	st R,[%g2]
#else
#define SYSCALL_ERROR_HANDLER1(R)					\
	.global C_SYMBOL_NAME(errno);					\
	.type C_SYMBOL_NAME(errno),@object;				\
	sethi %hi(errno),%g1;						\
	add %g1,%g4,%g1;						\
	st R,[%g1+%lo(errno)]
#endif

#ifdef _LIBC_REENTRANT
#define SYSCALL_ERROR_HANDLER						\
	.global C_SYMBOL_NAME(__errno_location);			\
        .type C_SYMBOL_NAME(__errno_location),@function;		\
	save %sp,-128,%sp;						\
	SYSCALL_ERROR_HANDLER1(%i0);					\
	call __errno_location;						\
	 nop;								\
	st %i0,[%o0];							\
	sub %g0,1,%i0;							\
	jmpl %i7+8, %g0;						\
	 restore
#else
#define SYSCALL_ERROR_HANDLER						\
	SYSCALL_ERROR_HANDLER1(%o0);					\
	retl;								\
	 sub %g0,1,%i0
#endif

#define PSEUDO(name, syscall_name, args)				\
	.text;								\
	ENTRY(name);							\
	LOADSYSCALL(syscall_name);					\
	ta 0x11;							\
	bcc,pt %xcc,1f;							\
	 nop;								\
	SYSCALL_ERROR_HANDLER;						\
1:

#undef PSEUDO_END
#define PSEUDO_END(name)						\
	.size name,.-name

#undef END
#define END(name)							\
	.size name,.-name

/* Careful here!  This "ret" define can interfere; use jmpl if unsure.  */
#define ret             retl; nop
#define r0              %o0
#define r1              %o1
#define MOVE(x,y)       mov x, y

#endif	/* ASSEMBLER */

/* This is the offset from the %sp to the backing store above the 
   register windows.  So if you poke stack memory directly you add this.  */
#define STACK_BIAS	2047

#endif /* linux/sparc64/sysdep.h */
