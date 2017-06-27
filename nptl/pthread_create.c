/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pthreadP.h"
#include <hp-timing.h>
#include <ldsodefs.h>
#include <atomic.h>
#include <libc-internal.h>
#include <resolv.h>
#include <kernel-features.h>
#include <exit-thread.h>
#include <default-sched.h>
#include <futex-internal.h>
#include <tls-setup.h>
#include "libioP.h"

#include <shlib-compat.h>

#include <stap-probe.h>


/* Nozero if debugging mode is enabled.  */
int __pthread_debug;

/* Globally enabled events.  */
static td_thr_events_t __nptl_threads_events __attribute_used__;

/* Pointer to descriptor with the last event.  */
static struct pthread *__nptl_last_event __attribute_used__;

/* Number of threads running.  */
unsigned int __nptl_nthreads = 1;


/* Code to allocate and deallocate a stack.  */
#include "allocatestack.c"

/* CONCURRENCY NOTES:

   Understanding who is the owner of the 'struct pthread' or 'PD'
   (refers to the value of the 'struct pthread *pd' function argument)
   is critically important in determining exactly which operations are
   allowed and which are not and when, particularly when it comes to the
   implementation of pthread_create, pthread_join, pthread_detach, and
   other functions which all operate on PD.

   The owner of PD is responsible for freeing the final resources
   associated with PD, and may examine the memory underlying PD at any
   point in time until it frees it back to the OS or to reuse by the
   runtime.

   The thread which calls pthread_create is called the creating thread.
   The creating thread begins as the owner of PD.

   During startup the new thread may examine PD in coordination with the
   owner thread (which may be itself).

   The four cases of ownership transfer are:

   (1) Ownership of PD is released to the process (all threads may use it)
       after the new thread starts in a joinable state
       i.e. pthread_create returns a usable pthread_t.

   (2) Ownership of PD is released to the new thread starting in a detached
       state.

   (3) Ownership of PD is dynamically released to a running thread via
       pthread_detach.

   (4) Ownership of PD is acquired by the thread which calls pthread_join.

   Implementation notes:

   The PD->stopped_start and thread_ran variables are used to determine
   exactly which of the four ownership states we are in and therefore
   what actions can be taken.  For example after (2) we cannot read or
   write from PD anymore since the thread may no longer exist and the
   memory may be unmapped.

   It is important to point out that PD->lock is being used both
   similar to a one-shot semaphore and subsequently as a mutex.  The
   lock is taken in the parent to force the child to wait, and then the
   child releases the lock.  However, this semaphore-like effect is used
   only for synchronizing the parent and child.  After startup the lock
   is used like a mutex to create a critical section during which a
   single owner modifies the thread parameters.

   The most complicated cases happen during thread startup:

   (a) If the created thread is in a detached (PTHREAD_CREATE_DETACHED),
       or joinable (default PTHREAD_CREATE_JOINABLE) state and
       STOPPED_START is true, then the creating thread has ownership of
       PD until the PD->lock is released by pthread_create.  If any
       errors occur we are in states (c), (d), or (e) below.

   (b) If the created thread is in a detached state
       (PTHREAD_CREATED_DETACHED), and STOPPED_START is false, then the
       creating thread has ownership of PD until it invokes the OS
       kernel's thread creation routine.  If this routine returns
       without error, then the created thread owns PD; otherwise, see
       (c) and (e) below.

   (c) If the detached thread setup failed and THREAD_RAN is true, then
       the creating thread releases ownership to the new thread by
       sending a cancellation signal.  All threads set THREAD_RAN to
       true as quickly as possible after returning from the OS kernel's
       thread creation routine.

   (d) If the joinable thread setup failed and THREAD_RAN is true, then
       then the creating thread retains ownership of PD and must cleanup
       state.  Ownership cannot be released to the process via the
       return of pthread_create since a non-zero result entails PD is
       undefined and therefore cannot be joined to free the resources.
       We privately call pthread_join on the thread to finish handling
       the resource shutdown (Or at least we should, see bug 19511).

   (e) If the thread creation failed and THREAD_RAN is false, then the
       creating thread retains ownership of PD and must cleanup state.
       No waiting for the new thread is required because it never
       started.

   The nptl_db interface:

   The interface with nptl_db requires that we enqueue PD into a linked
   list and then call a function which the debugger will trap.  The PD
   will then be dequeued and control returned to the thread.  The caller
   at the time must have ownership of PD and such ownership remains
   after control returns to thread. The enqueued PD is removed from the
   linked list by the nptl_db callback td_thr_event_getmsg.  The debugger
   must ensure that the thread does not resume execution, otherwise
   ownership of PD may be lost and examining PD will not be possible.

   Note that the GNU Debugger as of (December 10th 2015) commit
   c2c2a31fdb228d41ce3db62b268efea04bd39c18 no longer uses
   td_thr_event_getmsg and several other related nptl_db interfaces. The
   principal reason for this is that nptl_db does not support non-stop
   mode where other threads can run concurrently and modify runtime
   structures currently in use by the debugger and the nptl_db
   interface.

   Axioms:

   * The create_thread function can never set stopped_start to false.
   * The created thread can read stopped_start but never write to it.
   * The variable thread_ran is set some time after the OS thread
     creation routine returns, how much time after the thread is created
     is unspecified, but it should be as quickly as possible.

*/

/* CREATE THREAD NOTES:

   createthread.c defines the create_thread function, and two macros:
   START_THREAD_DEFN and START_THREAD_SELF (see below).

   create_thread must initialize PD->stopped_start.  It should be true
   if the STOPPED_START parameter is true, or if create_thread needs the
   new thread to synchronize at startup for some other implementation
   reason.  If STOPPED_START will be true, then create_thread is obliged
   to lock PD->lock before starting the thread.  Then pthread_create
   unlocks PD->lock which synchronizes-with START_THREAD_DEFN in the
   child thread which does an acquire/release of PD->lock as the last
   action before calling the user entry point.  The goal of all of this
   is to ensure that the required initial thread attributes are applied
   (by the creating thread) before the new thread runs user code.  Note
   that the the functions pthread_getschedparam, pthread_setschedparam,
   pthread_setschedprio, __pthread_tpp_change_priority, and
   __pthread_current_priority reuse the same lock, PD->lock, for a
   similar purpose e.g. synchronizing the setting of similar thread
   attributes.  These functions are never called before the thread is
   created, so don't participate in startup syncronization, but given
   that the lock is present already and in the unlocked state, reusing
   it saves space.

   The return value is zero for success or an errno code for failure.
   If the return value is ENOMEM, that will be translated to EAGAIN,
   so create_thread need not do that.  On failure, *THREAD_RAN should
   be set to true iff the thread actually started up and then got
   canceled before calling user code (*PD->start_routine).  */
static int create_thread (struct pthread *pd, const struct pthread_attr *attr,
			  bool *stopped_start, STACK_VARIABLES_PARMS,
			  bool *thread_ran);

#include <createthread.c>


struct pthread *
__find_in_stack_list (struct pthread *pd)
{
  list_t *entry;
  struct pthread *result = NULL;

  lll_lock (stack_cache_lock, LLL_PRIVATE);

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

  lll_unlock (stack_cache_lock, LLL_PRIVATE);

  return result;
}


/* Deallocate POSIX thread-local-storage.  */
void
attribute_hidden
__nptl_deallocate_tsd (void)
{
  struct pthread *self = THREAD_SELF;

  /* Maybe no data was ever allocated.  This happens often so we have
     a flag for this.  */
  if (THREAD_GETMEM (self, specific_used))
    {
      size_t round;
      size_t cnt;

      round = 0;
      do
	{
	  size_t idx;

	  /* So far no new nonzero data entry.  */
	  THREAD_SETMEM (self, specific_used, false);

	  for (cnt = idx = 0; cnt < PTHREAD_KEY_1STLEVEL_SIZE; ++cnt)
	    {
	      struct pthread_key_data *level2;

	      level2 = THREAD_GETMEM_NC (self, specific, cnt);

	      if (level2 != NULL)
		{
		  size_t inner;

		  for (inner = 0; inner < PTHREAD_KEY_2NDLEVEL_SIZE;
		       ++inner, ++idx)
		    {
		      void *data = level2[inner].data;

		      if (data != NULL)
			{
			  /* Always clear the data.  */
			  level2[inner].data = NULL;

			  /* Make sure the data corresponds to a valid
			     key.  This test fails if the key was
			     deallocated and also if it was
			     re-allocated.  It is the user's
			     responsibility to free the memory in this
			     case.  */
			  if (level2[inner].seq
			      == __pthread_keys[idx].seq
			      /* It is not necessary to register a destructor
				 function.  */
			      && __pthread_keys[idx].destr != NULL)
			    /* Call the user-provided destructor.  */
			    __pthread_keys[idx].destr (data);
			}
		    }
		}
	      else
		idx += PTHREAD_KEY_1STLEVEL_SIZE;
	    }

	  if (THREAD_GETMEM (self, specific_used) == 0)
	    /* No data has been modified.  */
	    goto just_free;
	}
      /* We only repeat the process a fixed number of times.  */
      while (__builtin_expect (++round < PTHREAD_DESTRUCTOR_ITERATIONS, 0));

      /* Just clear the memory of the first block for reuse.  */
      memset (&THREAD_SELF->specific_1stblock, '\0',
	      sizeof (self->specific_1stblock));

    just_free:
      /* Free the memory for the other blocks.  */
      for (cnt = 1; cnt < PTHREAD_KEY_1STLEVEL_SIZE; ++cnt)
	{
	  struct pthread_key_data *level2;

	  level2 = THREAD_GETMEM_NC (self, specific, cnt);
	  if (level2 != NULL)
	    {
	      /* The first block is allocated as part of the thread
		 descriptor.  */
	      free (level2);
	      THREAD_SETMEM_NC (self, specific, cnt, NULL);
	    }
	}

      THREAD_SETMEM (self, specific_used, false);
    }
}


/* Deallocate a thread's stack after optionally making sure the thread
   descriptor is still valid.  */
void
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

      /* Free TPP data.  */
      if (__glibc_unlikely (pd->tpp != NULL))
	{
	  struct priority_protection_data *tpp = pd->tpp;

	  pd->tpp = NULL;
	  free (tpp);
	}

      /* Queue the stack memory block for reuse and exit the process.  The
	 kernel will signal via writing to the address returned by
	 QUEUE-STACK when the stack is available.  */
      __deallocate_stack (pd);
    }
}


/* Local function to start thread and handle cleanup.
   createthread.c defines the macro START_THREAD_DEFN to the
   declaration that its create_thread function will refer to, and
   START_THREAD_SELF to the expression to optimally deliver the new
   thread's THREAD_SELF value.  */
START_THREAD_DEFN
{
  struct pthread *pd = START_THREAD_SELF;

#if HP_TIMING_AVAIL
  /* Remember the time when the thread was started.  */
  hp_timing_t now;
  HP_TIMING_NOW (now);
  THREAD_SETMEM (pd, cpuclock_offset, now);
#endif

  /* Initialize resolver state pointer.  */
  __resp = &pd->res;

  /* Initialize pointers to locale data.  */
  __ctype_init ();

  /* Allow setxid from now onwards.  */
  if (__glibc_unlikely (atomic_exchange_acq (&pd->setxid_futex, 0) == -2))
    futex_wake (&pd->setxid_futex, 1, FUTEX_PRIVATE);

#ifdef __NR_set_robust_list
# ifndef __ASSUME_SET_ROBUST_LIST
  if (__set_robust_list_avail >= 0)
# endif
    {
      INTERNAL_SYSCALL_DECL (err);
      /* This call should never fail because the initial call in init.c
	 succeeded.  */
      INTERNAL_SYSCALL (set_robust_list, err, 2, &pd->robust_head,
			sizeof (struct robust_list_head));
    }
#endif

#ifdef SIGCANCEL
  /* If the parent was running cancellation handlers while creating
     the thread the new thread inherited the signal mask.  Reset the
     cancellation signal mask.  */
  if (__glibc_unlikely (pd->parent_cancelhandling & CANCELING_BITMASK))
    {
      INTERNAL_SYSCALL_DECL (err);
      sigset_t mask;
      __sigemptyset (&mask);
      __sigaddset (&mask, SIGCANCEL);
      (void) INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_UNBLOCK, &mask,
			       NULL, _NSIG / 8);
    }
#endif

  /* This is where the try/finally block should be created.  For
     compilers without that support we do use setjmp.  */
  struct pthread_unwind_buf unwind_buf;

  int not_first_call;
  not_first_call = setjmp ((struct __jmp_buf_tag *) unwind_buf.cancel_jmp_buf);

  /* No previous handlers.  NB: This must be done after setjmp since the
     private space in the unwind jump buffer may overlap space used by
     setjmp to store extra architecture-specific information which is
     never used by the cancellation-specific __libc_unwind_longjmp.

     The private space is allowed to overlap because the unwinder never
     has to return through any of the jumped-to call frames, and thus
     only a minimum amount of saved data need be stored, and for example,
     need not include the process signal mask information. This is all
     an optimization to reduce stack usage when pushing cancellation
     handlers.  */
  unwind_buf.priv.data.prev = NULL;
  unwind_buf.priv.data.cleanup = NULL;

  if (__glibc_likely (! not_first_call))
    {
      /* Store the new cleanup handler info.  */
      THREAD_SETMEM (pd, cleanup_jmp_buf, &unwind_buf);

      /* We are either in (a) or (b), and in either case we either own
         PD already (2) or are about to own PD (1), and so our only
	 restriction would be that we can't free PD until we know we
	 have ownership (see CONCURRENCY NOTES above).  */
      if (__glibc_unlikely (pd->stopped_start))
	{
	  int oldtype = CANCEL_ASYNC ();

	  /* Get the lock the parent locked to force synchronization.  */
	  lll_lock (pd->lock, LLL_PRIVATE);

	  /* We have ownership of PD now.  */

	  /* And give it up right away.  */
	  lll_unlock (pd->lock, LLL_PRIVATE);

	  CANCEL_RESET (oldtype);
	}

      LIBC_PROBE (pthread_start, 3, (pthread_t) pd, pd->start_routine, pd->arg);

      /* Run the code the user provided.  */
      void *ret;
      if (pd->c11)
	{
	  /* The function pointer of the c11 thread start is cast to an incorrect
	     type on __pthread_create_2_1 call, however it is casted back to correct
	     one so the call behavior is well-defined (it is assumed that pointers
	     to void are able to represent all values of int.  */
	  int (*start)(void*) = (int (*) (void*)) pd->start_routine;
	  ret = (void*) (uintptr_t) start (pd->arg);
	}
      else
	ret = pd->start_routine (pd->arg);
      THREAD_SETMEM (pd, result, ret);
    }

  /* Call destructors for the thread_local TLS variables.  */
#ifndef SHARED
  if (&__call_tls_dtors != NULL)
#endif
    __call_tls_dtors ();

  /* Run the destructor for the thread-local data.  */
  __nptl_deallocate_tsd ();

  /* Clean up any state libc stored in thread-local variables.  */
  __libc_thread_freeres ();

  /* If this is the last thread we terminate the process now.  We
     do not notify the debugger, it might just irritate it if there
     is no thread left.  */
  if (__glibc_unlikely (atomic_decrement_and_test (&__nptl_nthreads)))
    /* This was the last thread.  */
    exit (0);

  /* Report the death of the thread if this is wanted.  */
  if (__glibc_unlikely (pd->report_events))
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
	      while (atomic_compare_and_exchange_bool_acq (&__nptl_last_event,
							   pd, pd->nextevent));
	    }

	  /* Now call the function which signals the event.  See
	     CONCURRENCY NOTES for the nptl_db interface comments.  */
	  __nptl_death_event ();
	}
    }

  /* The thread is exiting now.  Don't set this bit until after we've hit
     the event-reporting breakpoint, so that td_thr_get_info on us while at
     the breakpoint reports TD_THR_RUN state rather than TD_THR_ZOMBIE.  */
  atomic_bit_set (&pd->cancelhandling, EXITING_BIT);

#ifndef __ASSUME_SET_ROBUST_LIST
  /* If this thread has any robust mutexes locked, handle them now.  */
# if __PTHREAD_MUTEX_HAVE_PREV
  void *robust = pd->robust_head.list;
# else
  __pthread_slist_t *robust = pd->robust_list.__next;
# endif
  /* We let the kernel do the notification if it is able to do so.
     If we have to do it here there for sure are no PI mutexes involved
     since the kernel support for them is even more recent.  */
  if (__set_robust_list_avail < 0
      && __builtin_expect (robust != (void *) &pd->robust_head, 0))
    {
      do
	{
	  struct __pthread_mutex_s *this = (struct __pthread_mutex_s *)
	    ((char *) robust - offsetof (struct __pthread_mutex_s,
					 __list.__next));
	  robust = *((void **) robust);

# if __PTHREAD_MUTEX_HAVE_PREV
	  this->__list.__prev = NULL;
# endif
	  this->__list.__next = NULL;

	  atomic_or (&this->__lock, FUTEX_OWNER_DIED);
	  futex_wake ((unsigned int *) &this->__lock, 1,
		      /* XYZ */ FUTEX_SHARED);
	}
      while (robust != (void *) &pd->robust_head);
    }
#endif

  advise_stack_range (pd->stackblock, pd->stackblock_size, (uintptr_t) pd,
		      pd->guardsize);

  /* If the thread is detached free the TCB.  */
  if (IS_DETACHED (pd))
    /* Free the TCB.  */
    __free_tcb (pd);
  else if (__glibc_unlikely (pd->cancelhandling & SETXID_BITMASK))
    {
      /* Some other thread might call any of the setXid functions and expect
	 us to reply.  In this case wait until we did that.  */
      do
	/* XXX This differs from the typical futex_wait_simple pattern in that
	   the futex_wait condition (setxid_futex) is different from the
	   condition used in the surrounding loop (cancelhandling).  We need
	   to check and document why this is correct.  */
	futex_wait_simple (&pd->setxid_futex, 0, FUTEX_PRIVATE);
      while (pd->cancelhandling & SETXID_BITMASK);

      /* Reset the value so that the stack can be reused.  */
      pd->setxid_futex = 0;
    }

  /* We cannot call '_exit' here.  '_exit' will terminate the process.

     The 'exit' implementation in the kernel will signal when the
     process is really dead since 'clone' got passed the CLONE_CHILD_CLEARTID
     flag.  The 'tid' field in the TCB will be set to zero.

     The exit code is zero since in case all threads exit by calling
     'pthread_exit' the exit status must be 0 (zero).  */
  __exit_thread ();

  /* NOTREACHED */
}


/* Return true iff obliged to report TD_CREATE events.  */
static bool
report_thread_creation (struct pthread *pd)
{
  if (__glibc_unlikely (THREAD_GETMEM (THREAD_SELF, report_events)))
    {
      /* The parent thread is supposed to report events.
	 Check whether the TD_CREATE event is needed, too.  */
      const size_t idx = __td_eventword (TD_CREATE);
      const uint32_t mask = __td_eventmask (TD_CREATE);

      return ((mask & (__nptl_threads_events.event_bits[idx]
		       | pd->eventbuf.eventmask.event_bits[idx])) != 0);
    }
  return false;
}


int
__pthread_create_2_1 (pthread_t *newthread, const pthread_attr_t *attr,
		      void *(*start_routine) (void *), void *arg)
{
  STACK_VARIABLES;

  const struct pthread_attr *iattr = (struct pthread_attr *) attr;
  struct pthread_attr default_attr;
  bool free_cpuset = false;
  bool c11 = (attr == ATTR_C11_THREAD);
  if (iattr == NULL || c11)
    {
      lll_lock (__default_pthread_attr_lock, LLL_PRIVATE);
      default_attr = __default_pthread_attr;
      size_t cpusetsize = default_attr.cpusetsize;
      if (cpusetsize > 0)
	{
	  cpu_set_t *cpuset;
	  if (__glibc_likely (__libc_use_alloca (cpusetsize)))
	    cpuset = __alloca (cpusetsize);
	  else
	    {
	      cpuset = malloc (cpusetsize);
	      if (cpuset == NULL)
		{
		  lll_unlock (__default_pthread_attr_lock, LLL_PRIVATE);
		  return ENOMEM;
		}
	      free_cpuset = true;
	    }
	  memcpy (cpuset, default_attr.cpuset, cpusetsize);
	  default_attr.cpuset = cpuset;
	}
      lll_unlock (__default_pthread_attr_lock, LLL_PRIVATE);
      iattr = &default_attr;
    }

  struct pthread *pd = NULL;
  int err = ALLOCATE_STACK (iattr, &pd);
  int retval = 0;

  if (__glibc_unlikely (err != 0))
    /* Something went wrong.  Maybe a parameter of the attributes is
       invalid or we could not allocate memory.  Note we have to
       translate error codes.  */
    {
      retval = err == ENOMEM ? EAGAIN : err;
      goto out;
    }


  /* Initialize the TCB.  All initializations with zero should be
     performed in 'get_cached_stack'.  This way we avoid doing this if
     the stack freshly allocated with 'mmap'.  */

#if TLS_TCB_AT_TP
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
  pd->c11 = c11;

  /* Copy the thread attribute flags.  */
  struct pthread *self = THREAD_SELF;
  pd->flags = ((iattr->flags & ~(ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET))
	       | (self->flags & (ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET)));

  /* Initialize the field for the ID of the thread which is waiting
     for us.  This is a self-reference in case the thread is created
     detached.  */
  pd->joinid = iattr->flags & ATTR_FLAG_DETACHSTATE ? pd : NULL;

  /* The debug events are inherited from the parent.  */
  pd->eventbuf = self->eventbuf;


  /* Copy the parent's scheduling parameters.  The flags will say what
     is valid and what is not.  */
  pd->schedpolicy = self->schedpolicy;
  pd->schedparam = self->schedparam;

  /* Copy the stack guard canary.  */
#ifdef THREAD_COPY_STACK_GUARD
  THREAD_COPY_STACK_GUARD (pd);
#endif

  /* Copy the pointer guard value.  */
#ifdef THREAD_COPY_POINTER_GUARD
  THREAD_COPY_POINTER_GUARD (pd);
#endif

  /* Setup tcbhead.  */
  tls_setup_tcbhead (pd);

  /* Verify the sysinfo bits were copied in allocate_stack if needed.  */
#ifdef NEED_DL_SYSINFO
  CHECK_THREAD_SYSINFO (pd);
#endif

  /* Inform start_thread (above) about cancellation state that might
     translate into inherited signal state.  */
  pd->parent_cancelhandling = THREAD_GETMEM (THREAD_SELF, cancelhandling);

  /* Determine scheduling parameters for the thread.  */
  if (__builtin_expect ((iattr->flags & ATTR_FLAG_NOTINHERITSCHED) != 0, 0)
      && (iattr->flags & (ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET)) != 0)
    {
      /* Use the scheduling parameters the user provided.  */
      if (iattr->flags & ATTR_FLAG_POLICY_SET)
        {
          pd->schedpolicy = iattr->schedpolicy;
          pd->flags |= ATTR_FLAG_POLICY_SET;
        }
      if (iattr->flags & ATTR_FLAG_SCHED_SET)
        {
          /* The values were validated in pthread_attr_setschedparam.  */
          pd->schedparam = iattr->schedparam;
          pd->flags |= ATTR_FLAG_SCHED_SET;
        }

      if ((pd->flags & (ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET))
          != (ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET))
        collect_default_sched (pd);
    }

  if (__glibc_unlikely (__nptl_nthreads == 1))
    _IO_enable_locks ();

  /* Pass the descriptor to the caller.  */
  *newthread = (pthread_t) pd;

  LIBC_PROBE (pthread_create, 4, newthread, attr, start_routine, arg);

  /* One more thread.  We cannot have the thread do this itself, since it
     might exist but not have been scheduled yet by the time we've returned
     and need to check the value to behave correctly.  We must do it before
     creating the thread, in case it does get scheduled first and then
     might mistakenly think it was the only thread.  In the failure case,
     we momentarily store a false value; this doesn't matter because there
     is no kosher thing a signal handler interrupting us right here can do
     that cares whether the thread count is correct.  */
  atomic_increment (&__nptl_nthreads);

  /* Our local value of stopped_start and thread_ran can be accessed at
     any time. The PD->stopped_start may only be accessed if we have
     ownership of PD (see CONCURRENCY NOTES above).  */
  bool stopped_start = false; bool thread_ran = false;

  /* Start the thread.  */
  if (__glibc_unlikely (report_thread_creation (pd)))
    {
      stopped_start = true;

      /* We always create the thread stopped at startup so we can
	 notify the debugger.  */
      retval = create_thread (pd, iattr, &stopped_start,
			      STACK_VARIABLES_ARGS, &thread_ran);
      if (retval == 0)
	{
	  /* We retain ownership of PD until (a) (see CONCURRENCY NOTES
	     above).  */

	  /* Assert stopped_start is true in both our local copy and the
	     PD copy.  */
	  assert (stopped_start);
	  assert (pd->stopped_start);

	  /* Now fill in the information about the new thread in
	     the newly created thread's data structure.  We cannot let
	     the new thread do this since we don't know whether it was
	     already scheduled when we send the event.  */
	  pd->eventbuf.eventnum = TD_CREATE;
	  pd->eventbuf.eventdata = pd;

	  /* Enqueue the descriptor.  */
	  do
	    pd->nextevent = __nptl_last_event;
	  while (atomic_compare_and_exchange_bool_acq (&__nptl_last_event,
						       pd, pd->nextevent)
		 != 0);

	  /* Now call the function which signals the event.  See
	     CONCURRENCY NOTES for the nptl_db interface comments.  */
	  __nptl_create_event ();
	}
    }
  else
    retval = create_thread (pd, iattr, &stopped_start,
			    STACK_VARIABLES_ARGS, &thread_ran);

  if (__glibc_unlikely (retval != 0))
    {
      if (thread_ran)
	/* State (c) or (d) and we may not have PD ownership (see
	   CONCURRENCY NOTES above).  We can assert that STOPPED_START
	   must have been true because thread creation didn't fail, but
	   thread attribute setting did.  */
	/* See bug 19511 which explains why doing nothing here is a
	   resource leak for a joinable thread.  */
	assert (stopped_start);
      else
	{
	  /* State (e) and we have ownership of PD (see CONCURRENCY
	     NOTES above).  */

	  /* Oops, we lied for a second.  */
	  atomic_decrement (&__nptl_nthreads);

	  /* Perhaps a thread wants to change the IDs and is waiting for this
	     stillborn thread.  */
	  if (__glibc_unlikely (atomic_exchange_acq (&pd->setxid_futex, 0)
				== -2))
	    futex_wake (&pd->setxid_futex, 1, FUTEX_PRIVATE);

	  /* Free the resources.  */
	  __deallocate_stack (pd);
	}

      /* We have to translate error codes.  */
      if (retval == ENOMEM)
	retval = EAGAIN;
    }
  else
    {
      /* We don't know if we have PD ownership.  Once we check the local
         stopped_start we'll know if we're in state (a) or (b) (see
	 CONCURRENCY NOTES above).  */
      if (stopped_start)
	/* State (a), we own PD. The thread blocked on this lock either
	   because we're doing TD_CREATE event reporting, or for some
	   other reason that create_thread chose.  Now let it run
	   free.  */
	lll_unlock (pd->lock, LLL_PRIVATE);

      /* We now have for sure more than one thread.  The main thread might
	 not yet have the flag set.  No need to set the global variable
	 again if this is what we use.  */
      THREAD_SETMEM (THREAD_SELF, header.multiple_threads, 1);
    }

 out:
  if (__glibc_unlikely (free_cpuset))
    free (default_attr.cpuset);

  return retval;
}
versioned_symbol (libpthread, __pthread_create_2_1, pthread_create, GLIBC_2_1);


#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
int
__pthread_create_2_0 (pthread_t *newthread, const pthread_attr_t *attr,
		      void *(*start_routine) (void *), void *arg)
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
      new_attr.cpuset = NULL;

      /* We will pass this value on to the real implementation.  */
      attr = (pthread_attr_t *) &new_attr;
    }

  return __pthread_create_2_1 (newthread, attr, start_routine, arg);
}
compat_symbol (libpthread, __pthread_create_2_0, pthread_create,
	       GLIBC_2_0);
#endif

/* Information for libthread_db.  */

#include "../nptl_db/db_info.c"

/* If pthread_create is present, libgcc_eh.a and libsupc++.a expects some other POSIX thread
   functions to be present as well.  */
PTHREAD_STATIC_FN_REQUIRE (__pthread_mutex_lock)
PTHREAD_STATIC_FN_REQUIRE (__pthread_mutex_trylock)
PTHREAD_STATIC_FN_REQUIRE (__pthread_mutex_unlock)

PTHREAD_STATIC_FN_REQUIRE (__pthread_once)
PTHREAD_STATIC_FN_REQUIRE (__pthread_cancel)

PTHREAD_STATIC_FN_REQUIRE (__pthread_key_create)
PTHREAD_STATIC_FN_REQUIRE (__pthread_key_delete)
PTHREAD_STATIC_FN_REQUIRE (__pthread_setspecific)
PTHREAD_STATIC_FN_REQUIRE (__pthread_getspecific)
