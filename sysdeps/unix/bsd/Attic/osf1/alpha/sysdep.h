/* Copyright (C) 1993 Free Software Foundation, Inc.
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
#include <machine/pal.h>		/* get PAL_callsys */
#include <regdef.h>

#ifdef __STDC__
#define ENTRY(name) \
  .globl name;								      \
  .ent name,0;								      \
  name##:;								      \
  .frame sp,0,ra
#else
#define ENTRY(name) \
  .globl name;								      \
  .ent name,0;								      \
  name/**/:;								      \
  .frame sp,0,ra
#endif

#ifdef __STDC__
#define PSEUDO(name, syscall_name, args) \
  ENTRY(name);								      \
  ldiq v0, SYS_##syscall_name;						      \
  .set noat;							    	      \
  call_pal PAL_callsys;							      \
  .set at;							    	      \
  beq a3, 10f;								      \
  br gp, 20f;								      \
20:;									      \
  ldgp gp, 0(gp);							      \
  jmp zero, syscall_error;						      \
10:
#else
#define PSEUDO(name, syscall_name, args) \
  ENTRY(name);								      \
  ldiq v0, SYS_/**/syscall_name;					      \
  .set noat;							    	      \
  call_pal PAL_callsys;							      \
  .set at;							    	      \
  beq a3, 10f;								      \
  br gp, 20f;								      \
20:;									      \
  ldgp gp, 0(gp);							      \
  jmp zero, syscall_error;						      \
10:
#endif

#define ret		ret zero,(ra),1
#define r0		v0
#define r1		a4
#define MOVE(x,y)	mov x, y
