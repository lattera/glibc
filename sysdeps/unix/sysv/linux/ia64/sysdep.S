/* Copyright (C) 1999, 2000, 2001, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>.

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

#include <sysdep.h>
#include <features.h>
#include <tls.h>

ENTRY(__syscall_error)
#if RTLD_PRIVATE_ERRNO
	/*
	 * Note that the gp has to be set properly for this to work.
	 * As long as all syscalls are in the same load unit
	 * (executable or shared library) as this routine, we should
	 * be fine.  Otherwise, we would have to first load the global
	 * pointer register from __gp.
	 */
	addl	r2=@gprel(rtld_errno),gp
	;;
	st4	[r2]=r8
	mov	r8=-1
#elif USE___THREAD
# ifndef NOT_IN_libc
#  define SYSCALL_ERROR_ERRNO __libc_errno
# else
#  define SYSCALL_ERROR_ERRNO errno
# endif
	addl	r2=@ltoff(@tprel(SYSCALL_ERROR_ERRNO)), gp;;
	ld8	r2=[r2]
	mov	r3=r8;;
	mov	r8=-1
	add	r2=r2,r13;;
	st4	[r2]=r3
#elif defined _LIBC_REENTRANT
	.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(0)
	alloc	r33=ar.pfs, 0, 4, 0, 0
	mov	r32=rp
	.body
	mov	r35=r8
	mov	r34=r1
	;;
	br.call.sptk.many b0 = __errno_location
.Lret0:		/* force new bundle */
	st4	[r8]=r35
	mov	r1=r34
	mov	rp=r32
	mov	r8=-1
	mov	ar.pfs=r33
#else /* _LIBC_REENTRANT */
	/*
	 * Note that the gp has to be set properly for this to work.
	 * As long as all syscalls are in the same load unit
	 * (executable or shared library) as this routine, we should
	 * be fine.  Otherwise, we would have to first load the global
	 * pointer register from __gp.
	 */
	addl	r2=@ltoff(errno),gp
	;;
	ld8	r2=[r2]
	mov	r3=r8
	mov	r8=-1
	;;
	st4	[r2]=r3
#endif /* _LIBC_REENTRANT */
	ret			// ret is #define'd in syscall.h!
END(__syscall_error)

ENTRY(__ia64_syscall)
	mov r15=r37		/* syscall number */
	break __BREAK_SYSCALL
	cmp.eq p6,p0=-1,r10	/* r10 = -1 on error */
(p6)	br.cond.spnt.few __syscall_error
	ret
PSEUDO_END(__ia64_syscall)
