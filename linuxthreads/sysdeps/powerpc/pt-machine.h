/* Machine-dependent pthreads configuration and inline functions.
   powerpc version.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* These routines are from Appendix G of the 'PowerPC 601 RISC Microprocessor
   User's Manual', by IBM and Motorola.  */

#ifndef PT_EI
# define PT_EI extern inline
#endif

/* For multiprocessor systems, we want to ensure all memory accesses
   are completed before we reset a lock.  */
#if 0
/* on non multiprocessor systems, you can just: */
#define sync() /* nothing */
#else
#define sync() __asm__ __volatile__ ("sync")
#endif

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("r1");

/* Compare-and-swap for semaphores. */
/* note that test-and-set(x) is the same as compare-and-swap(x, 0, 1) */

#define HAS_COMPARE_AND_SWAP
#if BROKEN_PPC_ASM_CR0
static
#else
PT_EI
#endif
int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  int ret;

  sync();
  __asm__ __volatile__(
		       "0:    lwarx %0,0,%1 ;"
		       "      xor. %0,%3,%0;"
		       "      bne 1f;"
		       "      stwcx. %2,0,%1;"
		       "      bne- 0b;"
		       "1:    "
	: "=&r"(ret)
	: "r"(p), "r"(newval), "r"(oldval)
	: "cr0", "memory");
  sync();
  return ret == 0;
}
