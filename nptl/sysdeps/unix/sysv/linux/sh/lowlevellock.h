/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


/* Initializer for compatibility lock.  */
#define LLL_MUTEX_LOCK_INITIALIZER		(0)
#define LLL_MUTEX_LOCK_INITIALIZER_LOCKED	(1)
#define LLL_MUTEX_LOCK_INITIALIZER_WAITERS	(2)

extern int __lll_mutex_lock_wait (int val, int *__futex) attribute_hidden;
extern int __lll_mutex_timedlock_wait (int val, int *__futex,
				       const struct timespec *abstime)
     attribute_hidden;
extern int __lll_mutex_unlock_wake (int *__futex) attribute_hidden;


#define lll_mutex_trylock(futex) \
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.l %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) \
	: "r" (&(futex)), \
	  "r" (LLL_MUTEX_LOCK_INITIALIZER_LOCKED), \
	  "r" (LLL_MUTEX_LOCK_INITIALIZER) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define lll_mutex_cond_trylock(futex) \
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.l %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) \
	: "r" (&(futex)), \
	  "r" (LLL_MUTEX_LOCK_INITIALIZER_WAITERS), \
	  "r" (LLL_MUTEX_LOCK_INITIALIZER) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define lll_mutex_lock(futex) \
  (void) ({ int __result, val, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		nop\n\
		mov r15,r1\n\
		mov #-8,r15\n\
	     0: mov.l @%2,%0\n\
		tst %0,%0\n\
		bf 1f\n\
		mov.l %1,@%2\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (1), "r" (__futex) \
		: "r0", "r1", "t", "memory"); \
	    if (__result) \
	      __lll_mutex_lock_wait (__result, __futex); })

/* Special version of lll_mutex_lock which causes the unlock function to
   always wakeup waiters.  */
#define lll_mutex_cond_lock(futex) \
  (void) ({ int __result, val, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		nop\n\
		mov r15,r1\n\
		mov #-8,r15\n\
	     0: mov.l @%2,%0\n\
		tst %0,%0\n\
		bf 1f\n\
		mov.l %1,@%2\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (2), "r" (__futex) \
		: "r0", "r1", "t", "memory"); \
	    if (__result) \
	      __lll_mutex_lock_wait (__result, __futex); })

#define lll_mutex_timedlock(futex, timeout) \
  ({ int __result, val, *__futex = &(futex); \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%2,%0\n\
	tst %0,%0\n\
	bf 1f\n\
	mov.l %1,@%2\n\
     1: mov r1,r15"\
	: "=&r" (__result) : "r" (1), "r" (__futex) \
	: "r0", "r1", "t", "memory"); \
    if (__result) \
      __result = __lll_mutex_timedlock_wait (__result, __futex, timeout); \
    __result; })

#define lll_mutex_unlock(futex) \
  (void) ({ int __result, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.l @%1,%0\n\
		add #-1,%0\n\
		mov.l %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (__futex) \
		: "r0", "r1", "memory"); \
	    if (__result) \
	      __lll_mutex_unlock_wake (__futex); })

#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)


# ifdef NEED_SYSCALL_INST_PAD
#  define SYSCALL_WITH_INST_PAD "\
	trapa #0x14; or r0,r0; or r0,r0; or r0,r0; or r0,r0; or r0,r0"
# else
#  define SYSCALL_WITH_INST_PAD "\
	trapa #0x14"
# endif

#define lll_futex_wait(futex, val) \
  do {									      \
    int __ignore;							      \
    register unsigned long __r3 asm ("r3") = SYS_futex;			      \
    register unsigned long __r4 asm ("r4") = (unsigned long) (futex);	      \
    register unsigned long __r5 asm ("r5") = FUTEX_WAIT;		      \
    register unsigned long __r6 asm ("r6") = (unsigned long) (val);	      \
    register unsigned long __r7 asm ("r7") = 0;				      \
    __asm __volatile (SYSCALL_WITH_INST_PAD				      \
		      : "=z" (__ignore)					      \
		      : "r" (__r3), "r" (__r4), "r" (__r5),		      \
			"r" (__r6), "r" (__r7)				      \
		      : "memory", "t");					      \
  } while (0)


#define lll_futex_wake(futex, nr) \
  do {									      \
    int __ignore;							      \
    register unsigned long __r3 asm ("r3") = SYS_futex;			      \
    register unsigned long __r4 asm ("r4") = (unsigned long) (futex);	      \
    register unsigned long __r5 asm ("r5") = FUTEX_WAKE;		      \
    register unsigned long __r6 asm ("r6") = (unsigned long) (nr);	      \
    register unsigned long __r7 asm ("r7") = 0;				      \
    __asm __volatile (SYSCALL_WITH_INST_PAD				      \
		      : "=z" (__ignore)					      \
		      : "r" (__r3), "r" (__r4), "r" (__r5),		      \
			"r" (__r6), "r" (__r7)				      \
		      : "memory", "t");					      \
  } while (0)


extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;


/* The states of a lock are:
    0  -  untaken
    1  -  taken by one user
    2  -  taken by more users */

#define lll_trylock(futex) lll_mutex_trylock (futex)
#define lll_lock(futex) lll_mutex_lock (futex)
#define lll_unlock(futex) lll_mutex_unlock (futex)

#define lll_islocked(futex) \
  (futex != LLL_LOCK_INITIALIZER)


/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.  */

extern int __lll_wait_tid (int *tid) attribute_hidden;
#define lll_wait_tid(tid) \
  do {									      \
    __typeof (tid) *__tid = &(tid);					      \
    while (*__tid != 0)							      \
      lll_futex_wait (__tid, *__tid);					      \
  } while (0)

extern int __lll_timedwait_tid (int *tid, const struct timespec *abstime)
     attribute_hidden;
#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __result = 0;							      \
    if (tid != 0)							      \
      {									      \
	if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)	      \
	  __result = EINVAL;						      \
	else								      \
	  __result = __lll_timedwait_tid (&tid, abstime);		      \
      }									      \
    __result; })


/* Conditional variable handling.  */

extern void __lll_cond_wait (pthread_cond_t *cond) attribute_hidden;
extern int __lll_cond_timedwait (pthread_cond_t *cond,
				 const struct timespec *abstime)
     attribute_hidden;
extern void __lll_cond_wake (pthread_cond_t *cond) attribute_hidden;
extern void __lll_cond_broadcast (pthread_cond_t *cond) attribute_hidden;


#define lll_cond_wait(cond) \
  __lll_cond_wait (cond)
#define lll_cond_timedwait(cond, abstime) \
  __lll_cond_timedwait (cond, abstime)
#define lll_cond_wake(cond) \
  __lll_cond_wake (cond)
#define lll_cond_broadcast(cond) \
  __lll_cond_broadcast (cond)

#endif  /* lowlevellock.h */
