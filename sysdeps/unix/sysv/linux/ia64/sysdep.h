/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.
   Based on code originally written by David Mosberger-Tang

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
#include <sysdeps/ia64/sysdep.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#ifdef __STDC__
# define SYS_ify(syscall_name)	__NR_##syscall_name
#else
# define SYS_ify(syscall_name)	__NR_/**/syscall_name
#endif

#ifdef __ASSEMBLER__

#undef CALL_MCOUNT
#ifdef PROF
# define CALL_MCOUNT							\
	.data;								\
1:	data8 0;	/* XXX fixme: use .xdata8 once labels work */	\
	.previous;							\
	.prologue;							\
	.save ar.pfs, r40;						\
	alloc out0 = ar.pfs, 8, 0, 4, 0;				\
	mov out1 = gp;							\
	.save rp, out2;							\
	mov out2 = rp;							\
	.body;								\
	;;								\
	addl out3 = @ltoff(1b), gp;					\
	br.call.sptk.many rp = _mcount					\
	;;
#else
# define CALL_MCOUNT	/* Do nothing. */
#endif

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be negative
   even if the call succeeded.  E.g., the `lseek' system call might return
   a large offset.  Therefore we must not anymore test for < 0, but test
   for a real error by making sure the value in %d0 is a real error
   number.  Linus said he will make sure the no syscall returns a value
   in -1 .. -4095 as a valid result so we can savely test with -4095.  */

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#define SYSCALL_ERROR_LABEL __syscall_error

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)	\
  ENTRY(name)					\
    DO_CALL (SYS_ify(syscall_name));		\
	cmp.eq p6,p0=-1,r10;;			\
(p6)	br.cond.spnt.few __syscall_error;

#define DO_CALL(num)				\
	mov r15=num;				\
	break __BREAK_SYSCALL;

#undef PSEUDO_END
#define PSEUDO_END(name)	.endp C_SYMBOL_NAME(name);

#undef END
#define END(name)						\
	.size	C_SYMBOL_NAME(name), . - C_SYMBOL_NAME(name) ;	\
	.endp	C_SYMBOL_NAME(name)

#define ret			br.ret.sptk.few b0

#else /* not __ASSEMBLER__ */

/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#if 0
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)	__##name (args)
#endif

#endif /* not __ASSEMBLER__ */
