/* Copyright (C) 1992, 1995, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <sysdeps/unix/sysdep.h>

#ifdef __ASSEMBLER__

#ifdef __linux__
# include <alpha/regdef.h>
#else
# include <regdef.h>
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
#define PSEUDO(name, syscall_name, args)	\
    .globl name;				\
    .align 3;					\
    .ent name,0;				\
__LABEL(name)					\
    .frame sp, 0, ra;				\
    ldgp gp,0(pv);				\
    .set noat;					\
    lda AT,_mcount;				\
    jsr AT,(AT),_mcount;			\
    .set at;					\
    .prologue 1;				\
    ldiq	v0, SYS_ify(syscall_name);	\
    .set noat;					\
    call_pal	PAL_callsys;			\
    .set at;					\
    bne		a3, 1996f;			\
3:
#else
#define PSEUDO(name, syscall_name, args)	\
    .globl name;				\
    .align 3;					\
    .ent name,0;				\
__LABEL(name)					\
    .frame sp, 0, ra				\
    .prologue 0;				\
    ldiq	v0, SYS_ify(syscall_name);	\
    .set noat;					\
    call_pal	PAL_callsys;			\
    .set at;					\
    bne		a3, 1996f;			\
3:
#endif

#undef PSEUDO_END
#ifdef PROF
#define PSEUDO_END(sym)				\
1996:						\
    jmp		zero, __syscall_error;		\
    END(sym)
#else
#define PSEUDO_END(sym)				\
1996:						\
    br		gp, 2f;				\
2:  ldgp	gp, 0(gp);			\
    jmp		zero, __syscall_error;		\
    END(sym)
#endif

#define r0	v0
#define r1	a4

#define MOVE(x,y)	mov x,y

#endif
