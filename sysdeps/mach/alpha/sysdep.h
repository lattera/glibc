/* Copyright (C) 1994,97,2002 Free Software Foundation, Inc.
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

#define MOVE(x,y)	mov x, y

#define LOSE asm volatile ("call_pal 0") /* halt */

#define START_MACHDEP \
  asm ("_start:	mov	$30, $16\n" /* Put initial SP in a0.  */	      \
       "	br	$27, 1f\n" /* Load GP from PC.  */		      \
       "1:	ldgp	$29, 0($27)\n"					      \
       "	jmp	$26, _start0");	/* Jump to _start0; don't return.  */
#define START_ARGS	char **sparg
#define SNARF_ARGS(argc, argv, envp) \
  (envp = &(argv = &sparg[1])[(argc = *(int *) sparg) + 1])

#define CALL_WITH_SP(fn, sp) \
  ({ register long int __fn = (long int) fn, __sp = (long int) sp; \
     asm volatile ("mov %0,$30; jmp $31, (%1); ldgp $29, 0(%1)" \
		   : : "r" (__sp), "r" (__fn)); })

#define STACK_GROWTH_DOWN

#define RETURN_TO(sp, pc, retval) \
  asm volatile ("mov %0,$30; jmp $31, (%1); mov %2,$0" \
		: : "r" (sp), "r" (pc), "r" ((long int) (retval)));

#define ALIGN 3
#include <sysdeps/mach/sysdep.h>

/* Alpha needs the .ent and .frame magic that the generic version lacks.  */
#undef ENTRY
#define ENTRY(name)				\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  name##:					\
  .frame sp, 0, ra

#include <mach/alpha/asm.h>
#undef	at
#define at	28
#define AT	$28
#define fp	s6
