/* Copyright (C) 1993,1994,1995,1997,1998,2002 Free Software Foundation, Inc.
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

/* Solaris 2 does not precede the asm names of C symbols with a `_'. */
#ifndef NO_UNDERSCORES
#define	NO_UNDERSCORES
#endif

#include <sysdeps/unix/sysdep.h>

#ifdef	__ASSEMBLER__

/* As of gcc-2.6.0, it complains about pound signs in front of things
   that aren't arguments to the macro.  So we use this to pull it off
   instead.  */
#define cat(a,b) a##b
#define poundfnc cat(#,function)

#define	ENTRY(name)							      \
  .section ".text";							      \
  .align 4;								      \
  .global C_SYMBOL_NAME(name);						      \
  .type  C_SYMBOL_NAME(name), poundfnc;					      \
  C_LABEL(name)

#define	PSEUDO(name, syscall_name, args)				      \
  ENTRY (name)								      \
  mov SYS_ify(syscall_name), %g1;				   	      \
  ta 8;									      \
  bcs C_SYMBOL_NAME(syscall_error);					      \
  nop

#define	ret		retl; nop
#define	r0		%o0
#define	r1		%o1
#define	MOVE(x,y)	mov x, y

#define LOC(name)	.L##name

#endif	/* __ASSEMBLER__ */
