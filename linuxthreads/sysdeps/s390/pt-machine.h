/* Machine-dependent pthreads configuration and inline functions.
   S390 version.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
PT_EI long int
testandset (int *spinlock)
{
  int ret;

  __asm__ __volatile__(
       "    la    1,%1\n"
       "    lhi   0,1\n"
       "    l     %0,%1\n"
       "0:  cs    %0,0,0(1)\n"
       "    jl    0b"
       : "=&d" (ret), "+m" (*spinlock)
       : : "0", "1", "cc");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("15");

/* Return the thread descriptor for the current thread.
   S/390 registers uses access register 0 as "thread register".  */
#define THREAD_SELF  ({                                                       \
  register pthread_descr __self;                                              \
  __asm__ ("ear %0,%%a0" : "=d" (__self) );                                   \
  __self;                                                                     \
})

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr)  ({                                       \
  __asm__ ("sar %%a0,%0" : : "d" (descr) );                                   \
})

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) THREAD_SELF->member
#define THREAD_GETMEM_NC(descr, member) THREAD_SELF->member
#define THREAD_SETMEM(descr, member, value) THREAD_SELF->member = (value)
#define THREAD_SETMEM_NC(descr, member, value) THREAD_SELF->member = (value)


/* Compare-and-swap for semaphores. */

#define HAS_COMPARE_AND_SWAP

PT_EI int
__compare_and_swap(long int *p, long int oldval, long int newval)
{
        int retval;

        __asm__ __volatile__(
                "  la   1,%1\n"
                "  lr   0,%2\n"
                "  cs   0,%3,0(1)\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "+m" (*p)
                : "d" (oldval) , "d" (newval)
                : "cc", "0", "1" );
        return retval == 0;
}



/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%r15");
