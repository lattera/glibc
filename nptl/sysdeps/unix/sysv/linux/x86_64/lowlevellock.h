/* Copyright (C) 2002, 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_PRIVATE_FLAG	128


/* Initializer for compatibility lock.  */
#define LLL_MUTEX_LOCK_INITIALIZER		(0)
#define LLL_MUTEX_LOCK_INITIALIZER_LOCKED	(1)
#define LLL_MUTEX_LOCK_INITIALIZER_WAITERS	(2)

/* Delay in spinlock loop.  */
#define BUSY_WAIT_NOP          asm ("rep; nop")


#define LLL_STUB_UNWIND_INFO_START \
	".section	.eh_frame,\"a\",@progbits\n" 		\
"7:\t"	".long	9f-8f	# Length of Common Information Entry\n" \
"8:\t"	".long	0x0	# CIE Identifier Tag\n\t" 		\
	".byte	0x1	# CIE Version\n\t" 			\
	".ascii \"zR\\0\"	# CIE Augmentation\n\t" 	\
	".uleb128 0x1	# CIE Code Alignment Factor\n\t" 	\
	".sleb128 -8	# CIE Data Alignment Factor\n\t" 	\
	".byte	0x10	# CIE RA Column\n\t" 			\
	".uleb128 0x1	# Augmentation size\n\t" 		\
	".byte	0x1b	# FDE Encoding (pcrel sdata4)\n\t" 	\
	".byte	0x12	# DW_CFA_def_cfa_sf\n\t" 		\
	".uleb128 0x7\n\t" 					\
	".sleb128 16\n\t" 					\
	".align 8\n" 						\
"9:\t"	".long	23f-10f	# FDE Length\n" 			\
"10:\t"	".long	10b-7b	# FDE CIE offset\n\t" 			\
	".long	1b-.	# FDE initial location\n\t" 		\
	".long	6b-1b	# FDE address range\n\t" 		\
	".uleb128 0x0	# Augmentation size\n\t" 		\
	".byte	0x16	# DW_CFA_val_expression\n\t" 		\
	".uleb128 0x10\n\t" 					\
	".uleb128 12f-11f\n" 					\
"11:\t"	".byte	0x80	# DW_OP_breg16\n\t" 			\
	".sleb128 4b-1b\n"
#define LLL_STUB_UNWIND_INFO_END \
	".byte	0x16	# DW_CFA_val_expression\n\t" 		\
	".uleb128 0x10\n\t" 					\
	".uleb128 14f-13f\n" 					\
"13:\t"	".byte	0x80	# DW_OP_breg16\n\t" 			\
	".sleb128 4b-2b\n" 					\
"14:\t"	".byte	0x40 + (3b-2b) # DW_CFA_advance_loc\n\t" 	\
	".byte	0x0e	# DW_CFA_def_cfa_offset\n\t" 		\
	".uleb128 0\n\t" 					\
	".byte	0x16	# DW_CFA_val_expression\n\t" 		\
	".uleb128 0x10\n\t" 					\
	".uleb128 16f-15f\n" 					\
"15:\t"	".byte	0x80	# DW_OP_breg16\n\t" 			\
	".sleb128 4b-3b\n" 					\
"16:\t"	".byte	0x40 + (4b-3b-1) # DW_CFA_advance_loc\n\t" 	\
	".byte	0x0e	# DW_CFA_def_cfa_offset\n\t" 		\
	".uleb128 128\n\t" 					\
	".byte	0x16	# DW_CFA_val_expression\n\t" 		\
	".uleb128 0x10\n\t" 					\
	".uleb128 20f-17f\n" 					\
"17:\t"	".byte	0x80	# DW_OP_breg16\n\t" 			\
	".sleb128 19f-18f\n\t" 					\
	".byte	0x0d	# DW_OP_const4s\n" 			\
"18:\t"	".4byte	4b-.\n\t" 					\
	".byte	0x1c	# DW_OP_minus\n\t" 			\
	".byte	0x0d	# DW_OP_const4s\n" 			\
"19:\t"	".4byte	24f-.\n\t" 					\
	".byte	0x22	# DW_OP_plus\n" 			\
"20:\t"	".byte	0x40 + (5b-4b+1) # DW_CFA_advance_loc\n\t" 	\
	".byte	0x13	# DW_CFA_def_cfa_offset_sf\n\t" 	\
	".sleb128 16\n\t" 					\
	".byte	0x16	# DW_CFA_val_expression\n\t" 		\
	".uleb128 0x10\n\t" 					\
	".uleb128 22f-21f\n" 					\
"21:\t"	".byte	0x80	# DW_OP_breg16\n\t" 			\
	".sleb128 4b-5b\n" 					\
"22:\t"	".align 8\n" 						\
"23:\t"	".previous\n"

/* Unwind info for
   1: leaq ..., %rdi
   2: subq $128, %rsp
   3: callq ...
   4: addq $128, %rsp
   5: jmp 24f
   6:
   snippet.  */
#define LLL_STUB_UNWIND_INFO_5 \
LLL_STUB_UNWIND_INFO_START					\
"12:\t"	".byte	0x40 + (2b-1b) # DW_CFA_advance_loc\n\t" 	\
LLL_STUB_UNWIND_INFO_END

/* Unwind info for
   1: leaq ..., %rdi
   0: movq ..., %rdx
   2: subq $128, %rsp
   3: callq ...
   4: addq $128, %rsp
   5: jmp 24f
   6:
   snippet.  */
#define LLL_STUB_UNWIND_INFO_6 \
LLL_STUB_UNWIND_INFO_START					\
"12:\t"	".byte	0x40 + (0b-1b) # DW_CFA_advance_loc\n\t" 	\
	".byte	0x16	# DW_CFA_val_expression\n\t" 		\
	".uleb128 0x10\n\t" 					\
	".uleb128 26f-25f\n" 					\
"25:\t"	".byte	0x80	# DW_OP_breg16\n\t" 			\
	".sleb128 4b-0b\n" 					\
"26:\t"	".byte	0x40 + (2b-0b) # DW_CFA_advance_loc\n\t" 	\
LLL_STUB_UNWIND_INFO_END


#define lll_futex_wait(futex, val) \
  ({									      \
    int __status;							      \
    register __typeof (val) _val __asm ("edx") = (val);			      \
    __asm __volatile ("xorq %%r10, %%r10\n\t"				      \
		      "syscall"						      \
		      : "=a" (__status)					      \
		      : "0" (SYS_futex), "D" (futex), "S" (FUTEX_WAIT),	      \
			"d" (_val)					      \
		      : "memory", "cc", "r10", "r11", "cx");		      \
    __status;								      \
  })


#define lll_futex_timed_wait(futex, val, timeout)			      \
  ({									      \
    register const struct timespec *__to __asm ("r10") = timeout;	      \
    int __status;							      \
    register __typeof (val) _val __asm ("edx") = (val);			      \
    __asm __volatile ("syscall"						      \
		      : "=a" (__status)					      \
		      : "0" (SYS_futex), "D" (futex), "S" (FUTEX_WAIT),	      \
		        "d" (_val), "r" (__to)				      \
		      : "memory", "cc", "r11", "cx");			      \
    __status;								      \
  })


#define lll_futex_wake(futex, nr) \
  do {									      \
    int __ignore;							      \
    register __typeof (nr) _nr __asm ("edx") = (nr);			      \
    __asm __volatile ("syscall"						      \
		      : "=a" (__ignore)					      \
		      : "0" (SYS_futex), "D" (futex), "S" (FUTEX_WAKE),	      \
			"d" (_nr)					      \
		      : "memory", "cc", "r10", "r11", "cx");		      \
  } while (0)


/* Does not preserve %eax and %ecx.  */
extern int __lll_mutex_lock_wait (int *__futex, int __val) attribute_hidden;
/* Does not preserver %eax, %ecx, and %edx.  */
extern int __lll_mutex_timedlock_wait (int *__futex, int __val,
				       const struct timespec *__abstime)
     attribute_hidden;
/* Preserves all registers but %eax.  */
extern int __lll_mutex_unlock_wait (int *__futex) attribute_hidden;


/* NB: in the lll_mutex_trylock macro we simply return the value in %eax
   after the cmpxchg instruction.  In case the operation succeded this
   value is zero.  In case the operation failed, the cmpxchg instruction
   has loaded the current value of the memory work which is guaranteed
   to be nonzero.  */
#define lll_mutex_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_MUTEX_LOCK_INITIALIZER_LOCKED), "m" (futex),\
			 "0" (LLL_MUTEX_LOCK_INITIALIZER)		      \
		       : "memory");					      \
     ret; })


#define lll_robust_mutex_trylock(futex, id)				      \
  ({ int ret;								      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (id), "m" (futex),				      \
			 "0" (LLL_MUTEX_LOCK_INITIALIZER)		      \
		       : "memory");					      \
     ret; })


#define lll_mutex_cond_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_MUTEX_LOCK_INITIALIZER_WAITERS),	      \
			 "m" (futex), "0" (LLL_MUTEX_LOCK_INITIALIZER)	      \
		       : "memory");					      \
     ret; })


#define lll_mutex_lock(futex) \
  (void) ({ int ignore1, ignore2, ignore3;				      \
	    __asm __volatile (LOCK_INSTR "cmpxchgl %0, %2\n\t"		      \
			      "jnz 1f\n\t"				      \
			      ".subsection 1\n\t"			      \
			      ".type  _L_mutex_lock_%=, @function\n"	      \
			      "_L_mutex_lock_%=:\n"			      \
			      "1:\tleaq %2, %%rdi\n"			      \
			      "2:\tsubq $128, %%rsp\n"			      \
			      "3:\tcallq __lll_mutex_lock_wait\n"	      \
			      "4:\taddq $128, %%rsp\n"			      \
			      "5:\tjmp 24f\n"				      \
			      "6:\t.size _L_mutex_lock_%=, 6b-1b\n\t"	      \
			      ".previous\n"				      \
			      LLL_STUB_UNWIND_INFO_5			      \
			      "24:"					      \
			      : "=S" (ignore1), "=&D" (ignore2), "=m" (futex),\
				"=a" (ignore3)				      \
			      : "0" (1), "m" (futex), "3" (0)		      \
			      : "cx", "r11", "cc", "memory"); })


#define lll_robust_mutex_lock(futex, id) \
  ({ int result, ignore1, ignore2;					      \
    __asm __volatile (LOCK_INSTR "cmpxchgl %0, %2\n\t"			      \
		      "jnz 1f\n\t"					      \
		      ".subsection 1\n\t"				      \
		      ".type  _L_robust_mutex_lock_%=, @function\n"	      \
		      "_L_robust_mutex_lock_%=:\n"			      \
		      "1:\tleaq %2, %%rdi\n"				      \
		      "2:\tsubq $128, %%rsp\n"				      \
		      "3:\tcallq __lll_robust_mutex_lock_wait\n"	      \
		      "4:\taddq $128, %%rsp\n"				      \
		      "5:\tjmp 24f\n"					      \
		      "6:\t.size _L_robust_mutex_lock_%=, 6b-1b\n\t"	      \
		      ".previous\n"					      \
		      LLL_STUB_UNWIND_INFO_5				      \
		      "24:"						      \
		      : "=S" (ignore1), "=&D" (ignore2), "=m" (futex),	      \
			"=a" (result)					      \
		      : "0" (id), "m" (futex), "3" (0)			      \
		      : "cx", "r11", "cc", "memory");			      \
    result; })


#define lll_mutex_cond_lock(futex) \
  (void) ({ int ignore1, ignore2, ignore3;				      \
	    __asm __volatile (LOCK_INSTR "cmpxchgl %0, %2\n\t"		      \
			      "jnz 1f\n\t"				      \
			      ".subsection 1\n\t"			      \
			      ".type  _L_mutex_cond_lock_%=, @function\n"     \
			      "_L_mutex_cond_lock_%=:\n"		      \
			      "1:\tleaq %2, %%rdi\n"			      \
			      "2:\tsubq $128, %%rsp\n"			      \
			      "3:\tcallq __lll_mutex_lock_wait\n"	      \
			      "4:\taddq $128, %%rsp\n"			      \
			      "5:\tjmp 24f\n"				      \
			      "6:\t.size _L_mutex_cond_lock_%=, 6b-1b\n\t"    \
			      ".previous\n"				      \
			      LLL_STUB_UNWIND_INFO_5			      \
			      "24:"					      \
			      : "=S" (ignore1), "=&D" (ignore2), "=m" (futex),\
				"=a" (ignore3)				      \
			      : "0" (2), "m" (futex), "3" (0)		      \
			      : "cx", "r11", "cc", "memory"); })


#define lll_robust_mutex_cond_lock(futex, id) \
  ({ int result, ignore1, ignore2;					      \
    __asm __volatile (LOCK_INSTR "cmpxchgl %0, %2\n\t"			      \
		      "jnz 1f\n\t"					      \
		      ".subsection 1\n\t"				      \
		      ".type  _L_robust_mutex_cond_lock_%=, @function\n"      \
		      "_L_robust_mutex_cond_lock_%=:\n"			      \
		      "1:\tleaq %2, %%rdi\n"				      \
		      "2:\tsubq $128, %%rsp\n"				      \
		      "3:\tcallq __lll_robust_mutex_lock_wait\n"	      \
		      "4:\taddq $128, %%rsp\n"				      \
		      "5:\tjmp 24f\n"					      \
		      "6:\t.size _L_robust_mutex_cond_lock_%=, 6b-1b\n\t"     \
		      ".previous\n"					      \
		      LLL_STUB_UNWIND_INFO_5				      \
		      "24:"						      \
		      : "=S" (ignore1), "=&D" (ignore2), "=m" (futex),	      \
			"=a" (result)					      \
		      : "0" (id | FUTEX_WAITERS), "m" (futex), "3" (0)	      \
		      : "cx", "r11", "cc", "memory");			      \
    result; })


#define lll_mutex_timedlock(futex, timeout) \
  ({ int result, ignore1, ignore2, ignore3;				      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %4\n\t"			      \
		       "jnz 1f\n\t"					      \
		       ".subsection 1\n\t"				      \
		       ".type  _L_mutex_timedlock_%=, @function\n"	      \
		       "_L_mutex_timedlock_%=:\n"			      \
		       "1:\tleaq %4, %%rdi\n"				      \
		       "0:\tmovq %8, %%rdx\n"				      \
		       "2:\tsubq $128, %%rsp\n"				      \
		       "3:\tcallq __lll_mutex_timedlock_wait\n"		      \
		       "4:\taddq $128, %%rsp\n"				      \
		       "5:\tjmp 24f\n"					      \
		       "6:\t.size _L_mutex_timedlock_%=, 6b-1b\n\t"	      \
		       ".previous\n"					      \
		       LLL_STUB_UNWIND_INFO_6				      \
		       "24:"						      \
		       : "=a" (result), "=&D" (ignore1), "=S" (ignore2),      \
			 "=&d" (ignore3), "=m" (futex)			      \
		       : "0" (0), "2" (1), "m" (futex), "m" (timeout)	      \
		       : "memory", "cx", "cc", "r10", "r11");		      \
     result; })


#define lll_robust_mutex_timedlock(futex, timeout, id) \
  ({ int result, ignore1, ignore2, ignore3;				      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %4\n\t"			      \
		       "jnz 1f\n\t"					      \
		       ".subsection 1\n\t"				      \
		       ".type  _L_robust_mutex_timedlock_%=, @function\n"     \
		       "_L_robust_mutex_timedlock_%=:\n"		      \
		       "1:\tleaq %4, %%rdi\n"				      \
		       "0:\tmovq %8, %%rdx\n"				      \
		       "2:\tsubq $128, %%rsp\n"				      \
		       "3:\tcallq __lll_robust_mutex_timedlock_wait\n"	      \
		       "4:\taddq $128, %%rsp\n"				      \
		       "5:\tjmp 24f\n"					      \
		       "6:\t.size _L_robust_mutex_timedlock_%=, 6b-1b\n\t"    \
		       ".previous\n"					      \
		       LLL_STUB_UNWIND_INFO_6				      \
		       "24:"						      \
		       : "=a" (result), "=&D" (ignore1), "=S" (ignore2),      \
			 "=&d" (ignore3), "=m" (futex)			      \
		       : "0" (0), "2" (id), "m" (futex), "m" (timeout)	      \
		       : "memory", "cx", "cc", "r10", "r11");		      \
     result; })


#define lll_mutex_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile (LOCK_INSTR "decl %0\n\t"			      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n\t"			      \
			      ".type  _L_mutex_unlock_%=, @function\n"	      \
			      "_L_mutex_unlock_%=:\n"			      \
			      "1:\tleaq %0, %%rdi\n"			      \
			      "2:\tsubq $128, %%rsp\n"			      \
			      "3:\tcallq __lll_mutex_unlock_wake\n"	      \
			      "4:\taddq $128, %%rsp\n"			      \
			      "5:\tjmp 24f\n"				      \
			      "6:\t.size _L_mutex_unlock_%=, 6b-1b\n\t"	      \
			      ".previous\n"				      \
			      LLL_STUB_UNWIND_INFO_5			      \
			      "24:"					      \
			      : "=m" (futex), "=&D" (ignore)		      \
			      : "m" (futex)				      \
			      : "ax", "cx", "r11", "cc", "memory"); })


#define lll_robust_mutex_unlock(futex) \
  (void) ({ int ignore;							      \
	    __asm __volatile (LOCK_INSTR "andl %2, %0\n\t"		      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n\t"			      \
			      ".type  _L_robust_mutex_unlock_%=, @function\n" \
			      "_L_robust_mutex_unlock_%=:\n"		      \
			      "1:\tleaq %0, %%rdi\n"			      \
			      "2:\tsubq $128, %%rsp\n"			      \
			      "3:\tcallq __lll_mutex_unlock_wake\n"	      \
			      "4:\taddq $128, %%rsp\n"			      \
			      "5:\tjmp 24f\n"				      \
			      "6:\t.size _L_robust_mutex_unlock_%=, 6b-1b\n\t"\
			      ".previous\n"				      \
			      LLL_STUB_UNWIND_INFO_5			      \
			      "24:"					      \
			      : "=m" (futex), "=&D" (ignore)		      \
			      : "i" (FUTEX_WAITERS), "m" (futex)	      \
			      : "ax", "cx", "r11", "cc", "memory"); })


#define lll_robust_mutex_dead(futex) \
  (void) ({ int ignore;		     \
	    __asm __volatile (LOCK_INSTR "orl %3, (%2)\n\t"		      \
			      "syscall"					      \
			      : "=m" (futex), "=a" (ignore)		      \
			      : "D" (&(futex)), "i" (FUTEX_OWNER_DIED),	      \
				"S" (FUTEX_WAKE), "1" (__NR_futex),	      \
				"d" (1)					      \
			      : "cx", "r11", "cc", "memory"); })


/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(ftx, nr_wake, nr_move, mutex, val) \
  ({ int __res;								      \
     register int __nr_move __asm ("r10") = nr_move;			      \
     register void *__mutex __asm ("r8") = mutex;			      \
     register int __val __asm ("r9") = val;				      \
     __asm __volatile ("syscall"					      \
		       : "=a" (__res)					      \
		       : "0" (__NR_futex), "D" ((void *) ftx),		      \
		         "S" (FUTEX_CMP_REQUEUE), "d" (nr_wake),	      \
		         "r" (__nr_move), "r" (__mutex), "r" (__val)	      \
		       : "cx", "r11", "cc", "memory");			      \
     __res < 0; })


#define lll_mutex_islocked(futex) \
  (futex != LLL_MUTEX_LOCK_INITIALIZER)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)


extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;


/* The states of a lock are:
    0  -  untaken
    1  -  taken by one user
    2  -  taken by more users */


#if defined NOT_IN_libc || defined UP
# define lll_trylock(futex) lll_mutex_trylock (futex)
# define lll_lock(futex) lll_mutex_lock (futex)
# define lll_unlock(futex) lll_mutex_unlock (futex)
#else
/* Special versions of the macros for use in libc itself.  They avoid
   the lock prefix when the thread library is not used.

   The code sequence to avoid unnecessary lock prefixes is what the AMD
   guys suggested.  If you do not like it, bring it up with AMD.

   XXX In future we might even want to avoid it on UP machines.  */

# define lll_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile ("cmpl $0, __libc_multiple_threads(%%rip)\n\t"	      \
		       "je 0f\n\t"					      \
		       "lock; cmpxchgl %2, %1\n\t"			      \
		       "jmp 1f\n"					      \
		       "0:\tcmpxchgl %2, %1\n\t"			      \
		       "1:setne %0"					      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_MUTEX_LOCK_INITIALIZER_LOCKED), "m" (futex),\
			 "0" (LLL_MUTEX_LOCK_INITIALIZER)		      \
		       : "memory");					      \
     ret; })


# define lll_lock(futex) \
  (void) ({ int ignore1, ignore2, ignore3;				      \
	    __asm __volatile ("cmpl $0, __libc_multiple_threads(%%rip)\n\t"   \
			      "je 0f\n\t"				      \
			      "lock; cmpxchgl %0, %2\n\t"		      \
			      "jnz 1f\n\t"				      \
		  	      "jmp 24f\n"				      \
			      "0:\tcmpxchgl %0, %2\n\t"			      \
			      "jnz 1f\n\t"				      \
			      ".subsection 1\n\t"			      \
			      ".type  _L_lock_%=, @function\n"		      \
			      "_L_lock_%=:\n"				      \
			      "1:\tleaq %2, %%rdi\n"			      \
			      "2:\tsubq $128, %%rsp\n"			      \
			      "3:\tcallq __lll_mutex_lock_wait\n"	      \
			      "4:\taddq $128, %%rsp\n"			      \
			      "5:\tjmp 24f\n"				      \
			      "6:\t.size _L_lock_%=, 6b-1b\n\t"		      \
			      ".previous\n"				      \
			      LLL_STUB_UNWIND_INFO_5			      \
			      "24:"					      \
			      : "=S" (ignore1), "=&D" (ignore2), "=m" (futex),\
				"=a" (ignore3)				      \
			      : "0" (1), "m" (futex), "3" (0)		      \
			      : "cx", "r11", "cc", "memory"); })


# define lll_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile ("cmpl $0, __libc_multiple_threads(%%rip)\n\t"   \
			      "je 0f\n\t"				      \
			      "lock; decl %0\n\t"			      \
			      "jne 1f\n\t"				      \
		  	      "jmp 24f\n"				      \
			      "0:\tdecl %0\n\t"				      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n\t"			      \
			      ".type  _L_unlock_%=, @function\n"	      \
			      "_L_unlock_%=:\n"				      \
			      "1:\tleaq %0, %%rdi\n"			      \
			      "2:\tsubq $128, %%rsp\n"			      \
			      "3:\tcallq __lll_mutex_unlock_wake\n"	      \
			      "4:\taddq $128, %%rsp\n"			      \
			      "5:\tjmp 24f\n"				      \
			      "6:\t.size _L_unlock_%=, 6b-1b\n\t"	      \
			      ".previous\n"				      \
			      LLL_STUB_UNWIND_INFO_5			      \
			      "24:"					      \
			      : "=m" (futex), "=&D" (ignore)		      \
			      : "m" (futex)				      \
			      : "ax", "cx", "r11", "cc", "memory"); })
#endif


#define lll_islocked(futex) \
  (futex != LLL_MUTEX_LOCK_INITIALIZER)


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
