/* Copyright (C) 1993, 1996, 1998, 2002, 2003, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#if defined(__ELF__) && defined(PIC)
	/* Put this at the end of libc's text segment so that all of
	   the direct branches from the syscalls are forward, and 
	   thus predicted not taken.  */
	.section .text.last, "ax", @progbits
#else
	.text
#endif

#ifdef PIC
	/* When building a shared library, we branch here without
	   having loaded the GP.  Nor, since it was a direct branch,
	   have we loaded PV with our address.  Do both.  */
# define LOADGP		br pv, 1f; 1: ldgp gp, 0(pv)
# define PROLOGUE	.prologue 0
# define EPILOGUE
#else
	/* When building the static library, we tail call here from
	   elsewhere, which might use a different GP.  The entertaining
	   part is that we have to return with the GP of our caller
	   in place, so that linker relaxation works properly.  */
	/* ??? This is so ugly.  Consider always putting the errno
	   setting code with the syscall in the static case.  */
# define GPSAVEREG	t10
# define LOADGP		ldah	t11, 0(pv) !gpdisp!1;		\
			br	1f;				\
			.subsection 2;				\
			1: mov	gp, GPSAVEREG;			\
			lda	gp, 0(t11) !gpdisp!1;		\
			br	2f;				\
			.previous;				\
			mov	gp, GPSAVEREG;			\
			2:
# define PROLOGUE	.prologue 1
# define EPILOGUE	mov	GPSAVEREG, gp
#endif

	.align 4
	.globl __syscall_error
	.ent __syscall_error
__syscall_error:

#if defined(_LIBC_REENTRANT) && USE___THREAD

#ifndef NOT_IN_libc
# define SYSCALL_ERROR_ERRNO __libc_errno
#else
# define SYSCALL_ERROR_ERRNO errno
#endif

	LOADGP
	PROLOGUE
	mov	v0, t0
	call_pal PAL_rduniq
	ldq	t1, SYSCALL_ERROR_ERRNO(gp) !gottprel
	addq	v0, t1, v0
	stl	t0, 0(v0)
	lda	v0, -1
	EPILOGUE
	ret

#elif defined(_LIBC_REENTRANT)

	LOADGP
	lda	sp, -32(sp)
	.frame	sp, 32, ra, 0
	stq	ra, 0(sp)
	stq	v0, 8(sp)
#ifdef GPSAVEREG
	stq	GPSAVEREG, 16(sp)
#endif
	.mask	0x4000001, -32
	PROLOGUE

	/* Find our per-thread errno address  */
#if defined PIC && !defined IS_IN_librt
	bsr	ra, __errno_location	!samegp
#else
	jsr	ra, __errno_location
#ifndef GPSAVEREG
	ldgp	gp, 0(ra)
#endif
#endif

	/* Store the error value.  */
	ldq	t0, 8(sp)
	stl	t0, 0(v0)

	/* And kick back a -1.  */
	ldi	v0, -1

#ifdef GPSAVEREG
	ldq	GPSAVEREG, 16(sp)
#endif
	ldq	ra, 0(sp)
	lda	sp, 32(sp)
	EPILOGUE
	ret

#else

	LOADGP
	PROLOGUE
	stl	v0, errno
	lda	v0, -1
	EPILOGUE
	ret

#endif

	.subsection 3
	.end __syscall_error
