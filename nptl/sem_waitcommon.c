/* sem_waitcommon -- wait on a semaphore, shared code.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>
#include <sys/time.h>

#include <pthreadP.h>
#include <shlib-compat.h>
#include <atomic.h>

/* Wrapper for lll_futex_wait with absolute timeout and error checking.
   TODO Remove when cleaning up the futex API throughout glibc.  */
static __always_inline int
futex_abstimed_wait (unsigned int* futex, unsigned int expected,
		     const struct timespec* abstime, int private, bool cancel)
{
  int err, oldtype;
  if (abstime == NULL)
    {
      if (cancel)
	oldtype = __pthread_enable_asynccancel ();
      err = lll_futex_wait (futex, expected, private);
      if (cancel)
	__pthread_disable_asynccancel (oldtype);
    }
  else
    {
      struct timeval tv;
      struct timespec rt;
      int sec, nsec;

      /* Get the current time.  */
      __gettimeofday (&tv, NULL);

      /* Compute relative timeout.  */
      sec = abstime->tv_sec - tv.tv_sec;
      nsec = abstime->tv_nsec - tv.tv_usec * 1000;
      if (nsec < 0)
        {
          nsec += 1000000000;
          --sec;
        }

      /* Already timed out?  */
      if (sec < 0)
        return ETIMEDOUT;

      /* Do wait.  */
      rt.tv_sec = sec;
      rt.tv_nsec = nsec;
      if (cancel)
	oldtype = __pthread_enable_asynccancel ();
      err = lll_futex_timed_wait (futex, expected, &rt, private);
      if (cancel)
	__pthread_disable_asynccancel (oldtype);
    }
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      abort ();
    }
}

/* Wrapper for lll_futex_wake, with error checking.
   TODO Remove when cleaning up the futex API throughout glibc.  */
static __always_inline void
futex_wake (unsigned int* futex, int processes_to_wake, int private)
{
  int res = lll_futex_wake (futex, processes_to_wake, private);
  /* No error.  Ignore the number of woken processes.  */
  if (res >= 0)
    return;
  switch (res)
    {
    case -EFAULT: /* Could have happened due to memory reuse.  */
    case -EINVAL: /* Could be either due to incorrect alignment (a bug in
		     glibc or in the application) or due to memory being
		     reused for a PI futex.  We cannot distinguish between the
		     two causes, and one of them is correct use, so we do not
		     act in this case.  */
      return;
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      abort ();
    }
}


/* The semaphore provides two main operations: sem_post adds a token to the
   semaphore; sem_wait grabs a token from the semaphore, potentially waiting
   until there is a token available.  A sem_wait needs to synchronize with
   the sem_post that provided the token, so that whatever lead to the sem_post
   happens before the code after sem_wait.

   Conceptually, available tokens can simply be counted; let's call that the
   value of the semaphore.  However, we also want to know whether there might
   be a sem_wait that is blocked on the value because it was zero (using a
   futex with the value being the futex variable); if there is no blocked
   sem_wait, sem_post does not need to execute a futex_wake call.  Therefore,
   we also need to count the number of potentially blocked sem_wait calls
   (which we call nwaiters).

   What makes this tricky is that POSIX requires that a semaphore can be
   destroyed as soon as the last remaining sem_wait has returned, and no
   other sem_wait or sem_post calls are executing concurrently.  However, the
   sem_post call whose token was consumed by the last sem_wait is considered
   to have finished once it provided the token to the sem_wait.
   Thus, sem_post must not access the semaphore struct anymore after it has
   made a token available; IOW, it needs to be able to atomically provide
   a token and check whether any blocked sem_wait calls might exist.

   This is straightforward to do if the architecture provides 64b atomics
   because we can just put both the value and nwaiters into one variable that
   we access atomically: This is the data field, the value is in the
   least-significant 32 bits, and nwaiters in the other bits.  When sem_post
   makes a value available, it can atomically check nwaiters.

   If we have only 32b atomics available, we cannot put both nwaiters and
   value into one 32b value because then we might have too few bits for both
   of those counters.  Therefore, we need to use two distinct fields.

   To allow sem_post to atomically make a token available and check for
   blocked sem_wait calls, we use one bit in value to indicate whether
   nwaiters is nonzero.  That allows sem_post to use basically the same
   algorithm as with 64b atomics, but requires sem_wait to update the bit; it
   can't do this atomically with another access to nwaiters, but it can compute
   a conservative value for the bit because it's benign if the bit is set
   even if nwaiters is zero (all we get is an unnecessary futex wake call by
   sem_post).
   Specifically, sem_wait will unset the bit speculatively if it believes that
   there is no other concurrently executing sem_wait.  If it misspeculated,
   it will have to clean up by waking any other sem_wait call (i.e., what
   sem_post would do otherwise).  This does not conflict with the destruction
   requirement because the semaphore must not be destructed while any sem_wait
   is still executing.  */

/* Set this to true if you assume that, in contrast to current Linux futex
   documentation, lll_futex_wake can return -EINTR only if interrupted by a
   signal, not spuriously due to some other reason.
   TODO Discuss EINTR conditions with the Linux kernel community.  For
   now, we set this to true to not change behavior of semaphores compared
   to previous glibc builds.  */
static const int sem_assume_only_signals_cause_futex_EINTR = 1;

#if !__HAVE_64B_ATOMICS
static void
__sem_wait_32_finish (struct new_sem *sem);
#endif

static void
__sem_wait_cleanup (void *arg)
{
  struct new_sem *sem = (struct new_sem *) arg;

#if __HAVE_64B_ATOMICS
  /* Stop being registered as a waiter.  See below for MO.  */
  atomic_fetch_add_relaxed (&sem->data, -((uint64_t) 1 << SEM_NWAITERS_SHIFT));
#else
  __sem_wait_32_finish (sem);
#endif
}

/* Wait until at least one token is available, possibly with a timeout.
   This is in a separate function in order to make sure gcc
   puts the call site into an exception region, and thus the
   cleanups get properly run.  TODO still necessary?  Other futex_wait
   users don't seem to need it.  */
static int
__attribute__ ((noinline))
do_futex_wait (struct new_sem *sem, const struct timespec *abstime)
{
  int err;

#if __HAVE_64B_ATOMICS
  err = futex_abstimed_wait ((unsigned int *) &sem->data + SEM_VALUE_OFFSET, 0,
			     abstime, sem->private, true);
#else
  err = futex_abstimed_wait (&sem->value, SEM_NWAITERS_MASK, abstime,
			     sem->private, true);
#endif

  return err;
}

/* Fast path: Try to grab a token without blocking.  */
static int
__new_sem_wait_fast (struct new_sem *sem, int definitive_result)
{
  /* We need acquire MO if we actually grab a token, so that this
     synchronizes with all token providers (i.e., the RMW operation we read
     from or all those before it in modification order; also see sem_post).
     We do not need to guarantee any ordering if we observed that there is
     no token (POSIX leaves it unspecified whether functions that fail
     synchronize memory); thus, relaxed MO is sufficient for the initial load
     and the failure path of the CAS.  If the weak CAS fails and we need a
     definitive result, retry.  */
#if __HAVE_64B_ATOMICS
  uint64_t d = atomic_load_relaxed (&sem->data);
  do
    {
      if ((d & SEM_VALUE_MASK) == 0)
	break;
      if (atomic_compare_exchange_weak_acquire (&sem->data, &d, d - 1))
	return 0;
    }
  while (definitive_result);
  return -1;
#else
  unsigned int v = atomic_load_relaxed (&sem->value);
  do
    {
      if ((v >> SEM_VALUE_SHIFT) == 0)
	break;
      if (atomic_compare_exchange_weak_acquire (&sem->value,
	  &v, v - (1 << SEM_VALUE_SHIFT)))
	return 0;
    }
  while (definitive_result);
  return -1;
#endif
}

/* Slow path that blocks.  */
static int
__attribute__ ((noinline))
__new_sem_wait_slow (struct new_sem *sem, const struct timespec *abstime)
{
  int err = 0;

#if __HAVE_64B_ATOMICS
  /* Add a waiter.  Relaxed MO is sufficient because we can rely on the
     ordering provided by the RMW operations we use.  */
  uint64_t d = atomic_fetch_add_relaxed (&sem->data,
      (uint64_t) 1 << SEM_NWAITERS_SHIFT);

  pthread_cleanup_push (__sem_wait_cleanup, sem);

  /* Wait for a token to be available.  Retry until we can grab one.  */
  for (;;)
    {
      /* If there is no token available, sleep until there is.  */
      if ((d & SEM_VALUE_MASK) == 0)
	{
	  err = do_futex_wait (sem, abstime);
	  /* A futex return value of 0 or EAGAIN is due to a real or spurious
	     wake-up, or due to a change in the number of tokens.  We retry in
	     these cases.
	     If we timed out, forward this to the caller.
	     EINTR could be either due to being interrupted by a signal, or
	     due to a spurious wake-up.  Thus, we cannot distinguish between
	     both, and are not allowed to return EINTR to the caller but have
	     to retry; this is because we may not have been interrupted by a
	     signal.  However, if we assume that only signals cause a futex
	     return of EINTR, we forward EINTR to the caller.

	     Retrying on EINTR is technically always allowed because to
	     reliably interrupt sem_wait with a signal, the signal handler
	     must call sem_post (which is AS-Safe).  In executions where the
	     signal handler does not do that, the implementation can correctly
	     claim that sem_wait hadn't actually started to execute yet, and
	     thus the signal never actually interrupted sem_wait.  We make no
	     timing guarantees, so the program can never observe that sem_wait
	     actually did start to execute.  Thus, in a correct program, we
	     can expect a signal that wanted to interrupt the sem_wait to have
	     provided a token, and can just try to grab this token if
	     futex_wait returns EINTR.  */
	  if (err == ETIMEDOUT ||
	      (err == EINTR && sem_assume_only_signals_cause_futex_EINTR))
	    {
	      __set_errno (err);
	      err = -1;
	      /* Stop being registered as a waiter.  */
	      atomic_fetch_add_relaxed (&sem->data,
		  -((uint64_t) 1 << SEM_NWAITERS_SHIFT));
	      break;
	    }
	  /* Relaxed MO is sufficient; see below.  */
	  d = atomic_load_relaxed (&sem->data);
	}
      else
	{
	  /* Try to grab both a token and stop being a waiter.  We need
	     acquire MO so this synchronizes with all token providers (i.e.,
	     the RMW operation we read from or all those before it in
	     modification order; also see sem_post).  On the failure path,
	     relaxed MO is sufficient because we only eventually need the
	     up-to-date value; the futex_wait or the CAS perform the real
	     work.  */
	  if (atomic_compare_exchange_weak_acquire (&sem->data,
	      &d, d - 1 - ((uint64_t) 1 << SEM_NWAITERS_SHIFT)))
	    {
	      err = 0;
	      break;
	    }
	}
    }

  pthread_cleanup_pop (0);
#else
  /* The main difference to the 64b-atomics implementation is that we need to
     access value and nwaiters in separate steps, and that the nwaiters bit
     in the value can temporarily not be set even if nwaiters is nonzero.
     We work around incorrectly unsetting the nwaiters bit by letting sem_wait
     set the bit again and waking the number of waiters that could grab a
     token.  There are two additional properties we need to ensure:
     (1) We make sure that whenever unsetting the bit, we see the increment of
     nwaiters by the other thread that set the bit.  IOW, we will notice if
     we make a mistake.
     (2) When setting the nwaiters bit, we make sure that we see the unsetting
     of the bit by another waiter that happened before us.  This avoids having
     to blindly set the bit whenever we need to block on it.  We set/unset
     the bit while having incremented nwaiters (i.e., are a registered
     waiter), and the problematic case only happens when one waiter indeed
     followed another (i.e., nwaiters was never larger than 1); thus, this
     works similarly as with a critical section using nwaiters (see the MOs
     and related comments below).

     An alternative approach would be to unset the bit after decrementing
     nwaiters; however, that would result in needing Dekker-like
     synchronization and thus full memory barriers.  We also would not be able
     to prevent misspeculation, so this alternative scheme does not seem
     beneficial.  */
  unsigned int v;

  /* Add a waiter.  We need acquire MO so this synchronizes with the release
     MO we use when decrementing nwaiters below; it ensures that if another
     waiter unset the bit before us, we see that and set it again.  Also see
     property (2) above.  */
  atomic_fetch_add_acquire (&sem->nwaiters, 1);

  pthread_cleanup_push (__sem_wait_cleanup, sem);

  /* Wait for a token to be available.  Retry until we can grab one.  */
  /* We do not need any ordering wrt. to this load's reads-from, so relaxed
     MO is sufficient.  The acquire MO above ensures that in the problematic
     case, we do see the unsetting of the bit by another waiter.  */
  v = atomic_load_relaxed (&sem->value);
  do
    {
      do
	{
	  /* We are about to block, so make sure that the nwaiters bit is
	     set.  We need release MO on the CAS to ensure that when another
	     waiter unsets the nwaiters bit, it will also observe that we
	     incremented nwaiters in the meantime (also see the unsetting of
	     the bit below).  Relaxed MO on CAS failure is sufficient (see
	     above).  */
	  do
	    {
	      if ((v & SEM_NWAITERS_MASK) != 0)
		break;
	    }
	  while (!atomic_compare_exchange_weak_release (&sem->value,
	      &v, v | SEM_NWAITERS_MASK));
	  /* If there is no token, wait.  */
	  if ((v >> SEM_VALUE_SHIFT) == 0)
	    {
	      /* See __HAVE_64B_ATOMICS variant.  */
	      err = do_futex_wait(sem, abstime);
	      if (err == ETIMEDOUT ||
		  (err == EINTR && sem_assume_only_signals_cause_futex_EINTR))
		{
		  __set_errno (err);
		  err = -1;
		  goto error;
		}
	      err = 0;
	      /* We blocked, so there might be a token now.  Relaxed MO is
		 sufficient (see above).  */
	      v = atomic_load_relaxed (&sem->value);
	    }
	}
      /* If there is no token, we must not try to grab one.  */
      while ((v >> SEM_VALUE_SHIFT) == 0);
    }
  /* Try to grab a token.  We need acquire MO so this synchronizes with
     all token providers (i.e., the RMW operation we read from or all those
     before it in modification order; also see sem_post).  */
  while (!atomic_compare_exchange_weak_acquire (&sem->value,
      &v, v - (1 << SEM_VALUE_SHIFT)));

error:
  pthread_cleanup_pop (0);

  __sem_wait_32_finish (sem);
#endif

  return err;
}

/* Stop being a registered waiter (non-64b-atomics code only).  */
#if !__HAVE_64B_ATOMICS
static void
__sem_wait_32_finish (struct new_sem *sem)
{
  /* The nwaiters bit is still set, try to unset it now if this seems
     necessary.  We do this before decrementing nwaiters so that the unsetting
     is visible to other waiters entering after us.  Relaxed MO is sufficient
     because we are just speculating here; a stronger MO would not prevent
     misspeculation.  */
  unsigned int wguess = atomic_load_relaxed (&sem->nwaiters);
  if (wguess == 1)
    /* We might be the last waiter, so unset.  This needs acquire MO so that
       it syncronizes with the release MO when setting the bit above; if we
       overwrite someone else that set the bit, we'll read in the following
       decrement of nwaiters at least from that release sequence, so we'll
       see if the other waiter is still active or if another writer entered
       in the meantime (i.e., using the check below).  */
    atomic_fetch_and_acquire (&sem->value, ~SEM_NWAITERS_MASK);

  /* Now stop being a waiter, and see whether our guess was correct.
     This needs release MO so that it synchronizes with the acquire MO when
     a waiter increments nwaiters; this makes sure that newer writers see that
     we reset the waiters_present bit.  */
  unsigned int wfinal = atomic_fetch_add_release (&sem->nwaiters, -1);
  if (wfinal > 1 && wguess == 1)
    {
      /* We guessed wrong, and so need to clean up after the mistake and
         unblock any waiters that could have not been woken.  There is no
         additional ordering that we need to set up, so relaxed MO is
         sufficient.  */
      unsigned int v = atomic_fetch_or_relaxed (&sem->value,
						SEM_NWAITERS_MASK);
      /* If there are available tokens, then wake as many waiters.  If there
         aren't any, then there is no need to wake anyone because there is
         none to grab for another waiter.  If tokens become available
         subsequently, then the respective sem_post calls will do the wake-up
         due to us having set the nwaiters bit again.  */
      v >>= SEM_VALUE_SHIFT;
      if (v > 0)
	futex_wake (&sem->value, v, sem->private);
    }
}
#endif
