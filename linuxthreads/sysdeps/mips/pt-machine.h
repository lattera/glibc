/* Machine-dependent pthreads configuration and inline functions.

   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ralf Baechle <ralf@gnu.ai.mit.edu>.
   Based on the Alpha version by Richard Henderson <rth@tamu.edu>.

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
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   TODO: This version makes use of MIPS ISA 2 features.  It won't
   work on ISA 1.  These machines will have to take the overhead of
   a sysmips(MIPS_ATOMIC_SET, ...) syscall which isn't implemented
   yet correctly.  There is however a better solution for R3000
   uniprocessor machines possible.  */

#ifndef PT_EI
# define PT_EI extern inline
#endif


/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  long int ret, temp;

  __asm__ __volatile__(
	"# Inline spinlock test & set\n\t"
	".set\tmips2\n"
	"1:\tll\t%0,%3\n\t"
	"bnez\t%0,2f\n\t"
	".set\tnoreorder\n\t"
	"li\t%1,1\n\t"
	".set\treorder\n\t"
	"sc\t%1,%2\n\t"
	"beqz\t%1,1b\n"
	"2:\t.set\tmips0\n\t"
	"/* End spinlock test & set */"
	: "=&r"(ret), "=&r" (temp), "=m"(*spinlock)
	: "m"(*spinlock)
	: "memory");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("$29");


/* Compare-and-swap for semaphores. */

#define HAS_COMPARE_AND_SWAP
PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  long ret;

  __asm__ __volatile__ (
	"/* Inline compare & swap */\n\t"
	".set\tmips2\n"
	"1:\tll\t%0,%4\n\t"
	".set\tnoreorder\n\t"
	"bne\t%0,%2,2f\n\t"
	"move\t%0,%3\n\t"
	".set\treorder\n\t"
	"sc\t%0,%1\n\t"
	"beqz\t%0,1b\n"
	"2:\t.set\tmips0\n\t"
	"/* End compare & swap */"
	: "=&r"(ret), "=m"(*p)
	: "r"(oldval), "r"(newval), "m"(*p));

  return ret;
}
