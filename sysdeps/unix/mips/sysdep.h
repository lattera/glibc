/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <regdef.h>

#ifdef __STDC__
#define ENTRY(name) \
  .globl name;								      \
  .align 2;								      \
  name##:
#else
#define ENTRY(name) \
  .globl name;								      \
  .align 2;								      \
  name/**/:
#endif

/* Note that while it's better structurally, going back to call syscall_error
   can make things confusing if you're debugging---it looks like it's jumping
   backwards into the previous fn.  */
#ifdef __STDC__
#define PSEUDO(name, syscall_name, args) \
  .set noreorder;							      \
  .align 2;								      \
  99: j syscall_error;							      \
  nop;							      		      \
  ENTRY(name)								      \
  li v0, SYS_##syscall_name;						      \
  syscall;								      \
  bne a3, zero, 99b;							      \
  nop;									      \
syse1:
#else
#define PSEUDO(name, syscall_name, args) \
  .set noreorder;							      \
  .align 2;								      \
  99: j syscall_error;							      \
  nop;							      		      \
  ENTRY(name)								      \
  li v0, SYS_/**/syscall_name;						      \
  syscall;								      \
  bne a3, zero, 99b;							      \
  nop;									      \
syse1:
#endif

#define ret	j ra ; nop
#define r0	v0
#define r1	v1
/* The mips move insn is d,s.  */
#define MOVE(x,y)	move y , x
