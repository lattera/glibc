/* Copyright (C) 1992, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdeps/unix/sysdep.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#ifdef __STDC__
# define SYS_ify(syscall_name)	__NR_##syscall_name
#else
# define SYS_ify(syscall_name)	__NR_/**/syscall_name
#endif

#ifdef ASSEMBLER

#define ENTRY(name)                                                           \
  .globl name;							              \
  .type name,@function;						              \
  .align 2;								      \
  C_LABEL(name)

#define DO_CALL(syscall)				      		      \
    li 0,syscall;						              \
    sc

#ifdef PIC
#define PSEUDO(name, syscall_name, args)				      \
  .section ".text";							      \
  ENTRY (name)								      \
    DO_CALL (SYS_ify (syscall_name));					      \
    bnslr;								      \
    b __syscall_error@plt
#else
#define PSEUDO(name, syscall_name, args)                                      \
  .section ".text";							      \
  ENTRY (name)                                                                \
    DO_CALL (SYS_ify (syscall_name));				              \
    bnslr;                                                                    \
    b __syscall_error
#endif

#define ret	/* Nothing (should be 'blr', but never reached).  */

#endif	/* ASSEMBLER */
