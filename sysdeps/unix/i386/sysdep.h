/* Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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

#ifdef	ASSEMBLER

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),@function)			      \
  .align 4;								      \
  C_LABEL(name)

/* For ELF we need the `.type' directive to make shared libs work right.  */
#ifdef HAVE_ELF
#define ASM_TYPE_DIRECTIVE(name,typearg)	.type name,typearg;
#else
#define ASM_TYPE_DIRECTIVE(name,type) /* Nothing is specified.  */
#endif

#ifdef	NO_UNDERSCORES
/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#endif

#define	PSEUDO(name, syscall_name, args)				      \
lose: SYSCALL_PIC_SETUP							      \
  jmp JUMPTARGET(syscall_error)						      \
  .globl syscall_error;							      \
  ENTRY (name)								      \
  DO_CALL (syscall_name, args);						      \
  jb lose

#ifdef PIC
#define JUMPTARGET(name)	name##@PLT
#define SYSCALL_PIC_SETUP \
    pushl %ebx;								      \
    call 0f;								      \
0:  popl %ebx;								      \
    addl $_GLOBAL_OFFSET_TABLE+[.-0b], %ebx;
#else
#define JUMPTARGET(name)	name
#define SYSCALL_PIC_SETUP	/* Nothing.  */
#endif

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
