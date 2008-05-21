/* Copyright (C) 1997, 2002, 2003, 2004, 2006, 2008
   Free Software Foundation, Inc.
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

#ifdef IS_IN_rtld
# include <dl-sysdep.h>		/* Defines RTLD_PRIVATE_ERRNO.  */
#endif
#include <tls.h>

#undef SYS_ify
#define SYS_ify(syscall_name) __NR_##syscall_name

#ifdef __ASSEMBLER__

#define LOADSYSCALL(x) mov __NR_##x, %g1

/* Linux/SPARC uses a different trap number */
#undef PSEUDO
#undef PSEUDO_NOERRNO
#undef PSEUDO_ERRVAL
#undef PSEUDO_END
#undef ENTRY
#undef END
#undef LOC

#define ENTRY(name)			\
	.align	4;			\
	.global	C_SYMBOL_NAME(name);	\
	.type	name, @function;	\
C_LABEL(name)				\
	cfi_startproc;

#define END(name)			\
	cfi_endproc;			\
	.size name, . - name

#define LOC(name)  .L##name

	/* If the offset to __syscall_error fits into a signed 22-bit
	 * immediate branch offset, the linker will relax the call into
	 * a normal branch.
	 */
#define PSEUDO(name, syscall_name, args)	\
	.text;					\
	.globl		__syscall_error;	\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x10;			\
	bcc		1f;			\
	 mov		%o7, %g1;		\
	call		__syscall_error;	\
	 mov		%g1, %o7;		\
1:

#define PSEUDO_NOERRNO(name, syscall_name, args)\
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x10;

#define PSEUDO_ERRVAL(name, syscall_name, args)	\
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x10;

#define PSEUDO_END(name)			\
	END(name)

#else  /* __ASSEMBLER__ */

#if defined SHARED && defined DO_VERSIONING && defined PIC \
    && !defined NO_HIDDEN && !defined NOT_IN_libc
# define CALL_ERRNO_LOCATION "call   __GI___errno_location;"
#else
# define CALL_ERRNO_LOCATION "call   __errno_location;"
#endif

#define __SYSCALL_STRING						\
	"ta	0x10;"							\
	"bcs	2f;"							\
	" nop;"								\
	"1:"								\
	".subsection 2;"						\
	"2:"								\
	"save	%%sp, -192, %%sp;"					\
	CALL_ERRNO_LOCATION						\
	" nop;"								\
	"st	%%i0,[%%o0];"						\
	"ba	1b;"							\
	" restore %%g0, -1, %%o0;"					\
	".previous;"

#define __CLONE_SYSCALL_STRING						\
	"ta	0x10;"							\
	"bcs	2f;"							\
	" sub	%%o1, 1, %%o1;"						\
	"and	%%o0, %%o1, %%o0;"					\
	"1:"								\
	".subsection 2;"						\
	"2:"								\
	"save	%%sp, -192, %%sp;"					\
	CALL_ERRNO_LOCATION						\
	" nop;"								\
	"st	%%i0, [%%o0];"						\
	"ba	1b;"							\
	" restore %%g0, -1, %%o0;"					\
	".previous;"

#define __INTERNAL_SYSCALL_STRING					\
	"ta	0x10;"							\
	"bcs,a	1f;"							\
	" sub	%%g0, %%o0, %%o0;"					\
	"1:"

#define __SYSCALL_CLOBBERS						\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"cc", "memory"

#include <sysdeps/unix/sysv/linux/sparc/sysdep.h>

#endif	/* __ASSEMBLER__ */

/* Pointer mangling support.  */
#if defined NOT_IN_libc && defined IS_IN_rtld
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(dreg, reg, tmpreg) \
  ld	[%g7 + POINTER_GUARD], tmpreg; \
  xor	reg, tmpreg, dreg
#  define PTR_DEMANGLE(dreg, reg, tmpreg) PTR_MANGLE (dreg, reg, tmpreg)
#  define PTR_MANGLE2(dreg, reg, tmpreg) \
  xor	reg, tmpreg, dreg
#  define PTR_DEMANGLE2(dreg, reg, tmpreg) PTR_MANGLE2 (dreg, reg, tmpreg)
# else
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ THREAD_GET_POINTER_GUARD ())
#  define PTR_DEMANGLE(var)     PTR_MANGLE (var)
# endif
#endif

#endif /* linux/sparc/sysdep.h */
