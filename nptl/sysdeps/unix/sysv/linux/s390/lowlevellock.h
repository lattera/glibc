/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <atomic.h>

#define SYS_futex		238
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4

/* Initializer for compatibility lock.	*/
#define LLL_MUTEX_LOCK_INITIALIZER (0)

#define lll_futex_wait(futex, val) \
  ({									      \
     register unsigned long int __r2 asm ("2") = (unsigned long int) (futex); \
     register unsigned long int __r3 asm ("3") = FUTEX_WAIT;		      \
     register unsigned long int __r4 asm ("4") = (unsigned long int) (val);   \
     register unsigned long int __r5 asm ("5") = 0ul;			      \
     register unsigned long __result asm ("2");				      \
									      \
    __asm __volatile ("svc %b1"						      \
		      : "=d" (__result)					      \
		      : "i" (SYS_futex), "0" (__r2), "d" (__r3),	      \
			"d" (__r4), "d" (__r5)				      \
		      : "cc", "memory" );				      \
    __result;								      \
  })


#define lll_futex_timed_wait(futex, val, timespec) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3") = FUTEX_WAIT;		      \
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


#define lll_futex_wake(futex, nr) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3") = FUTEX_WAKE;		      \
    register unsigned long int __r4 asm ("4") = (unsigned long int) (nr);     \
    register unsigned long int __result asm ("2");			      \
									      \
    __asm __volatile ("svc %b1"						      \
		      : "=d" (__result)					      \
		      : "i" (SYS_futex), "0" (__r2), "d" (__r3), "d" (__r4)   \
		      : "cc", "memory" );				      \
    __result;								      \
  })


/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(futex, nr_wake, nr_move, mutex, val) \
  ({									      \
    register unsigned long int __r2 asm ("2") = (unsigned long int) (futex);  \
    register unsigned long int __r3 asm ("3") = FUTEX_CMP_REQUEUE;	      \
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
__lll_mutex_trylock (int *futex)
{
    unsigned int old;

    __asm __volatile ("cs %0,%3,%1"
		       : "=d" (old), "=Q" (*futex)
		       : "0" (0), "d" (1), "m" (*futex) : "cc", "memory" );
    return old != 0;
}
#define lll_mutex_trylock(futex) __lll_mutex_trylock (&(futex))


static inline int
__attribute__ ((always_inline))
__lll_mutex_cond_trylock (int *futex)
{
    unsigned int old;

    __asm __volatile ("cs %0,%3,%1"
		       : "=d" (old), "=Q" (*futex)
		       : "0" (0), "d" (2), "m" (*futex) : "cc", "memory" );
    return old != 0;
}
#define lll_mutex_cond_trylock(futex) __lll_mutex_cond_trylock (&(futex))


extern void __lll_lock_wait (int *futex) attribute_hidden;

static inline void
__attribute__ ((always_inline))
__lll_mutex_lock (int *futex)
{
  if (atomic_compare_and_exchange_bool_acq (futex, 1, 0) != 0)
    __lll_lock_wait (futex);
}
#define lll_mutex_lock(futex) __lll_mutex_lock (&(futex))

static inline void
__attribute__ ((always_inline))
__lll_mutex_cond_lock (int *futex)
{
  if (atomic_compare_and_exchange_bool_acq (futex, 2, 0) != 0)
    __lll_lock_wait (futex);
}
#define lll_mutex_cond_lock(futex) __lll_mutex_cond_lock (&(futex))

extern int __lll_timedlock_wait
  (int *futex, const struct timespec *) attribute_hidden;

static inline int
__attribute__ ((always_inline))
__lll_mutex_timedlock (int *futex, const struct timespec *abstime)
{
  int result = 0;
  if (atomic_compare_and_exchange_bool_acq (futex, 1, 0) != 0)
    result = __lll_timedlock_wait (futex, abstime);
  return result;
}
#define lll_mutex_timedlock(futex, abstime) \
  __lll_mutex_timedlock (&(futex), abstime)


static inline void
__attribute__ ((always_inline))
__lll_mutex_unlock (int *futex)
{
  int oldval;
  int newval = 0;

  lll_compare_and_swap (futex, oldval, newval, "slr %2,%2");
  if (oldval > 1)
    lll_futex_wake (futex, 1);
}
#define lll_mutex_unlock(futex) \
  __lll_mutex_unlock(&(futex))


static inline void
__attribute__ ((always_inline))
__lll_mutex_unlock_force (int *futex)
{
  *futex = 0;
  lll_futex_wake (futex, 1);
}
#define lll_mutex_unlock_force(futex) \
  __lll_mutex_unlock_force(&(futex))

#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  We can use the lll_mutex_*.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

#define lll_trylock(futex)      lll_mutex_trylock (futex)
#define lll_lock(futex)         lll_mutex_lock (futex)
#define lll_unlock(futex)       lll_mutex_unlock (futex)
#define lll_islocked(futex)     lll_mutex_islocked (futex)

extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;

/* The states of a lock are:
    1  -  untaken
    0  -  taken by one user
   <0  -  taken by more users */


/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.	*/
static inline void
__attribute__ ((always_inline))
__lll_wait_tid (int *ptid)
{
  int tid;

  while ((tid = *ptid) != 0)
    lll_futex_wait (ptid, tid);
}
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

/* Conditional variable handling.  */

extern void __lll_cond_wait (pthread_cond_t *cond)
     attribute_hidden;
extern int __lll_cond_timedwait (pthread_cond_t *cond,
				 const struct timespec *abstime)
     attribute_hidden;
extern void __lll_cond_wake (pthread_cond_t *cond)
     attribute_hidden;
extern void __lll_cond_broadcast (pthread_cond_t *cond)
     attribute_hidden;

#define lll_cond_wait(cond) \
  __lll_cond_wait (cond)
#define lll_cond_timedwait(cond, abstime) \
  __lll_cond_timedwait (cond, abstime)
#define lll_cond_wake(cond) \
  __lll_cond_wake (cond)
#define lll_cond_broadcast(cond) \
  __lll_cond_broadcast (cond)

#endif	/* lowlevellock.h */
