/* Copyright (C) 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.

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

#ifndef _LINUX_SPARC64_SYSDEP_H
#define _LINUX_SPARC64_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>

#undef SYS_ify
#define SYS_ify(syscall_name) __NR_##syscall_name

#ifdef __ASSEMBLER__

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

#define SYSCALL_ERROR_HANDLER						\
	save	%sp, -192, %sp;						\
	call	__errno_location;					\
	 nop;								\
	st	%i0, [%o0];						\
	jmpl	%i7+8, %g0;						\
	 restore %g0, -1, %o0

#define PSEUDO(name, syscall_name, args)				\
	.text;								\
	ENTRY(name);							\
	LOADSYSCALL(syscall_name);					\
	ta	0x6d;							\
	bcc,pt	%xcc, 1f;						\
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

#else  /* __ASSEMBLER__ */

#define __SYSCALL_STRING						\
	"ta	0x6d;"							\
	"bcc,pt	%%xcc, 1f;"						\
	" nop;"								\
	"save	%%sp, -192, %%sp;"					\
	"call	__errno_location;"					\
	" nop;"								\
	"st	%%i0,[%%o0];"						\
	"restore %%g0, -1, %%o0;"					\
	"1:"

#define __SYSCALL_CLOBBERS "g2", "g3", "g4", "g5", "g7",		\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"f32", "f34", "f36", "f38", "f40", "f42", "f44", "f46",		\
	"f48", "f50", "f52", "f54", "f56", "f58", "f60", "f62",		\
	"cc", "memory"

#include <sysdeps/unix/sysv/linux/sparc/sysdep.h>

#endif	/* __ASSEMBLER__ */

/* This is the offset from the %sp to the backing store above the 
   register windows.  So if you poke stack memory directly you add this.  */
#define STACK_BIAS	2047

#endif /* linux/sparc64/sysdep.h */
