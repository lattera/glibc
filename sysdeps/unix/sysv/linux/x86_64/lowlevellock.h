/* Copyright (C) 2002-2014 Free Software Foundation, Inc.
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
# include <tcb-offsets.h>

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

#define SYS_futex		__NR_futex
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_WAIT_REQUEUE_PI	11
#define FUTEX_CMP_REQUEUE_PI	12
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

#define FUTEX_OP_CLEAR_WAKE_IF_GT_ONE	((4 << 24) | 1)

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE	0
#define LLL_SHARED	FUTEX_PRIVATE_FLAG

#ifndef __ASSEMBLER__

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
   : ({ unsigned int __fl = ((private) ^ FUTEX_PRIVATE_FLAG);		      \
	asm ("andl %%fs:%P1, %0" : "+r" (__fl)				      \
	     : "i" (offsetof (struct pthread, header.private_futex)));	      \
	__fl | (fl); }))
# endif
#endif

/* Initializer for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)
#define LLL_LOCK_INITIALIZER_WAITERS	(2)

/* Delay in spinlock loop.  */
#define BUSY_WAIT_NOP	  asm ("rep; nop")

#define lll_futex_wait(futex, val, private) \
  lll_futex_timed_wait(futex, val, NULL, private)


#define lll_futex_timed_wait(futex, val, timeout, private) \
  ({									      \
    register const struct timespec *__to __asm ("r10") = timeout;	      \
    int __status;							      \
    register __typeof (val) _val __asm ("edx") = (val);			      \
    __asm __volatile ("syscall"						      \
		      : "=a" (__status)					      \
		      : "0" (SYS_futex), "D" (futex),			      \
			"S" (__lll_private_flag (FUTEX_WAIT, private)),	      \
			"d" (_val), "r" (__to)				      \
		      : "memory", "cc", "r11", "cx");			      \
    __status;								      \
  })


#define lll_futex_wake(futex, nr, private) \
  ({									      \
    int __status;							      \
    register __typeof (nr) _nr __asm ("edx") = (nr);			      \
    LIBC_PROBE (lll_futex_wake, 3, futex, nr, private);                       \
    __asm __volatile ("syscall"						      \
		      : "=a" (__status)					      \
		      : "0" (SYS_futex), "D" (futex),			      \
			"S" (__lll_private_flag (FUTEX_WAKE, private)),	      \
			"d" (_nr)					      \
		      : "memory", "cc", "r10", "r11", "cx");		      \
    __status;								      \
  })


/* NB: in the lll_trylock macro we simply return the value in %eax
   after the cmpxchg instruction.  In case the operation succeded this
   value is zero.  In case the operation failed, the cmpxchg instruction
   has loaded the current value of the memory work which is guaranteed
   to be nonzero.  */
#if !IS_IN (libc) || defined UP
# define __lll_trylock_asm LOCK_INSTR "cmpxchgl %2, %1"
#else
# define __lll_trylock_asm "cmpl $0, __libc_multiple_threads(%%rip)\n\t"      \
			   "je 0f\n\t"					      \
			   "lock; cmpxchgl %2, %1\n\t"			      \
			   "jmp 1f\n\t"					      \
			   "0:\tcmpxchgl %2, %1\n\t"			      \
			   "1:"
#endif

#define lll_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (__lll_trylock_asm				      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_LOCK_INITIALIZER_LOCKED), "m" (futex),      \
			 "0" (LLL_LOCK_INITIALIZER)			      \
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
# define __lll_lock_asm_start LOCK_INSTR "cmpxchgl %4, %2\n\t"		      \
			      "jz 24f\n\t"
#else
# define __lll_lock_asm_start "cmpl $0, __libc_multiple_threads(%%rip)\n\t"   \
			      "je 0f\n\t"				      \
			      "lock; cmpxchgl %4, %2\n\t"		      \
			      "jnz 1f\n\t"				      \
			      "jmp 24f\n"				      \
			      "0:\tcmpxchgl %4, %2\n\t"			      \
			      "jz 24f\n\t"
#endif

#define lll_lock(futex, private) \
  (void)								      \
    ({ int ignore1, ignore2, ignore3;					      \
       if (__builtin_constant_p (private) && (private) == LLL_PRIVATE)	      \
	 __asm __volatile (__lll_lock_asm_start				      \
			   "1:\tlea %2, %%" RDI_LP "\n"			      \
			   "2:\tsub $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset 128\n"		      \
			   "3:\tcallq __lll_lock_wait_private\n"	      \
			   "4:\tadd $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset -128\n"		      \
			   "24:"					      \
			   : "=S" (ignore1), "=&D" (ignore2), "=m" (futex),   \
			     "=a" (ignore3)				      \
			   : "0" (1), "m" (futex), "3" (0)		      \
			   : "cx", "r11", "cc", "memory");		      \
       else								      \
	 __asm __volatile (__lll_lock_asm_start				      \
			   "1:\tlea %2, %%" RDI_LP "\n"			      \
			   "2:\tsub $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset 128\n"		      \
			   "3:\tcallq __lll_lock_wait\n"		      \
			   "4:\tadd $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset -128\n"		      \
			   "24:"					      \
			   : "=S" (ignore1), "=D" (ignore2), "=m" (futex),    \
			     "=a" (ignore3)				      \
			   : "1" (1), "m" (futex), "3" (0), "0" (private)     \
			   : "cx", "r11", "cc", "memory");		      \
    })									      \

#define lll_robust_lock(futex, id, private) \
  ({ int result, ignore1, ignore2;					      \
    __asm __volatile (LOCK_INSTR "cmpxchgl %4, %2\n\t"			      \
		      "jz 24f\n"					      \
		      "1:\tlea %2, %%" RDI_LP "\n"			      \
		      "2:\tsub $128, %%" RSP_LP "\n"			      \
		      ".cfi_adjust_cfa_offset 128\n"			      \
		      "3:\tcallq __lll_robust_lock_wait\n"		      \
		      "4:\tadd $128, %%" RSP_LP "\n"			      \
		      ".cfi_adjust_cfa_offset -128\n"			      \
		      "24:"						      \
		      : "=S" (ignore1), "=D" (ignore2), "=m" (futex),	      \
			"=a" (result)					      \
		      : "1" (id), "m" (futex), "3" (0), "0" (private)	      \
		      : "cx", "r11", "cc", "memory");			      \
    result; })

#define lll_cond_lock(futex, private) \
  (void)								      \
    ({ int ignore1, ignore2, ignore3;					      \
       __asm __volatile (LOCK_INSTR "cmpxchgl %4, %2\n\t"		      \
			 "jz 24f\n"					      \
			 "1:\tlea %2, %%" RDI_LP "\n"			      \
			 "2:\tsub $128, %%" RSP_LP "\n"			      \
			 ".cfi_adjust_cfa_offset 128\n"			      \
			 "3:\tcallq __lll_lock_wait\n"			      \
			 "4:\tadd $128, %%" RSP_LP "\n"			      \
			 ".cfi_adjust_cfa_offset -128\n"		      \
			 "24:"						      \
			 : "=S" (ignore1), "=D" (ignore2), "=m" (futex),      \
			   "=a" (ignore3)				      \
			 : "1" (2), "m" (futex), "3" (0), "0" (private)	      \
			 : "cx", "r11", "cc", "memory");		      \
    })

#define lll_robust_cond_lock(futex, id, private) \
  ({ int result, ignore1, ignore2;					      \
    __asm __volatile (LOCK_INSTR "cmpxchgl %4, %2\n\t"			      \
		      "jz 24f\n"					      \
		      "1:\tlea %2, %%" RDI_LP "\n"			      \
		      "2:\tsub $128, %%" RSP_LP "\n"			      \
		      ".cfi_adjust_cfa_offset 128\n"			      \
		      "3:\tcallq __lll_robust_lock_wait\n"		      \
		      "4:\tadd $128, %%" RSP_LP "\n"			      \
		      ".cfi_adjust_cfa_offset -128\n"			      \
		      "24:"						      \
		      : "=S" (ignore1), "=D" (ignore2), "=m" (futex),	      \
			"=a" (result)					      \
		      : "1" (id | FUTEX_WAITERS), "m" (futex), "3" (0),	      \
			"0" (private)					      \
		      : "cx", "r11", "cc", "memory");			      \
    result; })

#define lll_timedlock(futex, timeout, private) \
  ({ int result, ignore1, ignore2, ignore3;				      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %1, %4\n\t"			      \
		       "jz 24f\n"					      \
		       "1:\tlea %4, %%" RDI_LP "\n"			      \
		       "0:\tmov %8, %%" RDX_LP "\n"			      \
		       "2:\tsub $128, %%" RSP_LP "\n"			      \
		       ".cfi_adjust_cfa_offset 128\n"			      \
		       "3:\tcallq __lll_timedlock_wait\n"		      \
		       "4:\tadd $128, %%" RSP_LP "\n"			      \
		       ".cfi_adjust_cfa_offset -128\n"			      \
		       "24:"						      \
		       : "=a" (result), "=D" (ignore1), "=S" (ignore2),	      \
			 "=&d" (ignore3), "=m" (futex)			      \
		       : "0" (0), "1" (1), "m" (futex), "m" (timeout),	      \
			 "2" (private)					      \
		       : "memory", "cx", "cc", "r10", "r11");		      \
     result; })

extern int __lll_timedlock_elision (int *futex, short *adapt_count,
					 const struct timespec *timeout,
					 int private) attribute_hidden;

#define lll_timedlock_elision(futex, adapt_count, timeout, private)	\
  __lll_timedlock_elision(&(futex), &(adapt_count), timeout, private)

#define lll_robust_timedlock(futex, timeout, id, private) \
  ({ int result, ignore1, ignore2, ignore3;				      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %1, %4\n\t"			      \
		       "jz 24f\n\t"					      \
		       "1:\tlea %4, %%" RDI_LP "\n"			      \
		       "0:\tmov %8, %%" RDX_LP "\n"			      \
		       "2:\tsub $128, %%" RSP_LP "\n"			      \
		       ".cfi_adjust_cfa_offset 128\n"			      \
		       "3:\tcallq __lll_robust_timedlock_wait\n"	      \
		       "4:\tadd $128, %%" RSP_LP "\n"			      \
		       ".cfi_adjust_cfa_offset -128\n"			      \
		       "24:"						      \
		       : "=a" (result), "=D" (ignore1), "=S" (ignore2),       \
			 "=&d" (ignore3), "=m" (futex)			      \
		       : "0" (0), "1" (id), "m" (futex), "m" (timeout),	      \
			 "2" (private)					      \
		       : "memory", "cx", "cc", "r10", "r11");		      \
     result; })

#if !IS_IN (libc) || defined UP
# define __lll_unlock_asm_start LOCK_INSTR "decl %0\n\t"		      \
				"je 24f\n\t"
#else
# define __lll_unlock_asm_start "cmpl $0, __libc_multiple_threads(%%rip)\n\t" \
				"je 0f\n\t"				      \
				"lock; decl %0\n\t"			      \
				"jne 1f\n\t"				      \
				"jmp 24f\n\t"				      \
				"0:\tdecl %0\n\t"			      \
				"je 24f\n\t"
#endif

#define lll_unlock(futex, private) \
  (void)								      \
    ({ int ignore;							      \
       if (__builtin_constant_p (private) && (private) == LLL_PRIVATE)	      \
	 __asm __volatile (__lll_unlock_asm_start			      \
			   "1:\tlea %0, %%" RDI_LP "\n"			      \
			   "2:\tsub $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset 128\n"		      \
			   "3:\tcallq __lll_unlock_wake_private\n"	      \
			   "4:\tadd $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset -128\n"		      \
			   "24:"					      \
			   : "=m" (futex), "=&D" (ignore)		      \
			   : "m" (futex)				      \
			   : "ax", "cx", "r11", "cc", "memory");	      \
       else								      \
	 __asm __volatile (__lll_unlock_asm_start			      \
			   "1:\tlea %0, %%" RDI_LP "\n"			      \
			   "2:\tsub $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset 128\n"		      \
			   "3:\tcallq __lll_unlock_wake\n"		      \
			   "4:\tadd $128, %%" RSP_LP "\n"		      \
			   ".cfi_adjust_cfa_offset -128\n"		      \
			   "24:"					      \
			   : "=m" (futex), "=&D" (ignore)		      \
			   : "m" (futex), "S" (private)			      \
			   : "ax", "cx", "r11", "cc", "memory");	      \
    })

#define lll_robust_unlock(futex, private) \
  do									      \
    {									      \
      int ignore;							      \
      __asm __volatile (LOCK_INSTR "andl %2, %0\n\t"			      \
			"je 24f\n\t"					      \
			"1:\tlea %0, %%" RDI_LP "\n"			      \
			"2:\tsub $128, %%" RSP_LP "\n"			      \
			".cfi_adjust_cfa_offset 128\n"			      \
			"3:\tcallq __lll_unlock_wake\n"			      \
			"4:\tadd $128, %%" RSP_LP "\n"			      \
			".cfi_adjust_cfa_offset -128\n"			      \
			"24:"						      \
			: "=m" (futex), "=&D" (ignore)			      \
			: "i" (FUTEX_WAITERS), "m" (futex),		      \
			  "S" (private)					      \
			: "ax", "cx", "r11", "cc", "memory");		      \
    }									      \
  while (0)

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(ftx, nr_wake, nr_move, mutex, val, private) \
  ({ int __res;								      \
     register int __nr_move __asm ("r10") = nr_move;			      \
     register void *__mutex __asm ("r8") = mutex;			      \
     register int __val __asm ("r9") = val;				      \
     __asm __volatile ("syscall"					      \
		       : "=a" (__res)					      \
		       : "0" (__NR_futex), "D" ((void *) ftx),		      \
			 "S" (__lll_private_flag (FUTEX_CMP_REQUEUE,	      \
						  private)), "d" (nr_wake),   \
			 "r" (__nr_move), "r" (__mutex), "r" (__val)	      \
		       : "cx", "r11", "cc", "memory");			      \
     __res < 0; })

#define lll_islocked(futex) \
  (futex != LLL_LOCK_INITIALIZER)


/* The kernel notifies a process which uses CLONE_CHILD_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.

   The macro parameter must not have any side effect.  */
#define lll_wait_tid(tid) \
  do {									      \
    int __ignore;							      \
    register __typeof (tid) _tid asm ("edx") = (tid);			      \
    if (_tid != 0)							      \
      __asm __volatile ("xorq %%r10, %%r10\n\t"				      \
			"1:\tmovq %2, %%rax\n\t"			      \
			"syscall\n\t"					      \
			"cmpl $0, (%%rdi)\n\t"				      \
			"jne 1b"					      \
			: "=&a" (__ignore)				      \
			: "S" (FUTEX_WAIT), "i" (SYS_futex), "D" (&tid),      \
			  "d" (_tid)					      \
			: "memory", "cc", "r10", "r11", "cx");		      \
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

extern int __lll_lock_elision (int *futex, short *adapt_count, int private)
  attribute_hidden;

extern int __lll_unlock_elision (int *lock, int private)
  attribute_hidden;

extern int __lll_trylock_elision (int *lock, short *adapt_count)
  attribute_hidden;

#define lll_lock_elision(futex, adapt_count, private) \
  __lll_lock_elision (&(futex), &(adapt_count), private)
#define lll_unlock_elision(futex, private) \
  __lll_unlock_elision (&(futex), private)
#define lll_trylock_elision(futex, adapt_count) \
  __lll_trylock_elision (&(futex), &(adapt_count))

#endif  /* !__ASSEMBLER__ */

#endif	/* lowlevellock.h */
