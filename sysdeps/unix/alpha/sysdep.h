/* Copyright (C) 1992, 1995, 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdeps/unix/sysdep.h>

#ifdef ASSEMBLER

#ifdef __linux__
# include <alpha/regdef.h>
#else
# include <regdef.h>
#endif

#ifdef __STDC__
#define LEAF(name, framesize)			\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  name##:					\
  .frame sp, framesize, ra
#else
#define LEAF(name, framesize)			\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  name/**/:					\
  .frame sp, framesize, ra
#endif

#ifdef __STDC__
#define ENTRY(name)				\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  name##:					\
  .frame sp, 0, ra
#else
#define ENTRY(name)				\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  name/**/:					\
  .frame sp, 0, ra
#endif

/* Mark the end of function SYM.  */
#undef END
#define END(sym)	.end sym

/* Note that PSEUDO/PSEUDO_END use label number 1996---do not use a
   label of that number between those two macros!  */
 
#ifdef __STDC__
#define PSEUDO(name, syscall_name, args)	\
    .globl name;				\
    .align 3;					\
    .ent name,0;				\
						\
name##:						\
    .frame sp, 0, ra				\
    .prologue 1; /* yes, we do use gp */	\
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
						\
name/**/:					\
    .frame sp, 0, ra				\
    .prologue 1; /* yes, we do use gp */	\
    ldiq	v0, SYS_ify(syscall_name);	\
    .set noat;					\
    call_pal	PAL_callsys;			\
    .set at;					\
    bne		a3, 1996f;			\
3:
#endif

#undef PSEUDO_END

#define PSEUDO_END(sym)				\
1996:						\
    br		gp, 2f;				\
2:  ldgp	gp, 0(gp);			\
    jmp		zero, __syscall_error;		\
    END(sym)

#define r0	v0
#define r1	a4

#define MOVE(x,y)	mov x,y

#endif
