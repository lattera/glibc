/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>

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
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .text	;								\
 ENTRY (name)								\
  PUSHARGS_##args							\
  DOARGS_##args								\
  SINGLE_THREAD_P;							\
  bne L(pseudo_cancel);							\
  mov SYS_ify (syscall_name),d0;					\
  syscall 0								\
  POPARGS_##args ;							\
  cmp -126,d0;								\
  bls L(pseudo_end);							\
  jmp SYSCALL_ERROR_LABEL;						\
 L(pseudo_cancel):							\
  add -(16+STACK_SPACE (args)),sp;					\
  SAVE_ARGS_##args							\
  CENABLE								\
  mov d0,r0;								\
  LOAD_ARGS_##args							\
  mov SYS_ify (syscall_name),d0;					\
  syscall 0;								\
  mov d0,(12,sp);							\
  mov r0,d0;								\
  CDISABLE								\
  mov (12,sp),d0;							\
  add +16+STACK_SPACE (args),sp						\
  POPARGS_##args ;							\
  cmp -126,d0;								\
  bls L(pseudo_end);							\
  jmp SYSCALL_ERROR_LABEL;						\
 L(pseudo_end):								\
  mov d0,a0

/* Reserve up to 2 stack slots for a0 and d1, but fewer than that if
   we don't have that many arguments.  */
# define STACK_SPACE(n) (((((n) < 3) * (2 - (n))) + 2) * 4)

# define SAVE_ARGS_0
# define SAVE_ARGS_1	mov a0,(20,sp) ;
# define SAVE_ARGS_2	SAVE_ARGS_1 mov d1,(24,sp) ;
# define SAVE_ARGS_3	SAVE_ARGS_2
# define SAVE_ARGS_4	SAVE_ARGS_3
# define SAVE_ARGS_5	SAVE_ARGS_4
# define SAVE_ARGS_6	SAVE_ARGS_5

# define LOAD_ARGS_0
# define LOAD_ARGS_1	mov (20,sp),a0 ;
# define LOAD_ARGS_2	LOAD_ARGS_1 mov (24,sp),d1 ;
# define LOAD_ARGS_3	LOAD_ARGS_2
# define LOAD_ARGS_4	LOAD_ARGS_3
# define LOAD_ARGS_5	LOAD_ARGS_4
# define LOAD_ARGS_6	LOAD_ARGS_5

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel,[],0;
#  define CDISABLE	call __pthread_disable_asynccancel,[],0;
# elif defined IS_IN_librt
#  ifdef PIC
#   define CENABLE	movm [a2],(sp); \
			1: mov pc,a2; \
			add _GLOBAL_OFFSET_TABLE_-(1b-.),a2; \
			call +__librt_enable_asynccancel@PLT,[],0; \
			movm (sp),[a2];
#   define CENABLE	movm [a2],(sp); \
			1: mov pc,a2; \
			add _GLOBAL_OFFSET_TABLE_-(1b-.),a2; \
			call +__librt_disable_asynccancel@PLT,[],0; \
			movm (sp),[a2];
#  else
#   define CENABLE	call +__librt_enable_asynccancel,[],0;
#   define CDISABLE	call +__librt_disable_asynccancel,[],0;
#  endif
# else
#  define CENABLE	call +__libc_enable_asynccancel,[],0;
#  define CDISABLE	call +__libc_disable_asynccancel,[],0;
# endif

#if !defined NOT_IN_libc
# define __local_multiple_threads __libc_multiple_threads
#elif defined IS_IN_libpthread
# define __local_multiple_threads __pthread_multiple_threads
#else
# define __local_multiple_threads __librt_multiple_threads
#endif

# ifndef __ASSEMBLER__
#  if defined FLOATING_STACKS && USE___THREAD && defined PIC
#   define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   p_header.data.multiple_threads) == 0, 1)
#  else
extern int __local_multiple_threads
#   if !defined NOT_IN_libc || defined IS_IN_libpthread
  attribute_hidden;
#   else
  ;
#   endif
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  endif
# else
#  if !defined PIC
#   define SINGLE_THREAD_P \
	mov (+__local_multiple_threads),d0; \
	cmp 0,d0
#  elif !defined NOT_IN_libc || defined IS_IN_libpthread
#   define SINGLE_THREAD_P \
	movm [a2],(sp); \
     1: mov pc,a2; \
	add _GLOBAL_OFFSET_TABLE_-(1b-.),a2; \
	mov (+__local_multiple_threads@GOTOFF,a2),d0; \
	movm (sp),[a2]; \
	cmp 0,d0
#  else
#   define SINGLE_THREAD_P \
	movm [a2],(sp); \
     1: mov pc,a2; \
	add _GLOBAL_OFFSET_TABLE_-(1b-.),a2; \
	mov (+__local_multiple_threads@GOT,a2),a2; \
	mov (a2),d0; \
	movm (sp),[a2]; \
	cmp 0,d0
#  endif
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
