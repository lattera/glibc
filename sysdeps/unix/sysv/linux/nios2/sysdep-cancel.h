/* Assembler macros with cancellation support, Nios II version.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .type __##syscall_name##_nocancel, @function;				      \
  .globl __##syscall_name##_nocancel;					      \
  __##syscall_name##_nocancel:						      \
    cfi_startproc;                                                            \
    DO_CALL (syscall_name, args);                                             \
    bne r7, zero, SYSCALL_ERROR_LABEL;                                        \
    ret;                                                                      \
    cfi_endproc;                                                              \
  .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel;	      \
  ENTRY (name)								      \
    SINGLE_THREAD_P(r2);						      \
    bne r2, zero, pseudo_cancel;					      \
    DO_CALL (syscall_name, args);					      \
    bne r7, zero, SYSCALL_ERROR_LABEL;                                        \
    ret;								      \
  pseudo_cancel:							      \
    SAVESTK_##args;                 /* save syscall args and adjust stack */  \
    SAVEREG(ra, 0);                     /* save return address */             \
    SAVEREG(r22, 4);                    /* save GOT pointer */                \
    nextpc r22;                                                               \
1:  movhi r2, %hiadj(_gp_got - 1b);					      \
    addi r2, r2, %lo(_gp_got - 1b);					      \
    add r22, r22, r2;                                                         \
    CENABLE;                                                                  \
    callr r3;                                                                 \
    stw r2, 8(sp);                      /* save mask */                       \
    LOADARGS_##args;                                                          \
    movi r2, SYS_ify(syscall_name);                                           \
    trap;                                                                     \
    stw r2, 12(sp);                     /* save syscall result */             \
    stw r7, 16(sp);                     /* save syscall error flag */         \
    ldw r4, 8(sp);                      /* pass mask as argument 1 */         \
    CDISABLE;                                                                 \
    callr r3;                                                                 \
    ldw r7, 16(sp);                     /* restore syscall error flag */      \
    ldw r2, 12(sp);                     /* restore syscall result */          \
    ldw ra, 0(sp);                      /* restore return address */          \
    ldw r22, 4(sp);                     /* restore GOT pointer */             \
    RESTORESTK_##args;                                                        \
    bne r7, zero, SYSCALL_ERROR_LABEL;


# undef PSEUDO_END
# define PSEUDO_END(sym) \
  SYSCALL_ERROR_HANDLER \
  END (sym)

#define SAVEREG(REG, LOC) stw REG, LOC(sp); cfi_rel_offset (REG, LOC)
#define SAVESTK(X) subi sp, sp, X; cfi_adjust_cfa_offset(X)
#define SAVESTK_0 SAVESTK(20)
#define SAVEARG_1 SAVEREG(r4, 20)
#define SAVESTK_1 SAVESTK(24); SAVEARG_1
#define SAVEARG_2 SAVEREG(r5, 24); SAVEARG_1
#define SAVESTK_2 SAVESTK(28); SAVEARG_2
#define SAVEARG_3 SAVEREG(r6, 28); SAVEARG_2
#define SAVESTK_3 SAVESTK(32); SAVEARG_3
#define SAVEARG_4 SAVEREG(r7, 32); SAVEARG_3
#define SAVESTK_4 SAVESTK(36); SAVEARG_4
#define SAVESTK_5 SAVESTK_4
#define SAVESTK_6 SAVESTK_5

#define LOADARGS_0
#define LOADARGS_1 ldw r4, 20(sp)
#define LOADARGS_2 LOADARGS_1; ldw r5, 24(sp)
#define LOADARGS_3 LOADARGS_2; ldw r6, 28(sp)
#define LOADARGS_4 LOADARGS_3; ldw r7, 32(sp)
#define LOADARGS_5 LOADARGS_4; ldw r8, 36(sp)
#define LOADARGS_6 LOADARGS_5; ldw r9, 40(sp)

#define RESTORESTK(X) addi sp, sp, X; cfi_adjust_cfa_offset(-X)
#define RESTORESTK_0 RESTORESTK(20)
#define RESTORESTK_1 RESTORESTK(24)
#define RESTORESTK_2 RESTORESTK(28)
#define RESTORESTK_3 RESTORESTK(32)
#define RESTORESTK_4 RESTORESTK(36)
#define RESTORESTK_5 RESTORESTK(36)
#define RESTORESTK_6 RESTORESTK(36)

# if IS_IN (libpthread)
#  define CENABLE	ldw r3, %call(__pthread_enable_asynccancel)(r22)
#  define CDISABLE	ldw r3, %call(__pthread_disable_asynccancel)(r22)
# elif IS_IN (librt)
#  define CENABLE	ldw r3, %call(__librt_enable_asynccancel)(r22)
#  define CDISABLE	ldw r3, %call(__librt_disable_asynccancel)(r22)
# elif IS_IN (libc)
#  define CENABLE	ldw r3, %call(__libc_enable_asynccancel)(r22)
#  define CDISABLE	ldw r3, %call(__libc_disable_asynccancel)(r22)
# else
#  error Unsupported library
# endif

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P						\
	__builtin_expect (THREAD_GETMEM (THREAD_SELF,			\
					 header.multiple_threads)	\
			  == 0, 1)
# else
#  define SINGLE_THREAD_P(reg)						\
	ldw reg, MULTIPLE_THREADS_OFFSET(r23)
#endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
