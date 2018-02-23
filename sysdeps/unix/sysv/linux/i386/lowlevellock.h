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

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <stap-probe.h>

#ifndef __ASSEMBLER__
# include <time.h>
# include <sys/param.h>
# include <bits/pthreadtypes.h>
# include <kernel-features.h>
/* <tcb-offsets.h> is generated from tcb-offsets.sym to define offsets
   and sizes of types in <tls.h> as well as <pthread.h> which includes
   <lowlevellock.h> via nptl/descr.h.  Don't include <tcb-offsets.h>
   when generating <tcb-offsets.h> to avoid circular dependency which
   may lead to build hang on a many-core machine.  */
# ifndef GEN_AS_CONST_HEADERS
#  include <tcb-offsets.h>
# endif

# ifndef LOCK_INSTR
#  ifdef UP
#   define LOCK_INSTR	/* nothing */
#  else
#   define LOCK_INSTR "lock;"
#  endif
# endif
#else
# ifndef LOCK
#  ifdef UP
#   define LOCK
#  else
#   define LOCK lock
#  endif
# endif
#endif

#include <lowlevellock-futex.h>

/* XXX Remove when no assembler code uses futexes anymore.  */
#define SYS_futex		__NR_futex

#ifndef __ASSEMBLER__

/* Initializer for compatibility lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)
#define LLL_LOCK_INITIALIZER_WAITERS	(2)


/* NB: in the lll_trylock macro we simply return the value in %eax
   after the cmpxchg instruction.  In case the operation succeded this
   value is zero.  In case the operation failed, the cmpxchg instruction
   has loaded the current value of the memory work which is guaranteed
   to be nonzero.  */
#if !IS_IN (libc) || defined UP
# define __lll_trylock_asm LOCK_INSTR "cmpxchgl %2, %1"
#else
# define __lll_trylock_asm "cmpl $0, %%gs:%P5\n\t" \
			   "je 0f\n\t"					      \
			   "lock\n"					      \
			   "0:\tcmpxchgl %2, %1"
#endif

#define lll_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (__lll_trylock_asm				      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_LOCK_INITIALIZER_LOCKED), "m" (futex),      \
			 "0" (LLL_LOCK_INITIALIZER),			      \
			 "i" (MULTIPLE_THREADS_OFFSET)			      \
		       : "memory");					      \
     ret; })


#define lll_cond_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_LOCK_INITIALIZER_WAITERS),		      \
			 "m" (futex), "0" (LLL_LOCK_INITIALIZER)	      \
		       : "memory");					      \
     ret; })

#if !IS_IN (libc) || defined UP
# define __lll_lock_asm_start LOCK_INSTR "cmpxchgl %1, %2\n\t"
#else
# define __lll_lock_asm_start "cmpl $0, %%gs:%P6\n\t"			      \
			      "je 0f\n\t"				      \
			      "lock\n"					      \
			      "0:\tcmpxchgl %1, %2\n\t"
#endif

#define lll_lock(futex, private) \
  (void)								      \
    ({ int ignore1, ignore2;						      \
       if (__builtin_constant_p (private) && (private) == LLL_PRIVATE)	      \
	 __asm __volatile (__lll_lock_asm_start				      \
			   "jz 18f\n\t"				      \
			   "1:\tleal %2, %%ecx\n"			      \
			   "2:\tcall __lll_lock_wait_private\n" 	      \
			   "18:"					      \
			   : "=a" (ignore1), "=c" (ignore2), "=m" (futex)     \
			   : "0" (0), "1" (1), "m" (futex),		      \
			     "i" (MULTIPLE_THREADS_OFFSET)		      \
			   : "memory");					      \
       else								      \
	 {								      \
	   int ignore3;							      \
	   __asm __volatile (__lll_lock_asm_start			      \
			     "jz 18f\n\t"			 	      \
			     "1:\tleal %2, %%edx\n"			      \
			     "0:\tmovl %8, %%ecx\n"			      \
			     "2:\tcall __lll_lock_wait\n"		      \
			     "18:"					      \
			     : "=a" (ignore1), "=c" (ignore2),		      \
			       "=m" (futex), "=&d" (ignore3) 		      \
			     : "1" (1), "m" (futex),			      \
			       "i" (MULTIPLE_THREADS_OFFSET), "0" (0),	      \
			       "g" ((int) (private))			      \
			     : "memory");				      \
	 }								      \
    })


/* Special version of lll_lock which causes the unlock function to
   always wakeup waiters.  */
#define lll_cond_lock(futex, private) \
  (void)								      \
    ({ int ignore1, ignore2, ignore3;					      \
       __asm __volatile (LOCK_INSTR "cmpxchgl %1, %2\n\t"		      \
			 "jz 18f\n\t"					      \
			 "1:\tleal %2, %%edx\n"				      \
			 "0:\tmovl %7, %%ecx\n"				      \
			 "2:\tcall __lll_lock_wait\n"			      \
			 "18:"						      \
			 : "=a" (ignore1), "=c" (ignore2), "=m" (futex),      \
			   "=&d" (ignore3)				      \
			 : "0" (0), "1" (2), "m" (futex), "g" ((int) (private))\
			 : "memory");					      \
    })


#define lll_timedlock(futex, timeout, private) \
  ({ int result, ignore1, ignore2, ignore3;				      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %1, %3\n\t"			      \
		       "jz 18f\n\t"					      \
		       "1:\tleal %3, %%ecx\n"				      \
		       "0:\tmovl %8, %%edx\n"				      \
		       "2:\tcall __lll_timedlock_wait\n"		      \
		       "18:"						      \
		       : "=a" (result), "=c" (ignore1), "=&d" (ignore2),      \
			 "=m" (futex), "=S" (ignore3)			      \
		       : "0" (0), "1" (1), "m" (futex), "m" (timeout),	      \
			 "4" ((int) (private))				      \
		       : "memory");					      \
     result; })

extern int __lll_timedlock_elision (int *futex, short *adapt_count,
					 const struct timespec *timeout,
					 int private) attribute_hidden;

#define lll_timedlock_elision(futex, adapt_count, timeout, private)	\
  __lll_timedlock_elision(&(futex), &(adapt_count), timeout, private)

#if !IS_IN (libc) || defined UP
# define __lll_unlock_asm LOCK_INSTR "subl $1, %0\n\t"
#else
# define __lll_unlock_asm "cmpl $0, %%gs:%P3\n\t"			      \
			  "je 0f\n\t"					      \
			  "lock\n"					      \
			  "0:\tsubl $1,%0\n\t"
#endif

#define lll_unlock(futex, private) \
  (void)								      \
    ({ int ignore;							      \
       if (__builtin_constant_p (private) && (private) == LLL_PRIVATE)	      \
	 __asm __volatile (__lll_unlock_asm				      \
			   "je 18f\n\t"					      \
			   "1:\tleal %0, %%eax\n"			      \
			   "2:\tcall __lll_unlock_wake_private\n"	      \
			   "18:"					      \
			   : "=m" (futex), "=&a" (ignore)		      \
			   : "m" (futex), "i" (MULTIPLE_THREADS_OFFSET)	      \
			   : "memory");					      \
       else								      \
	 {								      \
	   int ignore2;							      \
	   __asm __volatile (__lll_unlock_asm				      \
			     "je 18f\n\t"				      \
			     "1:\tleal %0, %%eax\n"			      \
			     "0:\tmovl %5, %%ecx\n"			      \
			     "2:\tcall __lll_unlock_wake\n"		      \
			     "18:"					      \
			     : "=m" (futex), "=&a" (ignore), "=&c" (ignore2)  \
			     : "i" (MULTIPLE_THREADS_OFFSET), "m" (futex),    \
			       "g" ((int) (private))			      \
			     : "memory");				      \
	 }								      \
    })


#define lll_islocked(futex) \
  (futex != LLL_LOCK_INITIALIZER)

/* The kernel notifies a process which uses CLONE_CHILD_CLEARTID via futex
   wake-up when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero by the kernel
   afterwards.  The kernel up to version 3.16.3 does not use the private futex
   operations for futex wake-up when the clone terminates.  */
#define lll_wait_tid(tid) \
  do {					\
    __typeof (tid) __tid;		\
    while ((__tid = (tid)) != 0)	\
      lll_futex_wait (&(tid), __tid, LLL_SHARED);\
  } while (0)

extern int __lll_timedwait_tid (int *tid, const struct timespec *abstime)
     __attribute__ ((regparm (2))) attribute_hidden;

/* As lll_wait_tid, but with a timeout.  If the timeout occurs then return
   ETIMEDOUT.  If ABSTIME is invalid, return EINVAL.
   XXX Note that this differs from the generic version in that we do the
   error checking here and not in __lll_timedwait_tid.  */
#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __result = 0;							      \
    if ((tid) != 0)							      \
      __result = __lll_timedwait_tid (&(tid), (abstime));		      \
    __result; })


extern int __lll_lock_elision (int *futex, short *adapt_count, int private)
  attribute_hidden;

extern int __lll_unlock_elision(int *lock, int private)
  attribute_hidden;

extern int __lll_trylock_elision(int *lock, short *adapt_count)
  attribute_hidden;

#define lll_lock_elision(futex, adapt_count, private) \
  __lll_lock_elision (&(futex), &(adapt_count), private)
#define lll_unlock_elision(futex, adapt_count, private) \
  __lll_unlock_elision (&(futex), private)
#define lll_trylock_elision(futex, adapt_count) \
  __lll_trylock_elision(&(futex), &(adapt_count))

#endif  /* !__ASSEMBLER__ */

#endif	/* lowlevellock.h */
