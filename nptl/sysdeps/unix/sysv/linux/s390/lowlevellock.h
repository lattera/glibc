/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#define SYS_futex		238
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

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


#define lll_compare_and_swap(futex, oldval, newval, operation) \
  do {									      \
    __typeof (futex) __futex = (futex);					      \
    __asm __volatile ("	  l   %1,%0\n"					      \
		      "0: " operation "\n"				      \
		      "	  cs  %1,%2,%0\n"				      \
		      "	  jl  0b\n"					      \
		      "1:"						      \
		      : "=Q" (*__futex), "=&d" (oldval), "=&d" (newval)	      \
		      : "m" (*__futex) : "cc" );			      \
  } while (0)


static inline int
__attribute__ ((always_inline))
__lll_mutex_trylock (int *futex)
{
    unsigned int old;

    __asm __volatile ("cs %0,%3,%1"
		       : "=d" (old), "=Q" (*futex)
		       : "0" (0), "d" (1), "m" (*futex) : "cc" );
    return old != 0;
}
#define lll_mutex_trylock(futex) __lll_mutex_trylock (&(futex))


extern void ___lll_mutex_lock (int *, int) attribute_hidden;


static inline void
__attribute__ ((always_inline))
__lll_mutex_lock (int *futex)
{
  int oldval;
  int newval;

  lll_compare_and_swap (futex, oldval, newval, "lr %2,%1; ahi %2,1");
  if (oldval > 0)
    ___lll_mutex_lock (futex, newval);
}
#define lll_mutex_lock(futex) __lll_mutex_lock (&(futex))


extern int ___lll_mutex_timedlock (int *, const struct timespec *, int)
  attribute_hidden;


static inline int
__attribute__ ((always_inline))
__lll_mutex_timedlock (int *futex, struct timespec *abstime)
{
  int oldval;
  int newval;
  int result = 0;

  lll_compare_and_swap (futex, oldval, newval, "lr %2,%1; ahi %2,1");
  if (oldval > 0)
    result = ___lll_mutex_timedlock (futex, abstime, newval);

  return result;
}
#define lll_mutex_timedlock(futex, abstime) \
  __lll_mutex_timedlock (&(futex), abstime)


static inline void
__attribute__ ((always_inline))
__lll_mutex_unlock (int *futex)
{
  int oldval;
  int newval;

  lll_compare_and_swap (futex, oldval, newval, "slr %2,%2");
  if (oldval > 1)
    lll_futex_wake (futex, 1);
}
#define lll_mutex_unlock(futex) __lll_mutex_unlock(&(futex))

#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(1)
#define LLL_LOCK_INITIALIZER_LOCKED	(0)


extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;

/* The states of a lock are:
    1  -  untaken
    0  -  taken by one user
   <0  -  taken by more users */


static inline int
__attribute__ ((always_inline))
__lll_trylock (int *futex)
{
  unsigned int old;

  __asm __volatile ("cs %0,%3,%1"
		    : "=d" (old), "=Q" (*futex)
		    : "0" (1), "d" (0), "m" (*futex) : "cc" );
  return old != 1;
}
#define lll_trylock(futex) __lll_trylock (&(futex))


extern void ___lll_lock (int *, int) attribute_hidden;

static inline void
__attribute__ ((always_inline))
__lll_lock (int *futex)
{
  int oldval;
  int newval;

  lll_compare_and_swap (futex, oldval, newval, "lr %2,%1; ahi %2,-1");
  if (newval < 0)
    ___lll_lock (futex, newval);
}
#define lll_lock(futex) __lll_lock (&(futex))


static inline void
__attribute__ ((always_inline))
__lll_unlock (int *futex)
{
  int oldval;
  int newval;

  lll_compare_and_swap (futex, oldval, newval, "lhi %2,1");
  if (oldval < 0)
    lll_futex_wake (futex, 1);
}
#define lll_unlock(futex) __lll_unlock(&(futex))


#define lll_islocked(futex) \
  (futex != 1)


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


extern int ___lll_timedwait_tid (int *, const struct timespec *)
     attribute_hidden;
static inline int
__attribute__ ((always_inline))
__lll_timedwait_tid (int *ptid, const struct timespec *abstime)
{
  if (*ptid == 0)
    return 0;

  return ___lll_timedwait_tid (ptid, abstime);
}
#define lll_timedwait_tid(tid, abstime) __lll_timedwait_tid (&(tid), abstime)


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
