/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>

#ifndef LOCK_INSTR
# ifdef UP
#  define LOCK_INSTR	/* nothing */
# else
#  define LOCK_INSTR "lock;"
# endif
#endif

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


/* Initializer for compatibility lock.  */
#define LLL_MUTEX_LOCK_INITIALIZER (0)


/* Does not preserve %eax and %ecx.  */
extern int __lll_mutex_lock_wait (int val, int *__futex) attribute_hidden;
/* Does not preserver %eax, %ecx, and %edx.  */
extern int __lll_mutex_timedlock_wait (int val, int *__futex,
				       const struct timespec *abstime)
     attribute_hidden;
/* Preserves all registers but %eax.  */
extern int __lll_mutex_unlock_wait (int *__futex) attribute_hidden;


#define lll_mutex_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (1), "1" (futex), "0" (0)			      \
		       : "memory");					      \
     ret; })


#define lll_mutex_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile (LOCK_INSTR "xaddl %0, %2\n\t"		      \
			      "testl %0, %0\n\t"			      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleaq %2, %%rsi\n\t"			      \
			      "call __lll_mutex_lock_wait\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=D" (ignore1), "=&S" (ignore2), "=m" (futex) \
			      : "0" (1), "2" (futex)			      \
			      : "memory"); })


#define lll_mutex_timedlock(futex, timeout) \
  ({ int result, ignore1, ignore2, ignore3;				      \
     __asm __volatile (LOCK_INSTR "xaddl %0, %4\n\t"			      \
		       "testl %0, %0\n\t"				      \
		       "jne 1f\n\t"					      \
		       ".subsection 1\n"				      \
		       "1:\tmovl %0, %%edi\n\t"				      \
		       "leaq %4, %%rsi\n\t"				      \
		       "movq %7, %%rdx\n\t"				      \
		       "call __lll_mutex_timedlock_wait\n\t"		      \
		       "jmp 2f\n\t"					      \
		       ".previous\n"					      \
		       "2:"						      \
		       : "=a" (result), "=&D" (ignore1), "=&S" (ignore2),     \
			 "=&d" (ignore2), "=m" (futex)			      \
		       : "0" (1), "4" (futex), "m" (timeout)		      \
		       : "memory");					      \
     result; })


#define lll_mutex_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile (LOCK_INSTR "decl %0\n\t"			      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleaq %0, %%rdi\n\t"			      \
			      "call __lll_mutex_unlock_wake\n\t"	      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=m" (futex), "=&D" (ignore)		      \
			      : "0" (futex)				      \
			      : "memory"); })


#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(1)
#define LLL_LOCK_INITIALIZER_LOCKED	(0)


extern int __lll_lock_wait (int *__futex, int val) attribute_hidden;
extern int __lll_unlock_wake (int *__futex) attribute_hidden;
extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;


/* The states of a lock are:
    1  -  untaken
    0  -  taken by one user
   <0  -  taken by more users */


#if defined NOT_IN_libc || defined UP
# define lll_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (0), "1" (futex), "0" (1)			      \
		       : "memory");					      \
     ret; })


# define lll_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile (LOCK_INSTR "xaddl %0, %2\n\t"		      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleaq %2, %%rsi\n\t"			      \
			      "call __lll_lock_wait\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=D" (ignore1), "=&S" (ignore2), "=m" (futex) \
			      : "0" (-1), "2" (futex)			      \
			      : "memory"); })


# define lll_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile (LOCK_INSTR "addl $1,%0\n\t"		      \
			      "jng 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleaq %0, %%rdi\n\t"			      \
			      "call __lll_unlock_wake\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=m" (futex), "=&D" (ignore)		      \
			      : "0" (futex)				      \
			      : "memory"); })
#else
/* Special versions of the macros for use in libc itself.  They avoid
   the lock prefix when the thread library is not used.

   XXX In future we might even want to avoid it on UP machines.  */

# define lll_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile ("cmpl $0, __libc_multiple_threads(%%rip)\n\t"	      \
		       "je 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgl %2, %1; setne %0"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (0), "1" (futex), "0" (1)			      \
		       : "memory");					      \
     ret; })


# define lll_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile ("cmpl $0, __libc_multiple_threads(%%rip)\n\t"   \
			      "je 0f\n\t"				      \
			      "lock\n"					      \
			      "0:\txaddl %0, %2\n\t"			      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleaq %2, %%rsi\n\t"			      \
			      "call __lll_lock_wait\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=D" (ignore1), "=&S" (ignore2), "=m" (futex) \
			      : "0" (-1), "2" (futex)			      \
			      : "memory"); })


# define lll_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile ("cmpl $0, __libc_multiple_threads(%%rip)\n\t"   \
			      "je 0f\n\t"				      \
			      "lock\n"					      \
			      "0:\taddl $1,%0\n\t"			      \
			      "jng 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleaq %0, %%rdi\n\t"			      \
			      "call __lll_unlock_wake\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=m" (futex), "=&D" (ignore)		      \
			      : "0" (futex)				      \
			      : "memory"); })
#endif


#define lll_islocked(futex) \
  (futex != 0)


/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.

   The macro parameter must not have any side effect.  */
#define lll_wait_tid(tid) \
  do {									      \
    int __ignore;							      \
    register __typeof (tid) _tid asm ("edx") = (tid);			      \
    if (_tid != 0)							      \
      __asm __volatile ("1:\tmovq %1, %%rax\n\t"			      \
			"syscall\n\t"					      \
			"cmpl $0, (%%rdi)\n\t"				      \
			"jne 1b"					      \
			: "=&a" (__ignore)				      \
			: "i" (SYS_futex), "D" (&tid), "r10" (0),	      \
			  "S" (FUTEX_WAIT), "d" (_tid)			      \
			: "memory", "cc", "r11", "cx");			      \
  } while (0)

extern int __lll_timedwait_tid (int *tid, const struct timespec *abstime)
     attribute_hidden;
#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __result = 0;							      \
    if (tid != 0)							      \
      {									      \
	if (abstime == NULL || abstime->tv_nsec >= 1000000000)		      \
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


#endif	/* lowlevellock.h */
