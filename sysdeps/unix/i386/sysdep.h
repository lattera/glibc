/* Copyright (C) 1991, 92, 93, 95, 96 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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
#include <sysdeps/i386/sysdep.h>

#ifdef	ASSEMBLER

/* This is defined as a separate macro so that other sysdep.h files
   can include this one and then redefine DO_CALL.  */

#define DO_CALL(syscall_name, args)					      \
  lea SYS_ify (syscall_name), %eax;					      \
  /* lcall $7, $0; */							      \
  /* Above loses; GAS bug.  */						      \
  .byte 0x9a, 0, 0, 0, 0, 7, 0

#define	r0		%eax	/* Normal return-value register.  */
#define	r1		%edx	/* Secondary return-value register.  */
#define scratch 	%ecx	/* Call-clobbered register for random use.  */
#define MOVE(x,y)	movl x, y

#endif	/* ASSEMBLER */
