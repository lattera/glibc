/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <atomic.h>
#include <ldsodefs.h>
#include <tls.h>


#define CLONE_SIGNAL    	(CLONE_SIGHAND | CLONE_THREAD)

/* Unless otherwise specified, the thread "register" is going to be
   initialized with a pointer to the TCB.  */
#ifndef TLS_VALUE
# define TLS_VALUE pd
#endif

#ifndef ARCH_CLONE
# define ARCH_CLONE __clone
#endif


#ifndef TLS_MULTIPLE_THREADS_IN_TCB
/* Variable set to a nonzero value if more than one thread runs or ran.  */
int __pthread_multiple_threads attribute_hidden;
/* Pointer to the corresponding variable in libc.  */
int *__libc_multiple_threads_ptr attribute_hidden;
#endif


static int
create_thread (struct pthread *pd, STACK_VARIABLES_PARMS)
{
#ifdef PREPARE_CREATE
  PREPARE_CREATE;
#endif

#ifdef TLS_TCB_AT_TP
  assert (pd->header.tcb != NULL);
#endif

  if (__builtin_expect (THREAD_GETMEM (THREAD_SELF, report_events), 0))
    {
      /* The parent thread is supposed to report events.  Check whether
	 the TD_CREATE event is needed, too.  */
      const int _idx = __td_eventword (TD_CREATE);
      const uint32_t _mask = __td_eventmask (TD_CREATE);

      if ((_mask & (__nptl_threads_events.event_bits[_idx]
		    | pd->eventbuf.eventmask.event_bits[_idx])) != 0)
	{
	  /* We have to report the new thread.  Make sure the thread
	     does not run far by forcing it to get a lock.  We lock it
	     here too so that the new thread cannot continue until we
	     tell it to.  */
	  lll_lock (pd->lock);

	  /* Create the thread.  */
	  if (ARCH_CLONE (start_thread_debug, STACK_VARIABLES_ARGS,
			  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGNAL |
			  CLONE_SETTLS | CLONE_PARENT_SETTID |
			  CLONE_CHILD_CLEARTID | CLONE_DETACHED | 0,
			  pd, &pd->tid, TLS_VALUE, &pd->tid) == -1)
	    /* Failed.  */
	    return errno;

#ifdef TLS_MULTIPLE_THREADS_IN_TCB
	  /* We now have for sure more than one thread.  */
	  pd->header.multiple_threads = 1;
#else
	  __pthread_multiple_threads = *__libc_multiple_threads_ptr = 1;
#endif

	  /* Now fill in the information about the new thread in
	     the newly created thread's data structure.  We cannot let
	     the new thread do this since we don't know whether it was
	     already scheduled when we send the event.  */
	  pd->eventbuf.eventnum = TD_CREATE;
	  pd->eventbuf.eventdata = pd;

	  /* Enqueue the descriptor.  */
	  do
	    pd->nextevent = __nptl_last_event;
	  while (atomic_compare_and_exchange_bool_acq (&__nptl_last_event, pd,
						       pd->nextevent) != 0);

	  /* Now call the function which signals the event.  */
	  __nptl_create_event ();

	  /* And finally restart the new thread.  */
	  lll_unlock (pd->lock);

	  return 0;
	}
    }

#ifdef NEED_DL_SYSINFO
  assert (THREAD_GETMEM (THREAD_SELF, header.sysinfo) == pd->header.sysinfo);
#endif

  /* We rely heavily on various flags the CLONE function understands:

     CLONE_VM, CLONE_FS, CLONE_FILES
	These flags select semantics with shared address space and
	file descriptors according to what POSIX requires.

     CLONE_SIGNAL
	This flag selects the POSIX signal semantics.

     CLONE_SETTLS
	The sixth parameter to CLONE determines the TLS area for the
	new thread.

     CLONE_PARENT_SETTID
	The kernels writes the thread ID of the newly created thread
	into the location pointed to by the fifth parameters to CLONE.

	Note that it would be semantically equivalent to use
	CLONE_CHILD_SETTID but it is be more expensive in the kernel.

     CLONE_CHILD_CLEARTID
	The kernels clears the thread ID of a thread that has called
	sys_exit() in the location pointed to by the seventh parameter
	to CLONE.

     CLONE_DETACHED
	No signal is generated if the thread exists and it is
	automatically reaped.

     The termination signal is chosen to be zero which means no signal
     is sent.  */
  if (ARCH_CLONE (start_thread, STACK_VARIABLES_ARGS,
		  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGNAL |
		  CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID |
		  CLONE_DETACHED | 0, pd, &pd->tid, TLS_VALUE, &pd->tid) == -1)
    /* Failed.  */
    return errno;

#ifdef TLS_MULTIPLE_THREADS_IN_TCB
  /* We now have for sure more than one thread.  */
  THREAD_SETMEM (THREAD_SELF, header.multiple_threads, 1);
#endif

  return 0;
}
