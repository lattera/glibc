/* Machine-dependent pthreads configuration and inline functions.
   i686 version.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

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


/* Spinlock implementation; required.  */
extern inline int
testandset (int *spinlock)
{
  int ret;

  __asm__ __volatile__("xchgl %0, %1"
	: "=r"(ret), "=m"(*spinlock)
	: "0"(1), "m"(*spinlock));

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%esp");


/* Compare-and-swap for semaphores.  It's always available on i686.  */
#define HAS_COMPARE_AND_SWAP

extern inline int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("lock; cmpxchgl %3, %1; sete %0"
			: "=q" (ret), "=m" (*p), "=a" (readval)
			: "r" (newval), "m" (*p), "a" (oldval));
  return ret;
}


extern inline int
get_eflags (void)
{
  int res;
  __asm__ __volatile__ ("pushfl; popl %0" : "=r" (res) : );
  return res;
}


extern inline void
set_eflags (int newflags)
{
  __asm__ __volatile__ ("pushl %0; popfl" : : "r" (newflags) : "cc");
}
