/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza <miguel@gnu.ai.mit.edu>, January 1997.

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

#ifndef _LINUX_SPARC_SYSDEP_H
#define _LINUX_SPARC_SYSDEP_H 1

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
	st %i0,[%o0];						\
	restore;						\
	retl;							\
	mov -1,%o0;
#else
#define SYSCALL_ERROR_HANDLER					\
	save %sp,-96,%sp;					\
	call __errno_location;					\
	nop;							\
	st %i0,[%o0];						\
	restore;						\
	retl;							\
	mov -1,%o0;
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

#endif	/* __ASSEMBLER__ */

#endif /* linux/sparc/sysdep.h */
