/* Copyright (C) 2002 Free Software Foundation, Inc.
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

#ifndef _PTHREADP_H
#define _PTHREADP_H	1

#include <pthread.h>
#include <setjmp.h>
#include <stdint.h>
#include <sys/syscall.h>
#include "descr.h"
#include <tls.h>
#include <lowlevellock.h>
#include <stackinfo.h>
#include <internaltypes.h>


/* Internal variables.  */


/* Default stack size.  */
extern size_t __default_stacksize attribute_hidden;

/* Size and alignment of static TLS block.  */
extern size_t __static_tls_size attribute_hidden;
extern size_t __static_tls_align attribute_hidden;

/* Thread descriptor handling.  */
extern list_t __stack_user;
hidden_proto (__stack_user)

/* Attribute handling.  */
extern struct pthread_attr *__attr_list attribute_hidden;
extern lll_lock_t __attr_list_lock attribute_hidden;

/* First available RT signal.  */
extern int __current_sigrtmin attribute_hidden;
/* Last available RT signal.  */
extern int __current_sigrtmax attribute_hidden;

/* Concurrency handling.  */
extern int __concurrency_level attribute_hidden;

/* Thread-local data key handling.  */
extern struct pthread_key_struct __pthread_keys[PTHREAD_KEYS_MAX];
hidden_proto (__pthread_keys)

/* The library can run in debugging mode where it performs a lot more
   tests.  */
extern int __pthread_debug attribute_hidden;
#define DEBUGGING_P __builtin_expect (__pthread_debug, 0)


/* Cancellation test.  */
#define CANCELLATION_P(self) \
  do {									      \
    int cancelhandling = THREAD_GETMEM (self, cancelhandling);		      \
    if (CANCEL_ENABLED_AND_CANCELED (cancelhandling))			      \
      {									      \
	THREAD_SETMEM (self, result, PTHREAD_CANCELED);			      \
	__do_cancel ();							      \
      }									      \
  } while (0)

/* Set cancellation mode to asynchronous.  */
#define CANCEL_ASYNC() \
  __pthread_enable_asynccancel ()
/* Reset to previous cancellation mode.  */
#define CANCEL_RESET(oldtype) \
  __pthread_disable_asynccancel (oldtype)

/* Same as CANCEL_ASYNC, but for use in libc.so.  */
#define LIBC_CANCEL_ASYNC() \
  __libc_enable_asynccancel ()
/* Same as CANCEL_RESET, but for use in libc.so.  */
#define LIBC_CANCEL_RESET(oldtype) \
  __libc_disable_asynccancel (oldtype)


/* This function is responsible for calling all registered cleanup
   handlers and then terminate the thread.  This includes dellocating
   the thread-specific data.  The implementation is complicated by the
   fact that we have to handle to cancellation handler registration
   methods: exceptions using try/finally and setjmp.

   The setjmp method is always available.  The user might compile some
   code which uses this method because no modern compiler is
   available.  So we have to handle these first since we cannot call
   the cleanup handlers if the stack frames are gone.  At the same
   time this opens a hole for the register exception handler blocks
   since now they might be in danger of using an overwritten stack
   frame.  The advise is to only use new or only old style cancellation
   handling.  */
static inline void
__do_cancel (void)
{
  struct pthread *self = THREAD_SELF;

  /* Throw an exception.  */
  // XXX TBI

  /* If throwing an exception didn't work try the longjmp.  */
  __libc_longjmp (self->cancelbuf, 1);

  /* NOTREACHED */
}


/* Test whether stackframe is still active.  */
#ifdef _STACK_GROWS_DOWN
# define FRAME_LEFT(frame, other) ((char *) frame >= (char *) other)
#elif _STACK_GROWS_UP
# define FRAME_LEFT(frame, other) ((char *) frame <= (char *) other)
#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif


/* Internal prototypes.  */

/* Thread list handling.  */
extern struct pthread *__find_in_stack_list (struct pthread *pd)
     attribute_hidden;

/* Deallocate a thread's stack after optionally making sure the thread
   descriptor is still valid.  */
extern void __free_tcb (struct pthread *pd) attribute_hidden;

/* Free allocated stack.  */
extern void __deallocate_stack (struct pthread *pd) attribute_hidden;

/* Mark all the stacks except for the current one as available.  This
   function also re-initializes the lock for the stack cache.  */
extern void __reclaim_stacks (void) attribute_hidden;

/* longjmp handling.  */
extern void __pthread_cleanup_upto (__jmp_buf target, char *targetframe);


/* Functions with versioned interfaces.  */
extern int __pthread_create_2_1 (pthread_t *newthread,
				 const pthread_attr_t *attr,
				 void *(*start_routine) (void *), void *arg);
extern int __pthread_create_2_0 (pthread_t *newthread,
				 const pthread_attr_t *attr,
				 void *(*start_routine) (void *), void *arg);
extern int __pthread_attr_init_2_1 (pthread_attr_t *attr);
extern int __pthread_attr_init_2_0 (pthread_attr_t *attr);


/* Event handlers for libthread_db interface.  */
extern void __nptl_create_event (void);
extern void __nptl_death_event (void);
hidden_proto (__nptl_create_event)
hidden_proto (__nptl_death_event)


/* Namespace save aliases.  */
extern int __pthread_mutex_init (pthread_mutex_t *__mutex,
				 __const pthread_mutexattr_t *__mutexattr);
extern int __pthread_mutex_destroy (pthread_mutex_t *__mutex);
extern int __pthread_mutex_trylock (pthread_mutex_t *_mutex);
extern int __pthread_mutex_lock (pthread_mutex_t *__mutex);
extern int __pthread_mutex_lock_internal (pthread_mutex_t *__mutex);
extern int __pthread_mutex_unlock (pthread_mutex_t *__mutex);
extern int __pthread_mutex_unlock_internal (pthread_mutex_t *__mutex);
extern int __pthread_mutexattr_init (pthread_mutexattr_t *attr);
extern int __pthread_mutexattr_destroy (pthread_mutexattr_t *attr);
extern int __pthread_mutexattr_settype (pthread_mutexattr_t *attr, int kind);
extern int __pthread_attr_getstackaddr (__const pthread_attr_t *__restrict
					__attr, void **__restrict __stackaddr);
extern int __pthread_attr_setstackaddr (pthread_attr_t *__attr,
					void *__stackaddr);
extern int __pthread_attr_getstacksize (__const pthread_attr_t *__restrict
					__attr,
					size_t *__restrict __stacksize);
extern int __pthread_attr_setstacksize (pthread_attr_t *__attr,
					size_t __stacksize);
extern int __pthread_attr_getstack (__const pthread_attr_t *__restrict __attr,
				    void **__restrict __stackaddr,
				    size_t *__restrict __stacksize);
extern int __pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
				    size_t __stacksize);
extern int __pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
				  __const pthread_rwlockattr_t *__restrict
				  __attr);
extern int __pthread_rwlock_destroy (pthread_rwlock_t *__rwlock);
extern int __pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock);
extern int __pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock);
extern int __pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock);
extern int __pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock);
extern int __pthread_rwlock_unlock (pthread_rwlock_t *__rwlock);
extern int __pthread_key_create (pthread_key_t *key, void (*destr) (void *));
extern void *__pthread_getspecific (pthread_key_t key);
extern int __pthread_setspecific (pthread_key_t key, const void *value);
extern int __pthread_once (pthread_once_t *once_control,
			   void (*init_routine) (void));
extern int __pthread_once_internal (pthread_once_t *once_control,
				    void (*init_routine) (void));
extern int __pthread_atfork (void (*prepare) (void), void (*parent) (void),
			     void (*child) (void));
extern int __pthread_kill (pthread_t threadid, int signo);
extern int __pthread_setcanceltype (int type, int *oldtype);
extern int __pthread_enable_asynccancel (void) attribute_hidden;
extern void __pthread_disable_asynccancel (int oldtype)
     internal_function attribute_hidden;

/* The two functions are in libc.so and not exported.  */
extern int __libc_enable_asynccancel (void) attribute_hidden;
extern void __libc_disable_asynccancel (int oldtype)
     internal_function attribute_hidden;

#ifdef IS_IN_libpthread
/* Special versions which use non-exported functions.  */
extern void _GI_pthread_cleanup_push (struct _pthread_cleanup_buffer *buffer,
				      void (*routine) (void *), void *arg)
     attribute_hidden;
# undef pthread_cleanup_push
# define pthread_cleanup_push(routine,arg) \
  { struct _pthread_cleanup_buffer _buffer;				      \
    _GI_pthread_cleanup_push (&_buffer, (routine), (arg));

extern void _GI_pthread_cleanup_pop (struct _pthread_cleanup_buffer *buffer,
				     int execute) attribute_hidden;
# undef pthread_cleanup_pop
# define pthread_cleanup_pop(execute) \
    _GI_pthread_cleanup_pop (&_buffer, (execute)); }
#endif

#endif	/* pthreadP.h */
