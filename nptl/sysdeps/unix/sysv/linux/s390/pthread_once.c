/* Copyright (C) 2003, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "pthreadP.h"
#include <lowlevellock.h>


unsigned long int __fork_generation attribute_hidden;


static void
clear_once_control (void *arg)
{
  pthread_once_t *once_control = (pthread_once_t *) arg;

  *once_control = 0;
  lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);
}


int
__pthread_once (once_control, init_routine)
     pthread_once_t *once_control;
     void (*init_routine) (void);
{
  while (1)
    {
      int oldval;
      int newval;

      /* Pseudo code:
	   oldval = *once_control;
	   if ((oldval & 2) == 0)
	    {
	      newval = (oldval & 3) | __fork_generation | 1;
	      *once_control = newval;
	    }
	 Do this atomically.  */
      __asm __volatile ("   l	 %1,%0\n"
			"0: lhi	 %2,2\n"
			"   tml	 %1,2\n"
			"   jnz	 1f\n"
			"   nr	 %2,%1\n"
			"   ahi	 %2,1\n"
			"   o	 %2,%3\n"
			"   cs	 %1,%2,%0\n"
			"   jl	 0b\n"
			"1:"
			: "=Q" (*once_control), "=&d" (oldval), "=&d" (newval)
			: "m" (__fork_generation), "m" (*once_control)
			: "cc" );
      /* Check if the initialized has already been done.  */
      if ((oldval & 2) != 0)
	  break;
      /* Check if another thread already runs the initializer.	*/
      if ((oldval & 1) != 0)
	{
	  /* Check whether the initializer execution was interrupted
	     by a fork.	 */
	  if (((oldval ^ newval) & -4) == 0)
	    {
	      /* Same generation, some other thread was faster. Wait.  */
	      lll_futex_wait (once_control, newval, LLL_PRIVATE);
	      continue;
	    }
	}

      /* This thread is the first here.  Do the initialization.
	 Register a cleanup handler so that in case the thread gets
	 interrupted the initialization can be restarted.  */
      pthread_cleanup_push (clear_once_control, once_control);

      init_routine ();

      pthread_cleanup_pop (0);


      /* Add one to *once_control.  */
      __asm __volatile ("   l	 %1,%0\n"
			"0: lr	 %2,%1\n"
			"   ahi	 %2,1\n"
			"   cs	 %1,%2,%0\n"
			"   jl	 0b\n"
			: "=Q" (*once_control), "=&d" (oldval), "=&d" (newval)
			: "m" (*once_control) : "cc" );

      /* Wake up all other threads.  */
      lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);
      break;
    }

  return 0;
}
weak_alias (__pthread_once, pthread_once)
strong_alias (__pthread_once, __pthread_once_internal)
