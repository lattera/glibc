/* Machine-dependent pthreads configuration and inline functions.
   SuperH version.
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Niibe Yutaka <gniibe@m17n.org>.

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
       "tas.b	@%1\n\t"
       "movt	%0"
       : "=r" (ret)
       : "r" (spinlock)
       : "memory", "cc");

  return (ret == 0);
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("r15");

/* Return the thread descriptor for the current thread.  */
struct _pthread_descr_struct;
#define THREAD_SELF \
  ({ struct _pthread_descr_struct *self; \
      __asm__("stc gbr,%0" : "=r" (self)); self;})

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr) \
  ({ __asm__("ldc %0,gbr" : : "r" (descr));})
