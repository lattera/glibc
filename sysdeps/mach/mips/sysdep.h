/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#define LOSE asm volatile ("1: b 1b")

#define START_MACHDEP asm ("\
	.text\n\
	.globl _start\n\
	.ent _start\n\
_start:\n\
	# Put initial SP in a0.\n\
	move $4, $29\n\
	# Jump to _start0; don't return.\n\
	j _start0\n\
	.end _start\n\
");
#define START_ARGS	int *entry_sp
#define SNARF_ARGS(argc, argv, envp)					      \
  do									      \
    {									      \
      register char **p;						      \
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

#define RETURN_TO(sp, pc, retval) \
  asm volatile ("move $29, %0; move $2, %2; move $25, %1; jr $25" \
		: : "r" (sp), "r" (pc), "r" (retval))

#define STACK_GROWTH_DOWN

#include <syscall.h>

#if defined (__ASSEMBLER__)

#define ALIGN	2

#define MOVE(x,y)	move y , x

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

#include <sysdeps/mach/sysdep.h>
