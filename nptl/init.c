/* Copyright (C) 2002 Free Software Foundation, Inc.
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

#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <pthreadP.h>
#include <atomic.h>
#include <ldsodefs.h>
#include <tls.h>
#include <fork.h>
#include <version.h>


/* XXX For the time being...  */
#define __NR_set_tid_address	258


/* Default stack size.  */
size_t __default_stacksize attribute_hidden;

/* Size and alignment of static TLS block.  */
size_t __static_tls_size;
size_t __static_tls_align;

/* Version of the library, used in libthread_db to detect mismatches.  */
static const char nptl_version[] = VERSION;


#if defined USE_TLS && !defined SHARED
extern void __libc_setup_tls (size_t tcbsize, size_t tcbalign);
#endif


/* For asynchronous cancellation we use a signal.  This is the handler.  */
static void
sigcancel_handler (int sig __attribute ((unused)))
{
  struct pthread *self = THREAD_SELF;

  while (1)
    {
      /* We are canceled now.  When canceled by another thread this flag
	 is already set but if the signal is directly send (internally or
	 from another process) is has to be done here.  */
      int oldval = THREAD_GETMEM (self, cancelhandling);
      int newval = oldval | CANCELED_BITMASK;

      if (oldval == newval || (oldval & EXITING_BITMASK) != 0)
	/* Already canceled or exiting.  */
	break;

      if (atomic_compare_and_exchange_acq (&self->cancelhandling, newval,
					   oldval) == 0)
	{
	  /* Set the return value.  */
	  THREAD_SETMEM (self, result, PTHREAD_CANCELED);

	  /* Make sure asynchronous cancellation is still enabled.  */
	  if ((newval & CANCELTYPE_BITMASK) != 0)
	    {
	      /* The thread is exiting now.  */
	      atomic_bit_set (&self->cancelhandling, EXITING_BIT);

	      /* Run the registered destructors and terminate the
		 thread.  */
	      __do_cancel (CURRENT_STACK_FRAME);
	    }

	  break;
	}
    }
}



void
#ifdef SHARED
__attribute ((constructor))
#endif
__pthread_initialize_minimal (void)
{
  struct sigaction sa;
  struct rlimit limit;
#ifdef USE_TLS
  struct pthread *pd;
#endif

#ifndef SHARED
  /* Unlike in the dynamically linked case the dynamic linker has not
     taken care of initializing the TLS data structures.  */
  __libc_setup_tls (TLS_TCB_SIZE, TLS_TCB_ALIGN);
#endif

  /* Minimal initialization of the thread descriptor.  */
  pd = THREAD_SELF;
  pd->tid = INTERNAL_SYSCALL (set_tid_address, 1, &pd->tid);
  THREAD_SETMEM (pd, specific[0], &pd->specific_1stblock[0]);
  THREAD_SETMEM (pd, user_stack, true);
  if (LLL_LOCK_INITIALIZER != 0)
    THREAD_SETMEM (pd, lock, LLL_LOCK_INITIALIZER);
#if HP_TIMING_AVAIL
  THREAD_SETMEM (pd, cpuclock_offset, GL(dl_cpuclock_offset));
#endif

  /* Initialize the list of all running threads with the main thread.  */
  INIT_LIST_HEAD (&__stack_user);
  list_add (&pd->header.data.list, &__stack_user);


  /* Install the cancellation signal handler.  If for some reason we
     cannot install the handler we do not abort.  Maybe we should, but
     it is only asynchronous cancellation which is affected.  */
  sa.sa_handler = sigcancel_handler;
  sa.sa_flags = 0;

  /* Avoid another cancellation signal when we process one.  */
  sigemptyset (&sa.sa_mask);
  sigaddset (&sa.sa_mask, SIGCANCEL);

  (void) __libc_sigaction (SIGCANCEL, &sa, NULL);


  /* Determine the default allowed stack size.  This is the size used
     in case the user does not specify one.  */
  if (getrlimit (RLIMIT_STACK, &limit) != 0
      || limit.rlim_cur == RLIM_INFINITY)
    /* The system limit is not usable.  Use an architecture-specific
       default.  */
    limit.rlim_cur = ARCH_STACK_DEFAULT_SIZE;

#ifdef NEED_SEPARATE_REGISTER_STACK
  __default_stacksize = MAX (limit.rlim_cur / 2, PTHREAD_STACK_MIN);
#else
  __default_stacksize = MAX (limit.rlim_cur, PTHREAD_STACK_MIN);
#endif
  /* The maximum page size better should be a multiple of the page
     size.  */
  assert (__default_stacksize % __sysconf (_SC_PAGESIZE) == 0);

  /* Get the size of the static and alignment requirements for the TLS
     block.  */
  _dl_get_tls_static_info (&__static_tls_size, &__static_tls_align);

  /* Make sure the size takes all the alignments into account.  */
  if (STACK_ALIGN > __static_tls_align)
    __static_tls_align = STACK_ALIGN;
  __static_tls_size = roundup (__static_tls_size, __static_tls_align);

  /* Register the fork generation counter with the libc.  */
  __register_pthread_fork_handler (&__fork_generation, __reclaim_stacks);
}
