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

#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/param.h>
#include <sys/time.h>
#include "pthreadP.h"
#include <atomic.h>
#include <lowlevellock.h>
#include <not-cancel.h>

#include <stap-probe.h>

#ifndef lll_timedlock_elision
#define lll_timedlock_elision(a,dummy,b,c) lll_timedlock(a, b, c)
#endif

#ifndef lll_trylock_elision
#define lll_trylock_elision(a,t) lll_trylock(a)
#endif

#ifndef FORCE_ELISION
#define FORCE_ELISION(m, s)
#endif

int
__pthread_mutex_timedlock (pthread_mutex_t *mutex,
			   const struct timespec *abstime)
{
  int oldval;
  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);
  int result = 0;

  LIBC_PROBE (mutex_timedlock_entry, 2, mutex, abstime);

  /* We must not check ABSTIME here.  If the thread does not block
     abstime must not be checked for a valid value.  */

  switch (__builtin_expect (PTHREAD_MUTEX_TYPE_ELISION (mutex),
			    PTHREAD_MUTEX_TIMED_NP))
    {
      /* Recursive mutex.  */
    case PTHREAD_MUTEX_RECURSIVE_NP|PTHREAD_MUTEX_ELISION_NP:
    case PTHREAD_MUTEX_RECURSIVE_NP:
      /* Check whether we already hold the mutex.  */
      if (mutex->__data.__owner == id)
	{
	  /* Just bump the counter.  */
	  if (__glibc_unlikely (mutex->__data.__count + 1 == 0))
	    /* Overflow of the counter.  */
	    return EAGAIN;

	  ++mutex->__data.__count;

	  goto out;
	}

      /* We have to get the mutex.  */
      result = lll_timedlock (mutex->__data.__lock, abstime,
			      PTHREAD_MUTEX_PSHARED (mutex));

      if (result != 0)
	goto out;

      /* Only locked once so far.  */
      mutex->__data.__count = 1;
      break;

      /* Error checking mutex.  */
    case PTHREAD_MUTEX_ERRORCHECK_NP:
      /* Check whether we already hold the mutex.  */
      if (__glibc_unlikely (mutex->__data.__owner == id))
	return EDEADLK;

      /* Don't do lock elision on an error checking mutex.  */
      goto simple;

    case PTHREAD_MUTEX_TIMED_NP:
      FORCE_ELISION (mutex, goto elision);
    simple:
      /* Normal mutex.  */
      result = lll_timedlock (mutex->__data.__lock, abstime,
			      PTHREAD_MUTEX_PSHARED (mutex));
      break;

    case PTHREAD_MUTEX_TIMED_ELISION_NP:
    elision: __attribute__((unused))
      /* Don't record ownership */
      return lll_timedlock_elision (mutex->__data.__lock,
				    mutex->__data.__spins,
				    abstime,
				    PTHREAD_MUTEX_PSHARED (mutex));


    case PTHREAD_MUTEX_ADAPTIVE_NP:
      if (! __is_smp)
	goto simple;

      if (lll_trylock (mutex->__data.__lock) != 0)
	{
	  int cnt = 0;
	  int max_cnt = MIN (MAX_ADAPTIVE_COUNT,
			     mutex->__data.__spins * 2 + 10);
	  do
	    {
	      if (cnt++ >= max_cnt)
		{
		  result = lll_timedlock (mutex->__data.__lock, abstime,
					  PTHREAD_MUTEX_PSHARED (mutex));
		  break;
		}
	      atomic_spin_nop ();
	    }
	  while (lll_trylock (mutex->__data.__lock) != 0);

	  mutex->__data.__spins += (cnt - mutex->__data.__spins) / 8;
	}
      break;

    case PTHREAD_MUTEX_ROBUST_RECURSIVE_NP:
    case PTHREAD_MUTEX_ROBUST_ERRORCHECK_NP:
    case PTHREAD_MUTEX_ROBUST_NORMAL_NP:
    case PTHREAD_MUTEX_ROBUST_ADAPTIVE_NP:
      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
		     &mutex->__data.__list.__next);
      /* We need to set op_pending before starting the operation.  Also
	 see comments at ENQUEUE_MUTEX.  */
      __asm ("" ::: "memory");

      oldval = mutex->__data.__lock;
      /* This is set to FUTEX_WAITERS iff we might have shared the
	 FUTEX_WAITERS flag with other threads, and therefore need to keep it
	 set to avoid lost wake-ups.  We have the same requirement in the
	 simple mutex algorithm.  */
      unsigned int assume_other_futex_waiters = 0;
      while (1)
	{
	  /* Try to acquire the lock through a CAS from 0 (not acquired) to
	     our TID | assume_other_futex_waiters.  */
	  if (__glibc_likely (oldval == 0))
	    {
	      oldval
	        = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
	            id | assume_other_futex_waiters, 0);
	      if (__glibc_likely (oldval == 0))
		break;
	    }

	  if ((oldval & FUTEX_OWNER_DIED) != 0)
	    {
	      /* The previous owner died.  Try locking the mutex.  */
	      int newval = id | (oldval & FUTEX_WAITERS)
		  | assume_other_futex_waiters;

	      newval
		= atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						       newval, oldval);
	      if (newval != oldval)
		{
		  oldval = newval;
		  continue;
		}

	      /* We got the mutex.  */
	      mutex->__data.__count = 1;
	      /* But it is inconsistent unless marked otherwise.  */
	      mutex->__data.__owner = PTHREAD_MUTEX_INCONSISTENT;

	      /* We must not enqueue the mutex before we have acquired it.
		 Also see comments at ENQUEUE_MUTEX.  */
	      __asm ("" ::: "memory");
	      ENQUEUE_MUTEX (mutex);
	      /* We need to clear op_pending after we enqueue the mutex.  */
	      __asm ("" ::: "memory");
	      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);

	      /* Note that we deliberately exit here.  If we fall
		 through to the end of the function __nusers would be
		 incremented which is not correct because the old
		 owner has to be discounted.  */
	      return EOWNERDEAD;
	    }

	  /* Check whether we already hold the mutex.  */
	  if (__glibc_unlikely ((oldval & FUTEX_TID_MASK) == id))
	    {
	      int kind = PTHREAD_MUTEX_TYPE (mutex);
	      if (kind == PTHREAD_MUTEX_ROBUST_ERRORCHECK_NP)
		{
		  /* We do not need to ensure ordering wrt another memory
		     access.  Also see comments at ENQUEUE_MUTEX. */
		  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
				 NULL);
		  return EDEADLK;
		}

	      if (kind == PTHREAD_MUTEX_ROBUST_RECURSIVE_NP)
		{
		  /* We do not need to ensure ordering wrt another memory
		     access.  */
		  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
				 NULL);

		  /* Just bump the counter.  */
		  if (__glibc_unlikely (mutex->__data.__count + 1 == 0))
		    /* Overflow of the counter.  */
		    return EAGAIN;

		  ++mutex->__data.__count;

		  LIBC_PROBE (mutex_timedlock_acquired, 1, mutex);

		  return 0;
		}
	    }

	  /* We are about to block; check whether the timeout is invalid.  */
	  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
	    return EINVAL;
	  /* Work around the fact that the kernel rejects negative timeout
	     values despite them being valid.  */
	  if (__glibc_unlikely (abstime->tv_sec < 0))
	    return ETIMEDOUT;
#if (!defined __ASSUME_FUTEX_CLOCK_REALTIME \
     || !defined lll_futex_timed_wait_bitset)
	  struct timeval tv;
	  struct timespec rt;

	  /* Get the current time.  */
	  (void) __gettimeofday (&tv, NULL);

	  /* Compute relative timeout.  */
	  rt.tv_sec = abstime->tv_sec - tv.tv_sec;
	  rt.tv_nsec = abstime->tv_nsec - tv.tv_usec * 1000;
	  if (rt.tv_nsec < 0)
	    {
	      rt.tv_nsec += 1000000000;
	      --rt.tv_sec;
	    }

	  /* Already timed out?  */
	  if (rt.tv_sec < 0)
	    return ETIMEDOUT;
#endif

	  /* We cannot acquire the mutex nor has its owner died.  Thus, try
	     to block using futexes.  Set FUTEX_WAITERS if necessary so that
	     other threads are aware that there are potentially threads
	     blocked on the futex.  Restart if oldval changed in the
	     meantime.  */
	  if ((oldval & FUTEX_WAITERS) == 0)
	    {
	      if (atomic_compare_and_exchange_bool_acq (&mutex->__data.__lock,
							oldval | FUTEX_WAITERS,
							oldval)
		  != 0)
		{
		  oldval = mutex->__data.__lock;
		  continue;
		}
	      oldval |= FUTEX_WAITERS;
	    }

	  /* It is now possible that we share the FUTEX_WAITERS flag with
	     another thread; therefore, update assume_other_futex_waiters so
	     that we do not forget about this when handling other cases
	     above and thus do not cause lost wake-ups.  */
	  assume_other_futex_waiters |= FUTEX_WAITERS;

	  /* Block using the futex.  */
#if (!defined __ASSUME_FUTEX_CLOCK_REALTIME \
     || !defined lll_futex_timed_wait_bitset)
	  lll_futex_timed_wait (&mutex->__data.__lock, oldval,
				&rt, PTHREAD_ROBUST_MUTEX_PSHARED (mutex));
#else
	  int err = lll_futex_timed_wait_bitset (&mutex->__data.__lock,
	      oldval, abstime, FUTEX_CLOCK_REALTIME,
	      PTHREAD_ROBUST_MUTEX_PSHARED (mutex));
	  /* The futex call timed out.  */
	  if (err == -ETIMEDOUT)
	    return -err;
#endif
	  /* Reload current lock value.  */
	  oldval = mutex->__data.__lock;
	}

      /* We have acquired the mutex; check if it is still consistent.  */
      if (__builtin_expect (mutex->__data.__owner
			    == PTHREAD_MUTEX_NOTRECOVERABLE, 0))
	{
	  /* This mutex is now not recoverable.  */
	  mutex->__data.__count = 0;
	  int private = PTHREAD_ROBUST_MUTEX_PSHARED (mutex);
	  lll_unlock (mutex->__data.__lock, private);
	  /* FIXME This violates the mutex destruction requirements.  See
	     __pthread_mutex_unlock_full.  */
	  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
	  return ENOTRECOVERABLE;
	}

      mutex->__data.__count = 1;
      /* We must not enqueue the mutex before we have acquired it.
	 Also see comments at ENQUEUE_MUTEX.  */
      __asm ("" ::: "memory");
      ENQUEUE_MUTEX (mutex);
      /* We need to clear op_pending after we enqueue the mutex.  */
      __asm ("" ::: "memory");
      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
      break;

    /* The PI support requires the Linux futex system call.  If that's not
       available, pthread_mutex_init should never have allowed the type to
       be set.  So it will get the default case for an invalid type.  */
#ifdef __NR_futex
    case PTHREAD_MUTEX_PI_RECURSIVE_NP:
    case PTHREAD_MUTEX_PI_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PI_NORMAL_NP:
    case PTHREAD_MUTEX_PI_ADAPTIVE_NP:
    case PTHREAD_MUTEX_PI_ROBUST_RECURSIVE_NP:
    case PTHREAD_MUTEX_PI_ROBUST_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PI_ROBUST_NORMAL_NP:
    case PTHREAD_MUTEX_PI_ROBUST_ADAPTIVE_NP:
      {
	int kind = mutex->__data.__kind & PTHREAD_MUTEX_KIND_MASK_NP;
	int robust = mutex->__data.__kind & PTHREAD_MUTEX_ROBUST_NORMAL_NP;

	if (robust)
	  {
	    /* Note: robust PI futexes are signaled by setting bit 0.  */
	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
			   (void *) (((uintptr_t) &mutex->__data.__list.__next)
				     | 1));
	    /* We need to set op_pending before starting the operation.  Also
	       see comments at ENQUEUE_MUTEX.  */
	    __asm ("" ::: "memory");
	  }

	oldval = mutex->__data.__lock;

	/* Check whether we already hold the mutex.  */
	if (__glibc_unlikely ((oldval & FUTEX_TID_MASK) == id))
	  {
	    if (kind == PTHREAD_MUTEX_ERRORCHECK_NP)
	      {
		/* We do not need to ensure ordering wrt another memory
		   access.  */
		THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
		return EDEADLK;
	      }

	    if (kind == PTHREAD_MUTEX_RECURSIVE_NP)
	      {
		/* We do not need to ensure ordering wrt another memory
		   access.  */
		THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);

		/* Just bump the counter.  */
		if (__glibc_unlikely (mutex->__data.__count + 1 == 0))
		  /* Overflow of the counter.  */
		  return EAGAIN;

		++mutex->__data.__count;

		LIBC_PROBE (mutex_timedlock_acquired, 1, mutex);

		return 0;
	      }
	  }

	oldval = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						      id, 0);

	if (oldval != 0)
	  {
	    /* The mutex is locked.  The kernel will now take care of
	       everything.  The timeout value must be a relative value.
	       Convert it.  */
	    int private = (robust
			   ? PTHREAD_ROBUST_MUTEX_PSHARED (mutex)
			   : PTHREAD_MUTEX_PSHARED (mutex));
	    INTERNAL_SYSCALL_DECL (__err);

	    int e = INTERNAL_SYSCALL (futex, __err, 4, &mutex->__data.__lock,
				      __lll_private_flag (FUTEX_LOCK_PI,
							  private), 1,
				      abstime);
	    if (INTERNAL_SYSCALL_ERROR_P (e, __err))
	      {
		if (INTERNAL_SYSCALL_ERRNO (e, __err) == ETIMEDOUT)
		  return ETIMEDOUT;

		if (INTERNAL_SYSCALL_ERRNO (e, __err) == ESRCH
		    || INTERNAL_SYSCALL_ERRNO (e, __err) == EDEADLK)
		  {
		    assert (INTERNAL_SYSCALL_ERRNO (e, __err) != EDEADLK
			    || (kind != PTHREAD_MUTEX_ERRORCHECK_NP
				&& kind != PTHREAD_MUTEX_RECURSIVE_NP));
		    /* ESRCH can happen only for non-robust PI mutexes where
		       the owner of the lock died.  */
		    assert (INTERNAL_SYSCALL_ERRNO (e, __err) != ESRCH
			    || !robust);

		    /* Delay the thread until the timeout is reached.
		       Then return ETIMEDOUT.  */
		    struct timespec reltime;
		    struct timespec now;

		    INTERNAL_SYSCALL (clock_gettime, __err, 2, CLOCK_REALTIME,
				      &now);
		    reltime.tv_sec = abstime->tv_sec - now.tv_sec;
		    reltime.tv_nsec = abstime->tv_nsec - now.tv_nsec;
		    if (reltime.tv_nsec < 0)
		      {
			reltime.tv_nsec += 1000000000;
			--reltime.tv_sec;
		      }
		    if (reltime.tv_sec >= 0)
		      while (__nanosleep_nocancel (&reltime, &reltime) != 0)
			continue;

		    return ETIMEDOUT;
		  }

		return INTERNAL_SYSCALL_ERRNO (e, __err);
	      }

	    oldval = mutex->__data.__lock;

	    assert (robust || (oldval & FUTEX_OWNER_DIED) == 0);
	  }

	if (__glibc_unlikely (oldval & FUTEX_OWNER_DIED))
	  {
	    atomic_and (&mutex->__data.__lock, ~FUTEX_OWNER_DIED);

	    /* We got the mutex.  */
	    mutex->__data.__count = 1;
	    /* But it is inconsistent unless marked otherwise.  */
	    mutex->__data.__owner = PTHREAD_MUTEX_INCONSISTENT;

	    /* We must not enqueue the mutex before we have acquired it.
	       Also see comments at ENQUEUE_MUTEX.  */
	    __asm ("" ::: "memory");
	    ENQUEUE_MUTEX_PI (mutex);
	    /* We need to clear op_pending after we enqueue the mutex.  */
	    __asm ("" ::: "memory");
	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);

	    /* Note that we deliberately exit here.  If we fall
	       through to the end of the function __nusers would be
	       incremented which is not correct because the old owner
	       has to be discounted.  */
	    return EOWNERDEAD;
	  }

	if (robust
	    && __builtin_expect (mutex->__data.__owner
				 == PTHREAD_MUTEX_NOTRECOVERABLE, 0))
	  {
	    /* This mutex is now not recoverable.  */
	    mutex->__data.__count = 0;

	    INTERNAL_SYSCALL_DECL (__err);
	    INTERNAL_SYSCALL (futex, __err, 4, &mutex->__data.__lock,
			      __lll_private_flag (FUTEX_UNLOCK_PI,
						  PTHREAD_ROBUST_MUTEX_PSHARED (mutex)),
			      0, 0);

	    /* To the kernel, this will be visible after the kernel has
	       acquired the mutex in the syscall.  */
	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
	    return ENOTRECOVERABLE;
	  }

	mutex->__data.__count = 1;
	if (robust)
	  {
	    /* We must not enqueue the mutex before we have acquired it.
	       Also see comments at ENQUEUE_MUTEX.  */
	    __asm ("" ::: "memory");
	    ENQUEUE_MUTEX_PI (mutex);
	    /* We need to clear op_pending after we enqueue the mutex.  */
	    __asm ("" ::: "memory");
	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
	  }
	}
      break;
#endif  /* __NR_futex.  */

    case PTHREAD_MUTEX_PP_RECURSIVE_NP:
    case PTHREAD_MUTEX_PP_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PP_NORMAL_NP:
    case PTHREAD_MUTEX_PP_ADAPTIVE_NP:
      {
	int kind = mutex->__data.__kind & PTHREAD_MUTEX_KIND_MASK_NP;

	oldval = mutex->__data.__lock;

	/* Check whether we already hold the mutex.  */
	if (mutex->__data.__owner == id)
	  {
	    if (kind == PTHREAD_MUTEX_ERRORCHECK_NP)
	      return EDEADLK;

	    if (kind == PTHREAD_MUTEX_RECURSIVE_NP)
	      {
		/* Just bump the counter.  */
		if (__glibc_unlikely (mutex->__data.__count + 1 == 0))
		  /* Overflow of the counter.  */
		  return EAGAIN;

		++mutex->__data.__count;

		LIBC_PROBE (mutex_timedlock_acquired, 1, mutex);

		return 0;
	      }
	  }

	int oldprio = -1, ceilval;
	do
	  {
	    int ceiling = (oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK)
			  >> PTHREAD_MUTEX_PRIO_CEILING_SHIFT;

	    if (__pthread_current_priority () > ceiling)
	      {
		result = EINVAL;
	      failpp:
		if (oldprio != -1)
		  __pthread_tpp_change_priority (oldprio, -1);
		return result;
	      }

	    result = __pthread_tpp_change_priority (oldprio, ceiling);
	    if (result)
	      return result;

	    ceilval = ceiling << PTHREAD_MUTEX_PRIO_CEILING_SHIFT;
	    oldprio = ceiling;

	    oldval
	      = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						     ceilval | 1, ceilval);

	    if (oldval == ceilval)
	      break;

	    do
	      {
		oldval
		  = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
							 ceilval | 2,
							 ceilval | 1);

		if ((oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK) != ceilval)
		  break;

		if (oldval != ceilval)
		  {
		    /* Reject invalid timeouts.  */
		    if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
		      {
			result = EINVAL;
			goto failpp;
		      }

		    struct timeval tv;
		    struct timespec rt;

		    /* Get the current time.  */
		    (void) __gettimeofday (&tv, NULL);

		    /* Compute relative timeout.  */
		    rt.tv_sec = abstime->tv_sec - tv.tv_sec;
		    rt.tv_nsec = abstime->tv_nsec - tv.tv_usec * 1000;
		    if (rt.tv_nsec < 0)
		      {
			rt.tv_nsec += 1000000000;
			--rt.tv_sec;
		      }

		    /* Already timed out?  */
		    if (rt.tv_sec < 0)
		      {
			result = ETIMEDOUT;
			goto failpp;
		      }

		    lll_futex_timed_wait (&mutex->__data.__lock,
					  ceilval | 2, &rt,
					  PTHREAD_MUTEX_PSHARED (mutex));
		  }
	      }
	    while (atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
							ceilval | 2, ceilval)
		   != ceilval);
	  }
	while ((oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK) != ceilval);

	assert (mutex->__data.__owner == 0);
	mutex->__data.__count = 1;
      }
      break;

    default:
      /* Correct code cannot set any other type.  */
      return EINVAL;
    }

  if (result == 0)
    {
      /* Record the ownership.  */
      mutex->__data.__owner = id;
      ++mutex->__data.__nusers;

      LIBC_PROBE (mutex_timedlock_acquired, 1, mutex);
    }

 out:
  return result;
}
weak_alias (__pthread_mutex_timedlock, pthread_mutex_timedlock)
