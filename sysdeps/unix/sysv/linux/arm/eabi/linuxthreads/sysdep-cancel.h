/* Copyright (C) 2003, 2005 Free Software Foundation, Inc.
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

#include <sysdep.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
    PSEUDO_PROLOGUE;							\
  ENTRY (name);								\
    SINGLE_THREAD_P;							\
    DOARGS_##args;							\
    bne .Lpseudo_cancel;						\
    mov ip, r7;								\
    ldr r7, =SYS_ify (syscall_name);					\
    swi 0x0;								\
    mov r7, ip;								\
    UNDOARGS_##args;							\
    cmn r0, $4096;							\
    PSEUDO_RET;								\
  .Lpseudo_cancel:							\
    DOCARGS_##args;	/* save syscall args etc. around CENABLE.  */	\
    CENABLE;								\
    mov ip, r0;		/* put mask in safe place.  */			\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    ldr r7, =SYS_ify (syscall_name);					\
    swi 0x0;		/* do the call.  */				\
    mov r7, r0;		/* save syscall return value.  */		\
    mov r0, ip;		/* get mask back.  */				\
    CDISABLE;								\
    mov r0, r7;		/* retrieve return value.  */			\
    RESTORE_LR_##args;							\
    UNDOARGS_##args;							\
    cmn r0, $4096;

/* DOARGS pushes four bytes on the stack for five arguments, eight bytes for
   six arguments, and nothing for fewer.  In order to preserve doubleword
   alignment, sometimes we must save an extra register.  */

# define DOCARGS_0	stmfd sp!, {r7, lr}
# define UNDOCARGS_0
# define RESTORE_LR_0	ldmfd sp!, {r7, lr}

# define DOCARGS_1	stmfd sp!, {r0, r1, r7, lr}
# define UNDOCARGS_1	ldr r0, [sp], #8
# define RESTORE_LR_1	RESTORE_LR_0

# define DOCARGS_2	stmfd sp!, {r0, r1, r7, lr}
# define UNDOCARGS_2	ldmfd sp!, {r0, r1}
# define RESTORE_LR_2	RESTORE_LR_0

# define DOCARGS_3	stmfd sp!, {r0, r1, r2, r3, r7, lr}
# define UNDOCARGS_3	ldmfd sp!, {r0, r1, r2, r3}
# define RESTORE_LR_3	RESTORE_LR_0

# define DOCARGS_4	stmfd sp!, {r0, r1, r2, r3, r7, lr}
# define UNDOCARGS_4	ldmfd sp!, {r0, r1, r2, r3}
# define RESTORE_LR_4	RESTORE_LR_0

# define DOCARGS_5	stmfd sp!, {r0, r1, r2, r3, r4, r7, lr}
# define UNDOCARGS_5	ldmfd sp!, {r0, r1, r2, r3}
# define RESTORE_LR_5	ldmfd sp!, {r4, r7, lr}

# define DOCARGS_6	stmfd sp!, {r0, r1, r2, r3, r7, lr}
# define UNDOCARGS_6	ldmfd sp!, {r0, r1, r2, r3}
# define RESTORE_LR_6	RESTORE_LR_0

# ifdef IS_IN_libpthread
#  define CENABLE	bl PLTJMP(__pthread_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# else
#  define CENABLE	bl PLTJMP(__libc_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# endif

# ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#  define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
# else
#  define SINGLE_THREAD_P						\
  ldr ip, 1b;								\
2:									\
  ldr ip, [pc, ip];							\
  teq ip, #0;
#  define PSEUDO_PROLOGUE						\
  1:  .word __local_multiple_threads - 2f - 8;
# endif

#else

/* For rtld, et cetera.  */
# define SINGLE_THREAD_P 1

#endif
