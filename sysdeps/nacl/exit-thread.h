/* Call to terminate the current thread.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <assert.h>
#include <atomic.h>
#include <futex-internal.h>
#include <nacl-interfaces.h>
#include <nptl/pthreadP.h>

/* This causes the current thread to exit, without affecting other
   threads in the process if there are any.  If there are no other
   threads left, then this has the effect of _exit (0).  */

static inline void __attribute__ ((noreturn, always_inline, unused))
__exit_thread (void)
{
  struct pthread *pd = THREAD_SELF;

  /* The generic logic for pthread_join and stack/descriptor reuse is
     based on the Linux kernel feature that will clear and futex-wake
     a designated address as a final part of thread teardown.  Correct
     synchronization relies on the fact that these happen only after
     there is no possibility of user code touching or examining the
     late thread's stack.

     The NaCl system interface implements half of this: it clears a
     word after the thread's user stack is safely dead, but it does
     not futex-wake the location.  So, some shenanigans are required.
     We change and futex-wake the location here, so as to wake up any
     blocked pthread_join (i.e. lll_wait_tid) or pthread_timedjoin_np
     (i.e. lll_timedwait_tid).  However, that's before we have safely
     vacated the stack.  So instead of clearing the location, we set
     it to a special magic value, NACL_EXITING_TID.  This counts as a
     "live thread" value for all the generic logic, but is recognized
     specially in lll_wait_tid and lll_timedwait_tid (lowlevellock.h).
     Once it has this value, lll_wait_tid will busy-wait for the
     location to be cleared to zero by the NaCl system code.  Only then
     is the stack actually safe to reuse.  */

  if (!IS_DETACHED (pd))
    {
      /* The magic value must not be one that could ever be a valid
	 TID value.  See pthread-pids.h about the low bit.  */
      assert (NACL_EXITING_TID & 1);

      /* The magic value must not be one that has the "free" flag
	 (i.e. sign bit) set.  If that bit is set, then the
	 descriptor could be reused for a new thread.  */
      assert (NACL_EXITING_TID > 0);

      atomic_store_relaxed (&pd->tid, NACL_EXITING_TID);
      futex_wake ((unsigned int *) &pd->tid, 1, FUTEX_PRIVATE);
    }

  /* This clears PD->tid some time after the thread stack can never
     be touched again.  Unfortunately, it does not also do a
     futex-wake at that time (as Linux does via CLONE_CHILD_CLEARTID
     and set_tid_address).  So lll_wait_tid does some busy-waiting.  */
  __nacl_irt_thread.thread_exit (&pd->tid);

  /* That never returns unless something is severely and unrecoverably wrong.
     If it ever does, try to make sure we crash.  */
  while (1)
    __builtin_trap ();
}
