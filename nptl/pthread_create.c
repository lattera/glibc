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

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pthreadP.h"
#include <hp-timing.h>
#include <ldsodefs.h>
#include <atomic.h>
#include <libc-internal.h>

#include <shlib-compat.h>


/* Local function to start thread and handle cleanup.  */
static int start_thread (void *arg);
/* Similar version used when debugging.  */
static int start_thread_debug (void *arg);


/* Nozero if debugging mode is enabled.  */
int __pthread_debug;

/* Globally enabled events.  */
static td_thr_events_t __nptl_threads_events;

/* Pointer to descriptor with the last event.  */
static struct pthread *__nptl_last_event;

/* Number of threads running.  */
unsigned int __nptl_nthreads = 1;


/* Code to allocate and deallocate a stack.  */
#include "allocatestack.c"

/* Code to create the thread.  */
#include "createthread.c"


/* Table of the key information.  */
struct pthread_key_struct __pthread_keys[PTHREAD_KEYS_MAX]
  __attribute__ ((nocommon));
hidden_def (__pthread_keys)

/* This is for libthread_db only.  */
const int __pthread_pthread_sizeof_descr = sizeof (struct pthread);

struct pthread *
internal_function
__find_in_stack_list (pd)
     struct pthread *pd;
{
  list_t *entry;
  struct pthread *result = NULL;

  lll_lock (stack_cache_lock);

  list_for_each (entry, &stack_used)
    {
      struct pthread *curp;

      curp = list_entry (entry, struct pthread, list);
      if (curp == pd)
	{
	  result = curp;
	  break;
	}
    }

  if (result == NULL)
    list_for_each (entry, &__stack_user)
      {
	struct pthread *curp;

	curp = list_entry (entry, struct pthread, list);
	if (curp == pd)
	  {
	    result = curp;
	    break;
	  }
      }

  lll_unlock (stack_cache_lock);

  return result;
}


/* Deallocate POSIX thread-local-storage.  */
static void
internal_function
deallocate_tsd (struct pthread *pd)
{
  /* Maybe no data was ever allocated.  This happens often so we have
     a flag for this.  */
  if (pd->specific_used)
    {
      size_t round;
      bool found_nonzero;

      for (round = 0, found_nonzero = true;
	   found_nonzero && round < PTHREAD_DESTRUCTOR_ITERATIONS;
	   ++round)
	{
	  size_t cnt;
	  size_t idx;

	  /* So far no new nonzero data entry.  */
	  found_nonzero = false;

	  for (cnt = idx = 0; cnt < PTHREAD_KEY_1STLEVEL_SIZE; ++cnt)
	    if (pd->specific[cnt] != NULL)
	      {
		size_t inner;

		for (inner = 0; inner < PTHREAD_KEY_2NDLEVEL_SIZE;
		     ++inner, ++idx)
		  {
		    void *data = pd->specific[cnt][inner].data;

		    if (data != NULL
			/* Make sure the data corresponds to a valid
			   key.  This test fails if the key was
			   deallocated and also if it was
			   re-allocated.  It is the user's
			   responsibility to free the memory in this
			   case.  */
			&& (pd->specific[cnt][inner].seq
			    == __pthread_keys[idx].seq)
			/* It is not necessary to register a destructor
			   function.  */
			&& __pthread_keys[idx].destr != NULL)
		      {
			pd->specific[cnt][inner].data = NULL;
			__pthread_keys[idx].destr (data);
			found_nonzero = true;
		      }
		  }

		if (cnt != 0)
		  {
		    /* The first block is allocated as part of the thread
		       descriptor.  */
		    free (pd->specific[cnt]);
		    pd->specific[cnt] = NULL;
		  }
		else
		  /* Clear the memory of the first block for reuse.  */
		  memset (&pd->specific_1stblock, '\0',
			  sizeof (pd->specific_1stblock));
	      }
	    else
	      idx += PTHREAD_KEY_1STLEVEL_SIZE;
	}

      pd->specific_used = false;
    }
}


/* Deallocate a thread's stack after optionally making sure the thread
   descriptor is still valid.  */
void
internal_function
__free_tcb (struct pthread *pd)
{
  /* The thread is exiting now.  */
  if (__builtin_expect (atomic_bit_test_set (&pd->cancelhandling,
					     TERMINATED_BIT) == 0, 1))
    {
      /* Remove the descriptor from the list.  */
      if (DEBUGGING_P && __find_in_stack_list (pd) == NULL)
	/* Something is really wrong.  The descriptor for a still
	   running thread is gone.  */
	abort ();

      /* Run the destructor for the thread-local data.  */
      deallocate_tsd (pd);

      /* Queue the stack memory block for reuse and exit the process.  The
	 kernel will signal via writing to the address returned by
	 QUEUE-STACK when the stack is available.  */
      __deallocate_stack (pd);
    }
}


static int
start_thread (void *arg)
{
  /* One more thread.  */
  atomic_increment (&__nptl_nthreads);

  struct pthread *pd = (struct pthread *) arg;

#if HP_TIMING_AVAIL
  /* Remember the time when the thread was started.  */
  hp_timing_t now;
  HP_TIMING_NOW (now);
  THREAD_SETMEM (pd, cpuclock_offset, now);
#endif

  /* This is where the try/finally block should be created.  For
     compilers without that support we do use setjmp.  */
  if (__builtin_expect (setjmp (pd->cancelbuf) == 0, 1))
    {
      /* Run the code the user provided.  */
      THREAD_SETMEM (pd, result, pd->start_routine (pd->arg));
    }

  /* Clean up any state libc stored in thread-local variables.  */
  __libc_thread_freeres ();

  /* If this is the last thread we terminate the process now.  We
     do not notify the debugger, it might just irritate it if there
     is no thread left.  */
  if (__builtin_expect (atomic_decrement_and_test (&__nptl_nthreads), 0))
    /* This was the last thread.  */
    exit (0);

  /* Report the death of the thread if this is wanted.  */
  if (__builtin_expect (pd->report_events, 0))
    {
      /* See whether TD_DEATH is in any of the mask.  */
      const int idx = __td_eventword (TD_DEATH);
      const uint32_t mask = __td_eventmask (TD_DEATH);

      if ((mask & (__nptl_threads_events.event_bits[idx]
		   | pd->eventbuf.eventmask.event_bits[idx])) != 0)
	{
	  /* Yep, we have to signal the death.  Add the descriptor to
	     the list but only if it is not already on it.  */
	  if (pd->nextevent == NULL)
	    {
	      pd->eventbuf.eventnum = TD_DEATH;
	      pd->eventbuf.eventdata = pd;

	      do
		pd->nextevent = __nptl_last_event;
	      while (atomic_compare_and_exchange_acq (&__nptl_last_event, pd,
						      pd->nextevent) != 0);
	    }

	  /* Now call the function to signal the event.  */
	  __nptl_death_event ();
	}
    }

  /* The thread is exiting now.  Don't set this bit until after we've hit
     the event-reporting breakpoint, so that td_thr_get_info on us while at
     the breakpoint reports TD_THR_RUN state rather than TD_THR_ZOMBIE.  */
  atomic_bit_set (&pd->cancelhandling, EXITING_BIT);

  /* If the thread is detached free the TCB.  */
  if (IS_DETACHED (pd))
    /* Free the TCB.  */
    __free_tcb (pd);

  /* We cannot call '_exit' here.  '_exit' will terminate the process.

     The 'exit' implementation in the kernel will signal when the
     process is really dead since 'clone' got passed the CLONE_CLEARTID
     flag.  The 'tid' field in the TCB will be set to zero.

     The exit code is zero since in case all threads exit by calling
     'pthread_exit' the exit status must be 0 (zero).  */
  __exit_thread_inline (0);

  /* NOTREACHED */
  return 0;
}


/* Just list start_thread but we do some more things needed for a run
   with a debugger attached.  */
static int
start_thread_debug (void *arg)
{
  struct pthread *pd = (struct pthread *) arg;

  /* Get the lock the parent locked to force synchronization.  */
  lll_lock (pd->lock);
  /* And give it up right away.  */
  lll_unlock (pd->lock);

  /* Now do the actual startup.  */
  return start_thread (arg);
}


/* Default thread attributes for the case when the user does not
   provide any.  */
static const struct pthread_attr default_attr =
  {
    /* Just some value > 0 which gets rounded to the nearest page size.  */
    .guardsize = 1,
  };


int
__pthread_create_2_1 (newthread, attr, start_routine, arg)
     pthread_t *newthread;
     const pthread_attr_t *attr;
     void *(*start_routine) (void *);
     void *arg;
{
  STACK_VARIABLES;
  const struct pthread_attr *iattr;
  struct pthread *pd;
  int err;

  iattr = (struct pthread_attr *) attr;
  if (iattr == NULL)
    /* Is this the best idea?  On NUMA machines this could mean
       accessing far-away memory.  */
    iattr = &default_attr;

  err = ALLOCATE_STACK (iattr, &pd);
  if (__builtin_expect (err != 0, 0))
    /* Something went wrong.  Maybe a parameter of the attributes is
       invalid or we could not allocate memory.  */
    return err;


  /* Initialize the TCB.  All initializations with zero should be
     performed in 'get_cached_stack'.  This way we avoid doing this if
     the stack freshly allocated with 'mmap'.  */

#ifdef TLS_TCB_AT_TP
  /* Reference to the TCB itself.  */
  pd->header.self = pd;

  /* Self-reference for TLS.  */
  pd->header.tcb = pd;
#endif

  /* Store the address of the start routine and the parameter.  Since
     we do not start the function directly the stillborn thread will
     get the information from its thread descriptor.  */
  pd->start_routine = start_routine;
  pd->arg = arg;

  /* Copy the thread attribute flags.  */
  pd->flags = iattr->flags;

  /* Initialize the field for the ID of the thread which is waiting
     for us.  This is a self-reference in case the thread is created
     detached.  */
  pd->joinid = iattr->flags & ATTR_FLAG_DETACHSTATE ? pd : NULL;

  /* The debug events are inherited from the parent.  */
  pd->eventbuf = THREAD_SELF->eventbuf;


  /* Determine scheduling parameters for the thread.
     XXX How to determine whether scheduling handling is needed?  */
  if (0 && attr != NULL)
    {
      if (iattr->flags & ATTR_FLAG_NOTINHERITSCHED)
	{
	  /* Use the scheduling parameters the user provided.  */
	  pd->schedpolicy = iattr->schedpolicy;
	  memcpy (&pd->schedparam, &iattr->schedparam,
		  sizeof (struct sched_param));
	}
      else
	{
	  /* Just store the scheduling attributes of the parent.  */
	  pd->schedpolicy = __sched_getscheduler (0);
	  __sched_getparam (0, &pd->schedparam);
	}
    }

  /* Pass the descriptor to the caller.  */
  *newthread = (pthread_t) pd;

  /* Start the thread.  */
  err = create_thread (pd, STACK_VARIABLES_ARGS);
  if (err != 0)
    {
      /* Something went wrong.  Free the resources.  */
      __deallocate_stack (pd);
      return err;
    }

  return 0;
}
versioned_symbol (libpthread, __pthread_create_2_1, pthread_create, GLIBC_2_1);


#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
int
__pthread_create_2_0 (newthread, attr, start_routine, arg)
     pthread_t *newthread;
     const pthread_attr_t *attr;
     void *(*start_routine) (void *);
     void *arg;
{
  /* The ATTR attribute is not really of type `pthread_attr_t *'.  It has
     the old size and access to the new members might crash the program.
     We convert the struct now.  */
  struct pthread_attr new_attr;

  if (attr != NULL)
    {
      struct pthread_attr *iattr = (struct pthread_attr *) attr;
      size_t ps = __getpagesize ();

      /* Copy values from the user-provided attributes.  */
      new_attr.schedparam = iattr->schedparam;
      new_attr.schedpolicy = iattr->schedpolicy;
      new_attr.flags = iattr->flags;

      /* Fill in default values for the fields not present in the old
	 implementation.  */
      new_attr.guardsize = ps;
      new_attr.stackaddr = NULL;
      new_attr.stacksize = 0;

      /* We will pass this value on to the real implementation.  */
      attr = (pthread_attr_t *) &new_attr;
    }

  return __pthread_create_2_1 (newthread, attr, start_routine, arg);
}
compat_symbol (libpthread, __pthread_create_2_0, pthread_create,
	       GLIBC_2_0);
#endif
