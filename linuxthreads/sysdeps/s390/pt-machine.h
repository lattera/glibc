/* Machine-dependent pthreads configuration and inline functions.
   S390 version.
   Copyright (C) 1999 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

#ifndef PT_EI
# define PT_EI extern inline
#endif

/* Spinlock implementation; required.  */
PT_EI int
testandset (int *spinlock)
{
  int ret;

  __asm__ __volatile__(
       "    lhi   0,1\n"
       "    l     %0,%1\n"
       "0:  cs    %0,0,%1\n"
       "    jl    0b"
       : "=&d" (ret), "+m" (*spinlock)
       : : "0", "cc");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("15");


/* Compare-and-swap for semaphores. */

#define HAS_COMPARE_AND_SWAP

PT_EI int
__compare_and_swap(long int *p, long int oldval, long int newval)
{
        int retval;

        __asm__ __volatile__(
                "  lr   0,%2\n"
                "  cs   0,%3,%1\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "+m" (*p)
                : "d" (oldval) , "d" (newval)
                : "cc", "0");
        return retval == 0;
}



/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%r15");
