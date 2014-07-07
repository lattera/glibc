/* Copyright (C) 2003-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "pthreadP.h"
#include <lowlevellock.h>
#include <atomic.h>


unsigned long int __fork_generation attribute_hidden;


static void
clear_once_control (void *arg)
{
  pthread_once_t *once_control = (pthread_once_t *) arg;

  /* Reset to the uninitialized state here.  We don't need a stronger memory
     order because we do not need to make any other of our writes visible to
     other threads that see this value: This function will be called if we
     get interrupted (see __pthread_once), so all we need to relay to other
     threads is the state being reset again.  */
  *once_control = 0;
  lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);
}


/* This is similar to a lock implementation, but we distinguish between three
   states: not yet initialized (0), initialization finished (2), and
   initialization in progress (__fork_generation | 1).  If in the first state,
   threads will try to run the initialization by moving to the second state;
   the first thread to do so via a CAS on once_control runs init_routine,
   other threads block.
   When forking the process, some threads can be interrupted during the second
   state; they won't be present in the forked child, so we need to restart
   initialization in the child.  To distinguish an in-progress initialization
   from an interrupted initialization (in which case we need to reclaim the
   lock), we look at the fork generation that's part of the second state: We
   can reclaim iff it differs from the current fork generation.
   XXX: This algorithm has an ABA issue on the fork generation: If an
   initialization is interrupted, we then fork 2^30 times (30 bits of
   once_control are used for the fork generation), and try to initialize
   again, we can deadlock because we can't distinguish the in-progress and
   interrupted cases anymore.  */
int
__pthread_once (once_control, init_routine)
     pthread_once_t *once_control;
     void (*init_routine) (void);
{
  while (1)
    {
      int oldval, val, newval;

      /* We need acquire memory order for this load because if the value
         signals that initialization has finished, we need to be see any
         data modifications done during initialization.  */
      val = *once_control;
      atomic_read_barrier();
      do
	{
	  /* Check if the initialization has already been done.  */
	  if (__glibc_likely ((val & 2) != 0))
	    return 0;

	  oldval = val;
	  /* We try to set the state to in-progress and having the current
	     fork generation.  We don't need atomic accesses for the fork
	     generation because it's immutable in a particular process, and
	     forked child processes start with a single thread that modified
	     the generation.  */
	  newval = __fork_generation | 1;
	  /* We need acquire memory order here for the same reason as for the
	     load from once_control above.  */
	  val = atomic_compare_and_exchange_val_acq (once_control, newval,
						     oldval);
	}
      while (__glibc_unlikely (val != oldval));

      /* Check if another thread already runs the initializer.	*/
      if ((oldval & 1) != 0)
	{
	  /* Check whether the initializer execution was interrupted by a
	     fork.  We know that for both values, bit 0 is set and bit 1 is
	     not.  */
	  if (oldval == newval)
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


      /* Mark *once_control as having finished the initialization.  We need
         release memory order here because we need to synchronize with other
         threads that want to use the initialized data.  */
      atomic_write_barrier();
      *once_control = 2;

      /* Wake up all other threads.  */
      lll_futex_wake (once_control, INT_MAX, LLL_PRIVATE);
      break;
    }

  return 0;
}
weak_alias (__pthread_once, pthread_once)
hidden_def (__pthread_once)
