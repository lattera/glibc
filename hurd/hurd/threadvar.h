/* Internal per-thread variables for the Hurd.
   Copyright (C) 1994,95,97,98,99,2001,02,06,07 Free Software Foundation, Inc.
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

#ifndef _HURD_THREADVAR_H
#define	_HURD_THREADVAR_H

#include <features.h>

/* The per-thread variables are found by ANDing this mask
   with the value of the stack pointer and then adding this offset.

   In the multi-threaded case, cthreads initialization sets
   __hurd_threadvar_stack_mask to ~(cthread_stack_size - 1), a mask which
   finds the base of the fixed-size cthreads stack; and
   __hurd_threadvar_stack_offset to a small offset that skips the data
   cthreads itself maintains at the base of each thread's stack.

   In the single-threaded case, __hurd_threadvar_stack_mask is zero, so the
   stack pointer is ignored; and __hurd_threadvar_stack_offset gives the
   address of a small allocated region which contains the variables for the
   single thread.  */

extern unsigned long int __hurd_threadvar_stack_mask;
extern unsigned long int __hurd_threadvar_stack_offset;

/* A special case must always be made for the signal thread.  Even when there
   is only one user thread and an allocated region can be used for the user
   thread's variables, the signal thread needs to have its own location for
   per-thread variables.  The variables __hurd_sigthread_stack_base and
   __hurd_sigthread_stack_end define the bounds of the stack used by the
   signal thread, so that thread can always be specifically identified.  */

extern unsigned long int __hurd_sigthread_stack_base;
extern unsigned long int __hurd_sigthread_stack_end;
extern unsigned long int *__hurd_sigthread_variables;


/* At the location described by the two variables above,
   there are __hurd_threadvar_max `unsigned long int's of per-thread data.  */
extern unsigned int __hurd_threadvar_max;

/* These values are the indices for the standard per-thread variables.  */
enum __hurd_threadvar_index
  {
    _HURD_THREADVAR_MIG_REPLY,	/* Reply port for MiG user stub functions.  */
    _HURD_THREADVAR_ERRNO,	/* `errno' value for this thread.  */
    _HURD_THREADVAR_SIGSTATE,	/* This thread's `struct hurd_sigstate'.  */
    _HURD_THREADVAR_DYNAMIC_USER, /* Dynamically-assigned user variables.  */
    _HURD_THREADVAR_MALLOC,	/* For use of malloc.  */
    _HURD_THREADVAR_DL_ERROR,	/* For use of -ldl and dynamic linker.  */
    _HURD_THREADVAR_RPC_VARS,	/* For state of RPC functions.  */
    _HURD_THREADVAR_LOCALE,	/* For thread-local locale setting.  */
    _HURD_THREADVAR_CTYPE_B,	/* Cache of thread-local locale data.  */
    _HURD_THREADVAR_CTYPE_TOLOWER, /* Cache of thread-local locale data.  */
    _HURD_THREADVAR_CTYPE_TOUPPER, /* Cache of thread-local locale data.  */
    _HURD_THREADVAR_MAX		/* Default value for __hurd_threadvar_max.  */
  };


#ifndef _HURD_THREADVAR_H_EXTERN_INLINE
#define _HURD_THREADVAR_H_EXTERN_INLINE __extern_inline
#endif

/* Return the location of the value for the per-thread variable with index
   INDEX used by the thread whose stack pointer is SP.  */

extern unsigned long int *__hurd_threadvar_location_from_sp
  (enum __hurd_threadvar_index __index, void *__sp);
_HURD_THREADVAR_H_EXTERN_INLINE unsigned long int *
__hurd_threadvar_location_from_sp (enum __hurd_threadvar_index __index,
				   void *__sp)
{
  unsigned long int __stack = (unsigned long int) __sp;
  return &((__stack >= __hurd_sigthread_stack_base &&
	    __stack < __hurd_sigthread_stack_end)
	   ? __hurd_sigthread_variables
	   : (unsigned long int *) ((__stack & __hurd_threadvar_stack_mask) +
				    __hurd_threadvar_stack_offset))[__index];
}

#include <machine-sp.h>		/* Define __thread_stack_pointer.  */

/* Return the location of the current thread's value for the
   per-thread variable with index INDEX.  */

extern unsigned long int *
__hurd_threadvar_location (enum __hurd_threadvar_index __index) __THROW
     /* This declaration tells the compiler that the value is constant
	given the same argument.  We assume this won't be called twice from
	the same stack frame by different threads.  */
     __attribute__ ((__const__));

_HURD_THREADVAR_H_EXTERN_INLINE unsigned long int *
__hurd_threadvar_location (enum __hurd_threadvar_index __index)
{
  return __hurd_threadvar_location_from_sp (__index,
					    __thread_stack_pointer ());
}


#endif	/* hurd/threadvar.h */
