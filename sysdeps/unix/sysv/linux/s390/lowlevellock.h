/* Copyright (C) 2003-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

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

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <atomic.h>
#include <kernel-features.h>

#define SYS_futex		238
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_OP_CLEAR_WAKE_IF_GT_ONE	((4 << 24) | 1)
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_WAIT_REQUEUE_PI   11
#define FUTEX_CMP_REQUEUE_PI    12
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE	0
#define LLL_SHARED	FUTEX_PRIVATE_FLAG


#if IS_IN (libc) || IS_IN (rtld)
/* In libc.so or ld.so all futexes are private.  */
# ifdef __ASSUME_PRIVATE_FUTEX
#  define __lll_private_flag(fl, private) \
  ((fl) | FUTEX_PRIVATE_FLAG)
# else
#  define __lll_private_flag(fl, private) \
  ((fl) | THREAD_GETMEM (THREAD_SELF, header.private_futex))
# endif
#else
# ifdef __ASSUME_PRIVATE_FUTEX
#  define __lll_private_flag(fl, private) \
  (((fl) | FUTEX_PRIVATE_FLAG) ^ (private))
# else
#  define __lll_private_flag(fl, private) \
  (__builtin_constant_p (private)					      \
   ? ((private) == 0							      \
      ? ((fl) | THREAD_GETMEM (THREAD_SELF, header.private_futex))	      \
      : (fl))								      \
   : ((fl) | (((private) ^ FUTEX_PRIVATE_FLAG)				      \
	      & THREAD_GETMEM (THREAD_SELF, header.private_futex))))
# endif
#endif

#define lll_futex_wait(futex, val, private) \
  lll_futex_timed_wait (futex, val, NULL, private)

#define lll_futex_timed_wait(futexp, val, timespec, private) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
									      \
    INTERNAL_SYSCALL (futex, __err, 4, (futexp),		      \
			      __lll_private_flag (FUTEX_WAIT, private),	      \
			      (val), (timespec));			      \
  })

#define lll_futex_timed_wait_bitset(futexp, val, timespec, clockbit, private) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    int __op = FUTEX_WAIT_BITSET | clockbit;				      \
									      \
    INTERNAL_SYSCALL (futex, __err, 6, (futexp),		      \
			      __lll_private_flag (__op, private),	      \
			      (val), (timespec), NULL /* Unused.  */, 	      \
			      FUTEX_BITSET_MATCH_ANY);			      \
  })

#define lll_futex_wake(futexp, nr, private) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
									      \
    INTERNAL_SYSCALL (futex, __err, 4, (futexp),		      \
			      __lll_private_flag (FUTEX_WAKE, private),	      \
			      (nr), 0);					      \
  })


/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(futexp, nr_wake, nr_move, mutex, val, private) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 6, (futexp),		      \
			      __lll_private_flag (FUTEX_CMP_REQUEUE, private),\
			      (nr_wake), (nr_move), (mutex), (val));	      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err);				      \
  })

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_wake_unlock(futexp, nr_wake, nr_wake2, futexp2, private) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 6, (futexp),		      \
			      __lll_private_flag (FUTEX_WAKE_OP, private),    \
			      (nr_wake), (nr_wake2), (futexp2),		      \
			      FUTEX_OP_CLEAR_WAKE_IF_GT_ONE);		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err);				      \
  })

/* Priority Inheritance support.  */
#define lll_futex_wait_requeue_pi(futexp, val, mutex, private) \
  lll_futex_timed_wait_requeue_pi (futexp, val, NULL, 0, mutex, private)

#define lll_futex_timed_wait_requeue_pi(futexp, val, timespec, clockbit,      \
					mutex, private)			      \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    int __op = FUTEX_WAIT_REQUEUE_PI | clockbit;			      \
									      \
    INTERNAL_SYSCALL (futex, __err, 5, (futexp),			      \
		      __lll_private_flag (__op, private),		      \
		      (val), (timespec), mutex); 			      \
  })

#define lll_futex_cmp_requeue_pi(futexp, nr_wake, nr_move, mutex, val, priv)  \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 6, (futexp),		      \
			      __lll_private_flag (FUTEX_CMP_REQUEUE_PI, priv),\
			      (nr_wake), (nr_move), (mutex), (val));	      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err);				      \
  })

#define lll_compare_and_swap(futex, oldval, newval, operation) \
  do {									      \
    __typeof (futex) __futex = (futex);					      \
    __asm __volatile ("	  l   %1,%0\n"					      \
		      "0: " operation "\n"				      \
		      "	  cs  %1,%2,%0\n"				      \
		      "	  jl  0b\n"					      \
		      "1:"						      \
		      : "=Q" (*__futex), "=&d" (oldval), "=&d" (newval)	      \
		      : "m" (*__futex) : "cc", "memory" );		      \
  } while (0)


static inline int
__attribute__ ((always_inline))
__lll_trylock (int *futex)
{
    unsigned int old;

    __asm __volatile ("cs %0,%3,%1"
		       : "=d" (old), "=Q" (*futex)
		       : "0" (0), "d" (1), "m" (*futex) : "cc", "memory" );
    return old != 0;
}
#define lll_trylock(futex) __lll_trylock (&(futex))


static inline int
__attribute__ ((always_inline))
__lll_cond_trylock (int *futex)
{
    unsigned int old;

    __asm __volatile ("cs %0,%3,%1"
		       : "=d" (old), "=Q" (*futex)
		       : "0" (0), "d" (2), "m" (*futex) : "cc", "memory" );
    return old != 0;
}
#define lll_cond_trylock(futex) __lll_cond_trylock (&(futex))


extern void __lll_lock_wait_private (int *futex) attribute_hidden;
extern void __lll_lock_wait (int *futex, int private) attribute_hidden;
extern int __lll_robust_lock_wait (int *futex, int private) attribute_hidden;

static inline void
__attribute__ ((always_inline))
__lll_lock (int *futex, int private)
{
  if (__glibc_unlikely (atomic_compare_and_exchange_bool_acq (futex, 1, 0)))
    {
      if (__builtin_constant_p (private) && private == LLL_PRIVATE)
	__lll_lock_wait_private (futex);
      else
	__lll_lock_wait (futex, private);
    }
}
#define lll_lock(futex, private) __lll_lock (&(futex), private)

static inline int
__attribute__ ((always_inline))
__lll_robust_lock (int *futex, int id, int private)
{
  int result = 0;
  if (__builtin_expect (atomic_compare_and_exchange_bool_acq (futex, id, 0),
			0))
    result = __lll_robust_lock_wait (futex, private);
  return result;
}
#define lll_robust_lock(futex, id, private) \
  __lll_robust_lock (&(futex), id, private)

static inline void
__attribute__ ((always_inline))
__lll_cond_lock (int *futex, int private)
{
  if (__glibc_unlikely (atomic_compare_and_exchange_bool_acq (futex, 2, 0)))
    __lll_lock_wait (futex, private);
}
#define lll_cond_lock(futex, private) __lll_cond_lock (&(futex), private)

#define lll_robust_cond_lock(futex, id, private) \
  __lll_robust_lock (&(futex), (id) | FUTEX_WAITERS, private)

extern int __lll_timedlock_wait
  (int *futex, const struct timespec *, int private) attribute_hidden;
extern int __lll_robust_timedlock_wait
  (int *futex, const struct timespec *, int private) attribute_hidden;

static inline int
__attribute__ ((always_inline))
__lll_timedlock (int *futex, const struct timespec *abstime, int private)
{
  int result = 0;
  if (__glibc_unlikely (atomic_compare_and_exchange_bool_acq (futex, 1, 0)))
    result = __lll_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_timedlock(futex, abstime, private) \
  __lll_timedlock (&(futex), abstime, private)

#ifdef ENABLE_LOCK_ELISION
extern int __lll_timedlock_elision
  (int *futex, short *adapt_count, const struct timespec *timeout, int private)
  attribute_hidden;

# define lll_timedlock_elision(futex, adapt_count, timeout, private)	\
  __lll_timedlock_elision(&(futex), &(adapt_count), timeout, private)
#endif

static inline int
__attribute__ ((always_inline))
__lll_robust_timedlock (int *futex, const struct timespec *abstime,
			int id, int private)
{
  int result = 0;
  if (__builtin_expect (atomic_compare_and_exchange_bool_acq (futex, id, 0),
			0))
    result = __lll_robust_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_robust_timedlock(futex, abstime, id, private) \
  __lll_robust_timedlock (&(futex), abstime, id, private)


#define __lll_unlock(futex, private) \
  (void)								      \
    ({ int __oldval;							      \
       int __newval = 0;						      \
       int *__futexp = (futex);						      \
									      \
       lll_compare_and_swap (__futexp, __oldval, __newval, "slr %2,%2");      \
       if (__glibc_unlikely (__oldval > 1))				      \
	 lll_futex_wake (__futexp, 1, private);				      \
    })
#define lll_unlock(futex, private) __lll_unlock(&(futex), private)


#define __lll_robust_unlock(futex, private) \
  (void)								      \
    ({ int __oldval;							      \
       int __newval = 0;						      \
       int *__futexp = (futex);						      \
									      \
       lll_compare_and_swap (__futexp, __oldval, __newval, "slr %2,%2");      \
       if (__glibc_unlikely (__oldval & FUTEX_WAITERS))			      \
	 lll_futex_wake (__futexp, 1, private);				      \
    })
#define lll_robust_unlock(futex, private) \
  __lll_robust_unlock(&(futex), private)

#define lll_islocked(futex) \
  (futex != 0)


/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

/* The kernel notifies a process which uses CLONE_CHILD_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.	*/
#define __lll_wait_tid(ptid) \
  do									      \
    {									      \
      int __tid;							      \
									      \
      while ((__tid = *ptid) != 0)					      \
	lll_futex_wait (ptid, __tid, LLL_SHARED);			      \
    }									      \
  while (0)
#define lll_wait_tid(tid) __lll_wait_tid(&(tid))

extern int __lll_timedwait_tid (int *, const struct timespec *)
     attribute_hidden;

#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __res = 0;							      \
    if ((tid) != 0)							      \
      __res = __lll_timedwait_tid (&(tid), (abstime));			      \
    __res;								      \
  })

#ifdef ENABLE_LOCK_ELISION
extern int __lll_lock_elision (int *futex, short *adapt_count, int private)
  attribute_hidden;

extern int __lll_unlock_elision(int *futex, int private)
  attribute_hidden;

extern int __lll_trylock_elision(int *futex, short *adapt_count)
  attribute_hidden;

# define lll_lock_elision(futex, adapt_count, private) \
  __lll_lock_elision (&(futex), &(adapt_count), private)
# define lll_unlock_elision(futex, private) \
  __lll_unlock_elision (&(futex), private)
# define lll_trylock_elision(futex, adapt_count) \
  __lll_trylock_elision(&(futex), &(adapt_count))
#endif

#endif	/* lowlevellock.h */
