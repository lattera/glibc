/* futex operations for glibc-internal use.  Stub version; do not include
   this file directly.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef STUB_FUTEX_INTERNAL_H
#define STUB_FUTEX_INTERNAL_H

#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include <libc-internal.h>

/* This file defines futex operations used internally in glibc.  A futex
   consists of the so-called futex word in userspace, which is of type
   unsigned int and represents an application-specific condition, and kernel
   state associated with this particular futex word (e.g., wait queues).  The
   futex operations we provide are wrappers for the futex syscalls and add
   glibc-specific error checking of the syscall return value.  We abort on
   error codes that are caused by bugs in glibc or in the calling application,
   or when an error code is not known.  We return error codes that can arise
   in correct executions to the caller.  Each operation calls out exactly the
   return values that callers need to handle.

   The private flag must be either FUTEX_PRIVATE or FUTEX_SHARED.
   FUTEX_PRIVATE is always supported, and the implementation can internally
   use FUTEX_SHARED when FUTEX_PRIVATE is requested.  FUTEX_SHARED is not
   necessarily supported (use futex_supports_pshared to detect this).

   We expect callers to only use these operations if futexes and the
   specific futex operations being used are supported (e.g., FUTEX_SHARED).

   Given that waking other threads waiting on a futex involves concurrent
   accesses to the futex word, you must use atomic operations to access the
   futex word.

   Both absolute and relative timeouts can be used.  An absolute timeout
   expires when the given specific point in time on the CLOCK_REALTIME clock
   passes, or when it already has passed.  A relative timeout expires when
   the given duration of time on the CLOCK_MONOTONIC clock passes.  Relative
   timeouts may be imprecise (see futex_supports_exact_relative_timeouts).

   Due to POSIX requirements on when synchronization data structures such
   as mutexes or semaphores can be destroyed and due to the futex design
   having separate fast/slow paths for wake-ups, we need to consider that
   futex_wake calls might effectively target a data structure that has been
   destroyed and reused for another object, or unmapped; thus, some
   errors or spurious wake-ups can happen in correct executions that would
   not be possible in a program using just a single futex whose lifetime
   does not end before the program terminates.  For background, see:
   https://sourceware.org/ml/libc-alpha/2014-04/msg00075.html
   https://lkml.org/lkml/2014/11/27/472  */

/* Defined this way for interoperability with lowlevellock.
   FUTEX_PRIVATE must be zero because the initializers for pthread_mutex_t,
   pthread_rwlock_t, and pthread_cond_t initialize the respective field of
   those structures to zero, and we want FUTEX_PRIVATE to be the default.  */
#define FUTEX_PRIVATE LLL_PRIVATE
#define FUTEX_SHARED  LLL_SHARED
#if FUTEX_PRIVATE != 0
# error FUTEX_PRIVATE must be equal to 0
#endif

/* Returns EINVAL if PSHARED is neither PTHREAD_PROCESS_PRIVATE nor
   PTHREAD_PROCESS_SHARED; otherwise, returns 0 if PSHARED is supported, and
   ENOTSUP if not.  */
static __always_inline int
futex_supports_pshared (int pshared);

/* Returns true if relative timeouts are robust to concurrent changes to the
   system clock.  If this returns false, relative timeouts can still be used
   but might be effectively longer or shorter than requested.  */
static __always_inline bool
futex_supports_exact_relative_timeouts (void);

/* Atomically wrt other futex operations on the same futex, this blocks iff
   the value *FUTEX_WORD matches the expected value.  This is
   semantically equivalent to:
     l = <get lock associated with futex> (FUTEX_WORD);
     wait_flag = <get wait_flag associated with futex> (FUTEX_WORD);
     lock (l);
     val = atomic_load_relaxed (FUTEX_WORD);
     if (val != expected) { unlock (l); return EAGAIN; }
     atomic_store_relaxed (wait_flag, true);
     unlock (l);
     // Now block; can time out in futex_time_wait (see below)
     while (atomic_load_relaxed(wait_flag) && !<spurious wake-up>);

   Note that no guarantee of a happens-before relation between a woken
   futex_wait and a futex_wake is documented; however, this does not matter
   in practice because we have to consider spurious wake-ups (see below),
   and thus would not be able to reliably reason about which futex_wake woke
   us.

   Returns 0 if woken by a futex operation or spuriously.  (Note that due to
   the POSIX requirements mentioned above, we need to conservatively assume
   that unrelated futex_wake operations could wake this futex; it is easiest
   to just be prepared for spurious wake-ups.)
   Returns EAGAIN if the futex word did not match the expected value.
   Returns EINTR if waiting was interrupted by a signal.

   Note that some previous code in glibc assumed the underlying futex
   operation (e.g., syscall) to start with or include the equivalent of a
   seq_cst fence; this allows one to avoid an explicit seq_cst fence before
   a futex_wait call when synchronizing similar to Dekker synchronization.
   However, we make no such guarantee here.  */
static __always_inline int
futex_wait (unsigned int *futex_word, unsigned int expected, int private);

/* Like futex_wait but does not provide any indication why we stopped waiting.
   Thus, when this function returns, you have to always check FUTEX_WORD to
   determine whether you need to continue waiting, and you cannot detect
   whether the waiting was interrupted by a signal.  Example use:
     while (atomic_load_relaxed (&futex_word) == 23)
       futex_wait_simple (&futex_word, 23, FUTEX_PRIVATE);
   This is common enough to make providing this wrapper worthwhile.  */
static __always_inline void
futex_wait_simple (unsigned int *futex_word, unsigned int expected,
		   int private)
{
  ignore_value (futex_wait (futex_word, expected, private));
}


/* Like futex_wait but is a POSIX cancellation point.  */
static __always_inline int
futex_wait_cancelable (unsigned int *futex_word, unsigned int expected,
		       int private);

/* Like futex_wait, but will eventually time out (i.e., stop being
   blocked) after the duration of time provided (i.e., RELTIME) has
   passed.  The caller must provide a normalized RELTIME.  RELTIME can also
   equal NULL, in which case this function behaves equivalent to futex_wait.

   Returns the same values as futex_wait under those same conditions;
   additionally, returns ETIMEDOUT if the timeout expired.
   */
static __always_inline int
futex_reltimed_wait (unsigned int* futex_word, unsigned int expected,
		     const struct timespec* reltime, int private);

/* Like futex_reltimed_wait but is a POSIX cancellation point.  */
static __always_inline int
futex_reltimed_wait_cancelable (unsigned int* futex_word,
				unsigned int expected,
			        const struct timespec* reltime, int private);

/* Like futex_reltimed_wait, but the provided timeout (ABSTIME) is an
   absolute point in time; a call will time out after this point in time.  */
static __always_inline int
futex_abstimed_wait (unsigned int* futex_word, unsigned int expected,
		     const struct timespec* abstime, int private);

/* Like futex_reltimed_wait but is a POSIX cancellation point.  */
static __always_inline int
futex_abstimed_wait_cancelable (unsigned int* futex_word,
				unsigned int expected,
			        const struct timespec* abstime, int private);

/* Atomically wrt other futex operations on the same futex, this unblocks the
   specified number of processes, or all processes blocked on this futex if
   there are fewer than the specified number.  Semantically, this is
   equivalent to:
     l = <get lock associated with futex> (FUTEX_WORD);
     lock (l);
     for (res = 0; PROCESSES_TO_WAKE > 0; PROCESSES_TO_WAKE--, res++) {
       if (<no process blocked on futex>) break;
       wf = <get wait_flag of a process blocked on futex> (FUTEX_WORD);
       // No happens-before guarantee with woken futex_wait (see above)
       atomic_store_relaxed (wf, 0);
     }
     return res;

   Note that we need to support futex_wake calls to past futexes whose memory
   has potentially been reused due to POSIX' requirements on synchronization
   object destruction (see above); therefore, we must not report or abort
   on most errors.  */
static __always_inline void
futex_wake (unsigned int* futex_word, int processes_to_wake, int private);

/* Calls __libc_fatal with an error message.  Convenience function for
   concrete implementations of the futex interface.  */
static __always_inline __attribute__ ((__noreturn__)) void
futex_fatal_error (void)
{
  __libc_fatal ("The futex facility returned an unexpected error code.");
}

#endif  /* futex-internal.h */
