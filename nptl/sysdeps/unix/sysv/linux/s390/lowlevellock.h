/* Copyright (C) 2003, 2004, 2006-2008, 2009 Free Software Foundation, Inc.
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
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE	0
#define LLL_SHARED	FUTEX_PRIVATE_FLAG


#if !defined NOT_IN_libc || defined IS_IN_rtld
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

#define lll_futex_timed_wait(futex, val, timespec, private) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3")				      \
      = __lll_private_flag (FUTEX_WAIT, private);			      \
    register unsigned long int __r4 asm ("4") = (unsigned long int) (val);    \
    register unsigned long int __r5 asm ("5") = (unsigned long int)(timespec);\
    register unsigned long int __result asm ("2");			      \
									      \
    __asm __volatile ("svc %b1"						      \
		      : "=d" (__result)					      \
		      : "i" (SYS_futex), "0" (__r2), "d" (__r3),	      \
			"d" (__r4), "d" (__r5)				      \
		      : "cc", "memory" );				      \
    __result;								      \
  })


#define lll_futex_wake(futex, nr, private) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3")				      \
      = __lll_private_flag (FUTEX_WAKE, private);			      \
    register unsigned long int __r4 asm ("4") = (unsigned long int) (nr);     \
    register unsigned long int __result asm ("2");			      \
									      \
    __asm __volatile ("svc %b1"						      \
		      : "=d" (__result)					      \
		      : "i" (SYS_futex), "0" (__r2), "d" (__r3), "d" (__r4)   \
		      : "cc", "memory" );				      \
    __result;								      \
  })


#define lll_robust_dead(futexv, private) \
  do									      \
    {									      \
      int *__futexp = &(futexv);					      \
									      \
      atomic_or (__futexp, FUTEX_OWNER_DIED);				      \
      lll_futex_wake (__futexp, 1, private);				      \
    }									      \
  while (0)


/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(futex, nr_wake, nr_move, mutex, val, private) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3")				      \
      = __lll_private_flag (FUTEX_CMP_REQUEUE, private);		      \
    register unsigned long int __r4 asm ("4") = (long int) (nr_wake);	      \
    register unsigned long int __r5 asm ("5") = (long int) (nr_move);	      \
    register unsigned long int __r6 asm ("6") = (unsigned long int) (mutex);  \
    register unsigned long int __r7 asm ("7") = (int) (val);		      \
    register unsigned long __result asm ("2");				      \
									      \
    __asm __volatile ("svc %b1"						      \
		      : "=d" (__result)					      \
		      : "i" (SYS_futex), "0" (__r2), "d" (__r3),	      \
			"d" (__r4), "d" (__r5), "d" (__r6), "d" (__r7)	      \
		      : "cc", "memory" );				      \
    __result > -4096UL;							      \
  })


/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_wake_unlock(futex, nr_wake, nr_wake2, futex2, private) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3")				      \
      = __lll_private_flag (FUTEX_WAKE_OP, private);			      \
    register unsigned long int __r4 asm ("4") = (long int) (nr_wake);	      \
    register unsigned long int __r5 asm ("5") = (long int) (nr_wake2);	      \
    register unsigned long int __r6 asm ("6") = (unsigned long int) (futex2); \
    register unsigned long int __r7 asm ("7")				      \
      = (int) FUTEX_OP_CLEAR_WAKE_IF_GT_ONE;				      \
    register unsigned long __result asm ("2");				      \
									      \
    __asm __volatile ("svc %b1"						      \
		      : "=d" (__result)					      \
		      : "i" (SYS_futex), "0" (__r2), "d" (__r3),	      \
			"d" (__r4), "d" (__r5), "d" (__r6), "d" (__r7)	      \
		      : "cc", "memory" );				      \
    __result > -4096UL;							      \
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


static inline int
__attribute__ ((always_inline))
__lll_robust_trylock (int *futex, int id)
{
    unsigned int old;

    __asm __volatile ("cs %0,%3,%1"
		       : "=d" (old), "=Q" (*futex)
		       : "0" (0), "d" (id), "m" (*futex) : "cc", "memory" );
    return old != 0;
}
#define lll_robust_trylock(futex, id) \
  __lll_robust_trylock (&(futex), id)


extern void __lll_lock_wait_private (int *futex) attribute_hidden;
extern void __lll_lock_wait (int *futex, int private) attribute_hidden;
extern int __lll_robust_lock_wait (int *futex, int private) attribute_hidden;

static inline void
__attribute__ ((always_inline))
__lll_lock (int *futex, int private)
{
  if (__builtin_expect (atomic_compare_and_exchange_bool_acq (futex, 1, 0), 0))
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
  if (__builtin_expect (atomic_compare_and_exchange_bool_acq (futex, 2, 0), 0))
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
  if (__builtin_expect (atomic_compare_and_exchange_bool_acq (futex, 1, 0), 0))
    result = __lll_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_timedlock(futex, abstime, private) \
  __lll_timedlock (&(futex), abstime, private)

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
       if (__builtin_expect (__oldval > 1, 0))				      \
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
       if (__builtin_expect (__oldval & FUTEX_WAITERS, 0))		      \
	 lll_futex_wake (__futexp, 1, private);				      \
    })
#define lll_robust_unlock(futex, private) \
  __lll_robust_unlock(&(futex), private)

#define lll_islocked(futex) \
  (futex != 0)


/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

/* The kernel notifies a process with uses CLONE_CLEARTID via futex
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

#endif	/* lowlevellock.h */
