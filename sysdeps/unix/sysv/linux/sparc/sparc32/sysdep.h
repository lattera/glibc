/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza <miguel@gnu.ai.mit.edu>, January 1997.

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

#ifndef _LINUX_SPARC32_SYSDEP_H
#define _LINUX_SPARC32_SYSDEP_H 1

#include <sysdeps/unix/sparc/sysdep.h>

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
#undef END
#undef LOC

#define ENTRY(name) \
	.global C_SYMBOL_NAME(name); \
	.align 4;\
	C_LABEL(name);\
	.type name,@function;

#define END(name) \
	.size name, . - name

#define LOC(name)  . ## L ## name

#ifdef PIC
#define SYSCALL_ERROR_HANDLER					\
	.global C_SYMBOL_NAME(__errno_location);		\
        .type   C_SYMBOL_NAME(__errno_location),@function;	\
	save   %sp,-96,%sp;					\
	call   __errno_location;				\
	 nop;							\
	st	%i0,[%o0];					\
	jmpl	%i7+8,%g0;					\
	 restore %g0,-1,%o0;
#else
#define SYSCALL_ERROR_HANDLER					\
	save	%sp,-96,%sp;					\
	call	__errno_location;				\
	nop;							\
	st	%i0,[%o0];					\
	jmpl	%i7+8,%g0;					\
	 restore %g0,-1,%o0;
#endif   /* PIC */

#define PSEUDO(name, syscall_name, args)			\
	.text;							\
	ENTRY(name);						\
	LOADSYSCALL(syscall_name);				\
	ta 0x10;						\
	bcc,a 9000f;						\
	nop;							\
	SYSCALL_ERROR_HANDLER;					\
9000:;

#else  /* __ASSEMBLER__ */

#define __SYSCALL_STRING						\
	"ta	0x10;"							\
	"bcs	2f;"							\
	" nop;"								\
	"1:"								\
	".subsection 2;"						\
	"2:"								\
	"save	%%sp, -192, %%sp;"					\
	"call	__errno_location;"					\
	" nop;"								\
	"st	%%i0,[%%o0];"						\
	"ba	1b;"							\
	" restore %%g0, -1, %%o0;"					\
	".previous;"

#define __SYSCALL_CLOBBERS "g2", "g3", "g4", "g5", "g7",		\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"cc", "memory"

#include <sysdeps/unix/sysv/linux/sparc/sysdep.h>

#endif	/* __ASSEMBLER__ */

#endif /* linux/sparc/sysdep.h */
