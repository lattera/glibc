/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

#define MOVE(x,y)	move y , x

#if 0
#define LOSE asm volatile ("1: b 1b")
#endif

#define SNARF_ARGS(argc, argv, envp)					      \
  do									      \
    {									      \
      int *entry_sp;							      \
      register char **p;						      \
									      \
      asm ("addu %0,$30,4" : "=r" (entry_sp));				      \
									      \
      argc = *entry_sp;							      \
      argv = (char **) (entry_sp + 1);					      \
      p = argv;								      \
      while (*p++ != NULL)						      \
	;								      \
      if (p >= (char **) argv[0])					      \
	--p;								      \
      envp = p;							      \
    } while (0)

#define CALL_WITH_SP(fn, sp) \
  ({ register int __fn = fn, __sp = (int) sp; \
     asm volatile ("move $sp,%0; j %1" : : "r" (__sp), "r" (__fn));})

#define STACK_GROWTH_DOWN

#ifdef P40
#include <syscall.h>

#define SYSCALL(name, args)	\
  .globl syscall_error;	\
  kernel_trap(name,SYS_##name,args);	\
  beq $1,$0,1f;	\
  j syscall_error;	\
1:

#define SYSCALL__(name, args)	\
  .globl syscall_error;	\
  kernel_trap(__##name,SYS_##name,args);	\
  beq $1,$0,1f;	\
  j syscall_error;	\
1:

#define ret	j ra; nop
#endif

#include_next <sysdep.h>
