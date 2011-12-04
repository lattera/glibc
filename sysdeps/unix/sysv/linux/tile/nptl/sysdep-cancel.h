/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

/* Allow hacking in some extra code if desired. */
#ifndef PSEUDO_EXTRA
#define PSEUDO_EXTRA
#endif

#undef PSEUDO
#define PSEUDO(name, syscall_name, args)				      \
  ENTRY(__##syscall_name##_nocancel);					      \
    PSEUDO_EXTRA							      \
    moveli TREG_SYSCALL_NR_NAME, SYS_ify(syscall_name);			      \
    swint1;								      \
    BNEZ r1, 0f;							      \
    jrp lr;								      \
  END(__##syscall_name##_nocancel);					      \
  ENTRY (name)								      \
    SINGLE_THREAD_P(r11);						      \
    BEQZ r11, L(pseudo_cancel);						      \
    PSEUDO_EXTRA							      \
    moveli TREG_SYSCALL_NR_NAME, SYS_ify(syscall_name);			      \
    swint1;								      \
    BNEZ r1, 0f;							      \
    jrp lr;								      \
  L(pseudo_cancel):							      \
    {									      \
     move r11, sp;							      \
     ST sp, lr;								      \
     ADDI_PTR sp, sp, -STKSPACE;					      \
    };									      \
    cfi_offset (lr, 0);							      \
    cfi_def_cfa_offset (STKSPACE);					      \
    {									      \
     ADDI_PTR r12, sp, REGSIZE;						      \
     ADDI_PTR r13, sp, 2 * REGSIZE;	/* set up for PUSHARGS_0 */	      \
    };									      \
    ST r12, r11;							      \
    PUSHARGS_##args			/* save syscall args */	      	      \
    CENABLE;								      \
    ADDI_PTR r12, sp, 10 * REGSIZE;					      \
    {									      \
     ST r12, r0;			/* save mask */			      \
     ADDI_PTR r13, sp, 2 * REGSIZE;	/* set up for POPARGS_0 */	      \
    };									      \
    POPARGS_##args			/* restore syscall args */	      \
    PSEUDO_EXTRA							      \
    moveli TREG_SYSCALL_NR_NAME, SYS_ify(syscall_name);			      \
    swint1;								      \
    ADDI_PTR r12, sp, 12 * REGSIZE;					      \
    {									      \
     ST r12, r1;			/* save syscall result */             \
     ADDI_PTR r12, sp, 11 * REGSIZE;					      \
    };									      \
    {									      \
     ST r12, r0;			                                      \
     ADDI_PTR r13, sp, 10 * REGSIZE;					      \
    };									      \
    LD r0, r13;				/* pass mask as arg1 */		      \
    CDISABLE;								      \
    {									      \
     ADDI_PTR lr, sp, STKSPACE;						      \
     ADDI_PTR r0, sp, 11 * REGSIZE;					      \
    };									      \
    {									      \
     LD r0, r0;								      \
     ADDI_PTR r1, sp, 12 * REGSIZE;					      \
    };									      \
    LD r1, r1;								      \
    {									      \
     LD lr, lr;								      \
     ADDI_PTR sp, sp, STKSPACE;						      \
    };									      \
    cfi_def_cfa_offset (0);						      \
    BNEZ r1, 0f

# define PUSHARGS_0 /* nothing to do */
# define PUSHARGS_1 PUSHARGS_0 { ADDI_PTR r14, sp, 3 * REGSIZE; ST r13, r0 };
# define PUSHARGS_2 PUSHARGS_1 { ADDI_PTR r13, sp, 4 * REGSIZE; ST r14, r1 };
# define PUSHARGS_3 PUSHARGS_2 { ADDI_PTR r14, sp, 5 * REGSIZE; ST r13, r2 };
# define PUSHARGS_4 PUSHARGS_3 { ADDI_PTR r13, sp, 6 * REGSIZE; ST r14, r3 };
# define PUSHARGS_5 PUSHARGS_4 { ADDI_PTR r14, sp, 7 * REGSIZE; ST r13, r4 };
# define PUSHARGS_6 PUSHARGS_5 { ADDI_PTR r13, sp, 8 * REGSIZE; ST r14, r5 };
# define PUSHARGS_7 PUSHARGS_6 { ADDI_PTR r14, sp, 9 * REGSIZE; ST r13, r6 };

# define POPARGS_0  /* nothing to do */
# define POPARGS_1  POPARGS_0 { ADDI_PTR r14, sp, 3 * REGSIZE; LD r0, r13 };
# define POPARGS_2  POPARGS_1 { ADDI_PTR r13, sp, 4 * REGSIZE; LD r1, r14 };
# define POPARGS_3  POPARGS_2 { ADDI_PTR r14, sp, 5 * REGSIZE; LD r2, r13 };
# define POPARGS_4  POPARGS_3 { ADDI_PTR r13, sp, 6 * REGSIZE; LD r3, r14 };
# define POPARGS_5  POPARGS_4 { ADDI_PTR r14, sp, 7 * REGSIZE; LD r4, r13 };
# define POPARGS_6  POPARGS_5 { ADDI_PTR r13, sp, 8 * REGSIZE; LD r5, r14 };
# define POPARGS_7  POPARGS_6 { ADDI_PTR r14, sp, 9 * REGSIZE; LD r6, r13 };

# define STKSPACE	(13 * REGSIZE)

# ifdef IS_IN_libpthread
#  define CENABLE	jal __pthread_enable_asynccancel
#  define CDISABLE	jal __pthread_disable_asynccancel
# elif defined IS_IN_librt
#  define CENABLE	jal __librt_enable_asynccancel
#  define CDISABLE	jal __librt_disable_asynccancel
# else
#  define CENABLE	jal __libc_enable_asynccancel
#  define CDISABLE	jal __libc_disable_asynccancel
# endif

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P						\
	__builtin_expect (THREAD_GETMEM (THREAD_SELF,			\
					 header.multiple_threads)	\
			  == 0, 1)
# else
#  define SINGLE_THREAD_P(reg)						\
  ADDLI_PTR reg, tp, MULTIPLE_THREADS_OFFSET;                           \
  LD reg, reg;                                                          \
  CMPEQI reg, reg, 0
#endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P                                           \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,                         \
                                   header.multiple_threads) == 0, 1)
#endif
