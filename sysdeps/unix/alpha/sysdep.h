/* Copyright (C) 1992, 1995, 1996, 2000 Free Software Foundation, Inc.
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

#include <sysdeps/unix/sysdep.h>

#ifdef __ASSEMBLER__

#ifdef __linux__
# include <alpha/regdef.h>
#else
# include <regdef.h>
#endif

#include <tls.h>		/* Defines USE___THREAD.  */

#ifdef IS_IN_rtld
# include <dl-sysdep.h>         /* Defines RTLD_PRIVATE_ERRNO.  */
#endif


#ifdef __STDC__
#define __LABEL(x)	x##:
#else
#define __LABEL(x)	x/**/:
#endif

#define LEAF(name, framesize)			\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  __LABEL(name)					\
  .frame sp, framesize, ra

#define ENTRY(name)				\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  __LABEL(name)					\
  .frame sp, 0, ra

/* Mark the end of function SYM.  */
#undef END
#define END(sym)	.end sym

/* Note that PSEUDO/PSEUDO_END use label number 1996---do not use a
   label of that number between those two macros!  */

#ifdef PROF
# define PSEUDO_PROLOGUE			\
	.frame sp, 0, ra;			\
	ldgp	gp,0(pv);			\
	.set noat;				\
	lda	AT,_mcount;			\
	jsr	AT,(AT),_mcount;		\
	.set at;				\
	.prologue 1
# define PSEUDO_LOADGP
#else
# define PSEUDO_PROLOGUE			\
	.frame sp, 0, ra;			\
	.prologue 0
# define PSEUDO_LOADGP				\
	br	gp, 2f;				\
2:	ldgp	gp, 0(gp)
#endif /* PROF */

#if RTLD_PRIVATE_ERRNO
# define SYSCALL_ERROR_HANDLER			\
	stl	v0, errno(gp)	!gprel;		\
	lda	v0, -1;				\
	ret
#else
# define SYSCALL_ERROR_HANDLER \
	jmp	$31, __syscall_error
#endif /* RTLD_PRIVATE_ERRNO */

#if defined(PIC) && !RTLD_PRIVATE_ERRNO
# define PSEUDO(name, syscall_name, args)	\
	.globl name;				\
	.align 4;				\
	.ent name,0;				\
__LABEL(name)					\
	PSEUDO_PROLOGUE;			\
	lda	v0, SYS_ify(syscall_name);	\
	call_pal PAL_callsys;			\
	bne	a3, __syscall_error !samegp;	\
3:
# undef PSEUDO_END
# define PSEUDO_END(sym)  END(sym)
#else
# define PSEUDO(name, syscall_name, args)	\
	.globl name;				\
	.align 4;				\
	.ent name,0;				\
__LABEL(name)					\
	lda	v0, SYS_ify(syscall_name);	\
	call_pal PAL_callsys;			\
	bne	a3, 1996f;			\
3:

# undef PSEUDO_END
# define PSEUDO_END(sym)			\
1996:						\
	PSEUDO_LOADGP;				\
	SYSCALL_ERROR_HANDLER;			\
	END(sym)
#endif /* PIC && !RTLD_PRIVATE_ERRNO */

#define r0	v0
#define r1	a4

#define MOVE(x,y)	mov x,y

#endif /* ASSEMBLER */
