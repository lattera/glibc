/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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
#include <pt-machine.h>
#ifndef ASSEMBLER
# include <linuxthreads/internals.h>
#endif

#if defined FLOATING_STACKS && USE___THREAD
# define MULTIPLE_THREADS_OFFSET	12
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P;							      \
    jne L(pseudo_cancel);						      \
    DO_CALL (syscall_name, args);					      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
    ret;								      \
  L(pseudo_cancel):							      \
    CENABLE								      \
    SAVE_OLDTYPE_##args							      \
    PUSHARGS_##args							      \
    DOCARGS_##args							      \
    movl $SYS_ify (syscall_name), %eax;					      \
    int $0x80								      \
    POPARGS_##args;							      \
    POPCARGS_##args							      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):

# define SAVE_OLDTYPE_0	movl %eax, %edx;
# define SAVE_OLDTYPE_1	SAVE_OLDTYPE_0
# define SAVE_OLDTYPE_2	pushl %eax;
# define SAVE_OLDTYPE_3	SAVE_OLDTYPE_2
# define SAVE_OLDTYPE_4	SAVE_OLDTYPE_2
# define SAVE_OLDTYPE_5	SAVE_OLDTYPE_2

# define DOCARGS_0	DOARGS_0
# define DOCARGS_1	DOARGS_1
# define DOCARGS_2	_DOARGS_2 (12)
# define DOCARGS_3	_DOARGS_3 (20)
# define DOCARGS_4	_DOARGS_4 (28)
# define DOCARGS_5	_DOARGS_5 (36)

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel;
#  define CDISABLE	call __pthread_disable_asynccancel
# else
#  define CENABLE	call __libc_enable_asynccancel;
#  define CDISABLE	call __libc_disable_asynccancel
# endif
# define POPCARGS_0	pushl %eax; movl %ecx, %eax; CDISABLE; popl %eax;
# define POPCARGS_1	POPCARGS_0
# define POPCARGS_2	xchgl (%esp), %eax; CDISABLE; popl %eax;
# define POPCARGS_3	POPCARGS_2
# define POPCARGS_4	POPCARGS_2
# define POPCARGS_5	POPCARGS_2

#if !defined NOT_IN_libc
# define __local_multiple_threads __libc_multiple_threads
#else
# define __local_multiple_threads __pthread_multiple_threads
#endif

# ifndef ASSEMBLER
#  if defined MULTIPLE_THREADS_OFFSET && defined PIC
#   define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   p_header.data.multiple_threads) == 0, 1)
#  else
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  endif
# else
#  if !defined PIC
#   define SINGLE_THREAD_P cmpl $0, __local_multiple_threads
#  elif defined MULTIPLE_THREADS_OFFSET
#   define SINGLE_THREAD_P cmpl $0, %gs:MULTIPLE_THREADS_OFFSET
#  else
#   if !defined HAVE_HIDDEN || !USE___THREAD
#    define SINGLE_THREAD_P \
  SETUP_PIC_REG (cx);				\
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;		\
  cmpl $0, __local_multiple_threads@GOTOFF(%ecx)
#   else
#    define SINGLE_THREAD_P \
  call __i686.get_pc_thunk.cx;			\
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;		\
  cmpl $0, __local_multiple_threads@GOTOFF(%ecx)
#   endif
#  endif
# endif

#elif !defined ASSEMBLER

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
