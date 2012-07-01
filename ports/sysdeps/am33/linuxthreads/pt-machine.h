/* Machine-dependent pthreads configuration and inline functions.
   am33 version.
   Copyright (C) 1996,1997,1998,1999,2000,2001, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   Based on ../i386/pt-machine.h.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H	1

#ifndef __ASSEMBLER__
#ifndef PT_EI
# define PT_EI extern inline
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  __builtin_frame_address (0)

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  long int ret = 1;

  /* This won't test&set the entire int, only the least significant
     byte.  I hope this doesn't matter, since we can't do better.  */
  __asm__ __volatile__ ("bset %0, %1; bne 1f; clr %0; 1:" :
			"+d" (ret), "+m" (*(volatile int *)spinlock));

  return ret;
}


PT_EI int
get_eflags (void)
{
  int res;
  __asm__ __volatile__ ("mov psw,%0" : "=d" (res));
  return res;
}


PT_EI void
set_eflags (int newflags)
{
  __asm__ __volatile__ ("mov %0,psw" : : "d" (newflags) : "cc");
}

#endif /* __ASSEMBLER__ */

#endif /* pt-machine.h */
