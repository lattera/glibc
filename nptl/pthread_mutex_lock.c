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
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <not-cancel.h>
#include "pthreadP.h"
#include <atomic.h>
#include <lowlevellock.h>
#include <stap-probe.h>

#ifndef lll_lock_elision
#define lll_lock_elision(lock, try_lock, private)	({ \
      lll_lock (lock, private); 0; })
#endif

#ifndef lll_trylock_elision
#define lll_trylock_elision(a,t) lll_trylock(a)
#endif

/* Some of the following definitions differ when pthread_mutex_cond_lock.c
   includes this file.  */
#ifndef LLL_MUTEX_LOCK
# define LLL_MUTEX_LOCK(mutex) \
  lll_lock ((mutex)->__data.__lock, PTHREAD_MUTEX_PSHARED (mutex))
# define LLL_MUTEX_TRYLOCK(mutex) \
  lll_trylock ((mutex)->__data.__lock)
# define LLL_ROBUST_MUTEX_LOCK_MODIFIER 0
# define LLL_MUTEX_LOCK_ELISION(mutex) \
  lll_lock_elision ((mutex)->__data.__lock, (mutex)->__data.__elision, \
		   PTHREAD_MUTEX_PSHARED (mutex))
# define LLL_MUTEX_TRYLOCK_ELISION(mutex) \
  lll_trylock_elision((mutex)->__data.__lock, (mutex)->__data.__elision, \
		   PTHREAD_MUTEX_PSHARED (mutex))
#endif

#ifndef FORCE_ELISION
#define FORCE_ELISION(m, s)
#endif

static int __pthread_mutex_lock_full (pthread_mutex_t *mutex)
     __attribute_noinline__;

int
__pthread_mutex_lock (pthread_mutex_t *mutex)
{
  unsigned int type = PTHREAD_MUTEX_TYPE_ELISION (mutex);

  LIBC_PROBE (mutex_entry, 1, mutex);

  if (__builtin_expect (type & ~(PTHREAD_MUTEX_KIND_MASK_NP
				 | PTHREAD_MUTEX_ELISION_FLAGS_NP), 0))
    return __pthread_mutex_lock_full (mutex);

  if (__glibc_likely (type == PTHREAD_MUTEX_TIMED_NP))
    {
      FORCE_ELISION (mutex, goto elision);
    simple:
      /* Normal mutex.  */
      LLL_MUTEX_LOCK (mutex);
      assert (mutex->__data.__owner == 0);
    }
#ifdef HAVE_ELISION
  else if (__glibc_likely (type == PTHREAD_MUTEX_TIMED_ELISION_NP))
    {
  elision: __attribute__((unused))
      /* This case can never happen on a system without elision,
         as the mutex type initialization functions will not
	 allow to set the elision flags.  */
      /* Don't record owner or users for elision case.  This is a
         tail call.  */
      return LLL_MUTEX_LOCK_ELISION (mutex);
    }
#endif
  else if (__builtin_expect (PTHREAD_MUTEX_TYPE (mutex)
			     == PTHREAD_MUTEX_RECURSIVE_NP, 1))
    {
      /* Recursive mutex.  */
      pid_t id = THREAD_GETMEM (THREAD_SELF, tid);

      /* Check whether we already hold the mutex.  */
      if (mutex->__data.__owner == id)
	{
	  /* Just bump the counter.  */
	  if (__glibc_unlikely (mutex->__data.__count + 1 == 0))
	    /* Overflow of the counter.  */
	    return EAGAIN;

	  ++mutex->__data.__count;

	  return 0;
	}

      /* We have to get the mutex.  */
      LLL_MUTEX_LOCK (mutex);

      assert (mutex->__data.__owner == 0);
      mutex->__data.__count = 1;
    }
  else if (__builtin_expect (PTHREAD_MUTEX_TYPE (mutex)
			  == PTHREAD_MUTEX_ADAPTIVE_NP, 1))
    {
      if (! __is_smp)
	goto simple;

      if (LLL_MUTEX_TRYLOCK (mutex) != 0)
	{
	  int cnt = 0;
	  int max_cnt = MIN (MAX_ADAPTIVE_COUNT,
			     mutex->__data.__spins * 2 + 10);
	  do
	    {
	      if (cnt++ >= max_cnt)
		{
		  LLL_MUTEX_LOCK (mutex);
		  break;
		}
	      atomic_spin_nop ();
	    }
	  while (LLL_MUTEX_TRYLOCK (mutex) != 0);

	  mutex->__data.__spins += (cnt - mutex->__data.__spins) / 8;
	}
      assert (mutex->__data.__owner == 0);
    }
  else
    {
      pid_t id = THREAD_GETMEM (THREAD_SELF, tid);
      assert (PTHREAD_MUTEX_TYPE (mutex) == PTHREAD_MUTEX_ERRORCHECK_NP);
      /* Check whether we already hold the mutex.  */
      if (__glibc_unlikely (mutex->__data.__owner == id))
	return EDEADLK;
      goto simple;
    }

  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);

  /* Record the ownership.  */
  mutex->__data.__owner = id;
#ifndef NO_INCR
  ++mutex->__data.__nusers;
#endif

  LIBC_PROBE (mutex_acquired, 1, mutex);

  return 0;
}

static int
__pthread_mutex_lock_full (pthread_mutex_t *mutex)
{
  int oldval;
  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);

  switch (PTHREAD_MUTEX_TYPE (mutex))
    {
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
	 simple mutex algorithm.
	 We start with value zero for a normal mutex, and FUTEX_WAITERS if we
	 are building the special case mutexes for use from within condition
	 variables.  */
      unsigned int assume_other_futex_waiters = LLL_ROBUST_MUTEX_LOCK_MODIFIER;
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
	      int newval = id;
#ifdef NO_INCR
	      /* We are not taking assume_other_futex_waiters into accoount
		 here simply because we'll set FUTEX_WAITERS anyway.  */
	      newval |= FUTEX_WAITERS;
#else
	      newval |= (oldval & FUTEX_WAITERS) | assume_other_futex_waiters;
#endif

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
		 owner has to be discounted.  If we are not supposed
		 to increment __nusers we actually have to decrement
		 it here.  */
#ifdef NO_INCR
	      --mutex->__data.__nusers;
#endif

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

		  return 0;
		}
	    }

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

	  /* Block using the futex and reload current lock value.  */
	  lll_futex_wait (&mutex->__data.__lock, oldval,
			  PTHREAD_ROBUST_MUTEX_PSHARED (mutex));
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

		return 0;
	      }
	  }

	int newval = id;
# ifdef NO_INCR
	newval |= FUTEX_WAITERS;
# endif
	oldval = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						      newval, 0);

	if (oldval != 0)
	  {
	    /* The mutex is locked.  The kernel will now take care of
	       everything.  */
	    int private = (robust
			   ? PTHREAD_ROBUST_MUTEX_PSHARED (mutex)
			   : PTHREAD_MUTEX_PSHARED (mutex));
	    INTERNAL_SYSCALL_DECL (__err);
	    int e = INTERNAL_SYSCALL (futex, __err, 4, &mutex->__data.__lock,
				      __lll_private_flag (FUTEX_LOCK_PI,
							  private), 1, 0);

	    if (INTERNAL_SYSCALL_ERROR_P (e, __err)
		&& (INTERNAL_SYSCALL_ERRNO (e, __err) == ESRCH
		    || INTERNAL_SYSCALL_ERRNO (e, __err) == EDEADLK))
	      {
		assert (INTERNAL_SYSCALL_ERRNO (e, __err) != EDEADLK
			|| (kind != PTHREAD_MUTEX_ERRORCHECK_NP
			    && kind != PTHREAD_MUTEX_RECURSIVE_NP));
		/* ESRCH can happen only for non-robust PI mutexes where
		   the owner of the lock died.  */
		assert (INTERNAL_SYSCALL_ERRNO (e, __err) != ESRCH || !robust);

		/* Delay the thread indefinitely.  */
		while (1)
		  __pause_nocancel ();
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
	       has to be discounted.  If we are not supposed to
	       increment __nusers we actually have to decrement it here.  */
# ifdef NO_INCR
	    --mutex->__data.__nusers;
# endif

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
		if (oldprio != -1)
		  __pthread_tpp_change_priority (oldprio, -1);
		return EINVAL;
	      }

	    int retval = __pthread_tpp_change_priority (oldprio, ceiling);
	    if (retval)
	      return retval;

	    ceilval = ceiling << PTHREAD_MUTEX_PRIO_CEILING_SHIFT;
	    oldprio = ceiling;

	    oldval
	      = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
#ifdef NO_INCR
						     ceilval | 2,
#else
						     ceilval | 1,
#endif
						     ceilval);

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
		  lll_futex_wait (&mutex->__data.__lock, ceilval | 2,
				  PTHREAD_MUTEX_PSHARED (mutex));
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

  /* Record the ownership.  */
  mutex->__data.__owner = id;
#ifndef NO_INCR
  ++mutex->__data.__nusers;
#endif

  LIBC_PROBE (mutex_acquired, 1, mutex);

  return 0;
}
#ifndef __pthread_mutex_lock
weak_alias (__pthread_mutex_lock, pthread_mutex_lock)
hidden_def (__pthread_mutex_lock)
#endif


#ifdef NO_INCR
void
__pthread_mutex_cond_lock_adjust (pthread_mutex_t *mutex)
{
  assert ((mutex->__data.__kind & PTHREAD_MUTEX_PRIO_INHERIT_NP) != 0);
  assert ((mutex->__data.__kind & PTHREAD_MUTEX_ROBUST_NORMAL_NP) == 0);
  assert ((mutex->__data.__kind & PTHREAD_MUTEX_PSHARED_BIT) == 0);

  /* Record the ownership.  */
  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);
  mutex->__data.__owner = id;

  if (mutex->__data.__kind == PTHREAD_MUTEX_PI_RECURSIVE_NP)
    ++mutex->__data.__count;
}
#endif
