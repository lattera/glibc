/* System call interface code for Sequent Symmetry running Dynix version 3.
   Copyright (C) 1993, 1995, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sysdeps/unix/i386/sysdep.h>

#ifdef	__ASSEMBLER__

/* Get the symbols for system call interrupts.  */
#include <machine/trap.h>

/* Use the BSD versions of system calls, by setting the high 16 bits
   of the syscall number (see /usr/include/syscall.h).  */
#define SYS_HANDLER (SYS_bsd << 16)

/* Dynix uses an interrupt interface to system calls.
   "int $T_SVCn" are syscall interfaces for 0-6 arg functions.
   (see /usr/include/machine/trap.h).  */

#undef	DO_CALL

#ifdef	__STDC__
#define DO_CALL(syscall_name, args) 					      \
  movl $(SYS_HANDLER | SYS_##syscall_name), %eax;			      \
  int $T_SVC##args;
#else
#define DO_CALL(syscall_name, args)					      \
  movl $(SYS_HANDLER | SYS_/**/syscall_name), %eax;			      \
  int $T_SVC/**/args;
#endif

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  .globl syscall_error;							      \
  .align 4;								      \
  ENTRY (name)								      \
  ARGS (args)								      \
  DO_CALL (syscall_name, args)						      \
  jb syscall_error

/* For one and two-argument calls, Dynix takes the arguments in %ecx and
   %edx.  For 3-6 argument calls, Dynix takes the address of the first
   argument in %ecx.  */

#ifdef __STDC__
#define ARGS(n) ARGS_##n
#else
#define ARGS(n) ARGS_/**/n
#endif

#define ARGS_0
#define ARGS_1	movl 4(%esp), %ecx;
#define ARGS_2	movl 4(%esp), %ecx; movl 8(%esp), %edx;
#define ARGS_3	leal 4(%esp), %ecx;
#define ARGS_4	ARGS_3
#define ARGS_5	ARGS_3
#define ARGS_6	ARGS_3

/* Dynix reverses %ecx and %edx relative to most i386 Unices. */

#undef	r1
#define	r1		%ecx	/* Secondary return-value register.  */
#undef	scratch
#define scratch 	%edx	/* Call-clobbered register for random use.  */

#endif	/* __ASSEMBLER__ */
