/* Copyright (C) 1997, 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
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

#ifdef IS_IN_rtld
# include <dl-sysdep.h>		/* Defines RTLD_PRIVATE_ERRNO.  */
#endif

#undef SYS_ify
#define SYS_ify(syscall_name) __NR_##syscall_name

/* This is a kludge to make syscalls.list find these under the names
   pread and pwrite, since some kernel headers define those names
   and some define the *64 names for the same system calls.  */
#if !defined __NR_pread && defined __NR_pread64
# define __NR_pread __NR_pread64
#endif
#if !defined __NR_pwrite && defined __NR_pwrite64
# define __NR_pwrite __NR_pwrite64
#endif

#ifdef __ASSEMBLER__

#define LOADSYSCALL(x) mov __NR_##x, %g1

/* Linux/SPARC uses a different trap number */
#undef PSEUDO
#undef PSEUDO_NOERRNO
#undef PSEUDO_ERRVAL
#undef ENTRY

#define ENTRY(name)							\
	.global C_SYMBOL_NAME(name);					\
	.align 2;							\
	C_LABEL(name);							\
	.type name,@function;

#ifdef LINKER_HANDLES_R_SPARC_WDISP22
/* Unfortunately, we cannot do this yet.  Linker doesn't seem to
   handle R_SPARC_WDISP22 against non-STB_LOCAL symbols properly .  */
# define SYSCALL_ERROR_HANDLER_ENTRY(handler)				\
	.section .gnu.linkonce.t.handler,"ax",@progbits;		\
	.globl handler;							\
	.hidden handler;						\
	.type handler,@function;					\
handler:
#else
# define SYSCALL_ERROR_HANDLER_ENTRY(handler)				\
	.subsection 3;							\
handler:
#endif

#if RTLD_PRIVATE_ERRNO
# define SYSCALL_ERROR_HANDLER						\
	.section .gnu.linkonce.t.__sparc64.get_pic.l7,"ax",@progbits;	\
	.globl __sparc64.get_pic.l7;					\
	.hidden __sparc64.get_pic.l7;					\
	.type __sparc64.get_pic.l7,@function;				\
__sparc64.get_pic.l7:							\
	retl;								\
	 add	%o7, %l7, %l7;						\
	.previous;							\
SYSCALL_ERROR_HANDLER_ENTRY(__syscall_error_handler)			\
	save	%sp, -192, %sp;						\
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %l7;			\
	call	__sparc64.get_pic.l7;					\
	 add	%l7, %lo(_GLOBAL_OFFSET_TABLE_+4), %l7;			\
	ldx	[%l7 + rtld_errno], %l0;				\
	st	%i0, [%l0];						\
	jmpl	%i7+8, %g0;						\
	 restore %g0, -1, %o0;						\
	.previous;
#else
# define SYSCALL_ERROR_HANDLER						\
SYSCALL_ERROR_HANDLER_ENTRY(__syscall_error_handler)			\
	.global __errno_location;					\
	.type   __errno_location,@function;				\
	save	%sp, -192, %sp;						\
	call	__errno_location;					\
	 nop;								\
	st	%i0, [%o0];						\
	jmpl	%i7+8, %g0;						\
	 restore %g0, -1, %o0;						\
	.previous;
#endif

#define PSEUDO(name, syscall_name, args)				\
	.text;								\
	ENTRY(name);							\
	LOADSYSCALL(syscall_name);					\
	ta	0x6d;							\
	bcs,pn	%xcc, __syscall_error_handler;				\
	 nop;								\
	SYSCALL_ERROR_HANDLER

#define PSEUDO_NOERRNO(name, syscall_name, args)			\
	.text;								\
	ENTRY(name);							\
	LOADSYSCALL(syscall_name);					\
	ta	0x6d

#define PSEUDO_ERRVAL(name, syscall_name, args)				\
	.text;								\
	ENTRY(name);							\
	LOADSYSCALL(syscall_name);					\
	ta	0x6d

#undef PSEUDO_END
#define PSEUDO_END(name)						\
	.size name,.-name

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(name)					\
	.size name,.-name

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name)						\
	.size name,.-name

#undef END
#define END(name)							\
	.size name,.-name

/* Careful here!  This "ret" define can interfere; use jmpl if unsure.  */
#define ret		retl; nop
#define ret_NOERRNO	retl; nop
#define ret_ERRVAL	retl; nop
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

#define __CLONE_SYSCALL_STRING						\
	"ta	0x6d;"							\
	"bcc,pt	%%xcc, 1f;"						\
	" sub	%%o1, 1, %%o1;"						\
	"save	%%sp, -192, %%sp;"					\
	"call	__errno_location;"					\
	" mov	-1, %%i1;"						\
	"st	%%i0,[%%o0];"						\
	"restore %%g0, -1, %%o0;"					\
	"1:"								\
	"and	%%o0, %%o1, %%o0"

#define __INTERNAL_SYSCALL_STRING					\
	"ta	0x6d;"							\
	"bcs,a,pt %%xcc, 1f;"						\
	" sub	%%g0, %%o0, %%o0;"					\
	"1:"

#define __SYSCALL_CLOBBERS "g2", "g3", "g4", "g5", "g6",		\
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
