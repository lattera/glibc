/* Declarations of internal pthread functions used by libc.  Hurd version.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#ifndef _PTHREADP_H
#define _PTHREADP_H	1

#include <pthread.h>

/* These represent the interface used by glibc itself.  */

extern pthread_t __pthread_self (void);
extern int __pthread_kill (pthread_t threadid, int signo);
extern struct __pthread **__pthread_threads;

extern int _pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
extern int __pthread_mutex_lock (pthread_mutex_t *__mutex);
extern int __pthread_mutex_unlock (pthread_mutex_t *__mutex);

extern int __pthread_cond_broadcast (pthread_cond_t *cond);

typedef struct __cthread *__cthread_t;
typedef int __cthread_key_t;
typedef void *	(*__cthread_fn_t)(void *__arg);

__cthread_t __cthread_fork (__cthread_fn_t, void *);
int __pthread_create (pthread_t *newthread,
		      const pthread_attr_t *attr,
		      void *(*start_routine) (void *), void *arg);

void __cthread_detach (__cthread_t);
int __pthread_detach (pthread_t __threadp);
void __pthread_exit (void *value) __attribute__ ((__noreturn__));
int __cthread_keycreate (__cthread_key_t *);
int __cthread_getspecific (__cthread_key_t, void **);
int __cthread_setspecific (__cthread_key_t, void *);
int __pthread_key_create (pthread_key_t *key, void (*destr) (void *));
void *__pthread_getspecific (pthread_key_t key);
int __pthread_setspecific (pthread_key_t key, const void *value);

int __pthread_setcancelstate (int state, int *oldstate);

int __pthread_getattr_np (pthread_t, pthread_attr_t *);
int __pthread_attr_getstackaddr (const pthread_attr_t *__restrict __attr,
				 void **__restrict __stackaddr);
int __pthread_attr_setstackaddr (pthread_attr_t *__attr, void *__stackaddr);
int __pthread_attr_getstacksize (const pthread_attr_t *__restrict __attr,
				 size_t *__restrict __stacksize);
int __pthread_attr_setstacksize (pthread_attr_t *__attr, size_t __stacksize);
int __pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
			     size_t __stacksize);
int __pthread_attr_getstack (const pthread_attr_t *, void **, size_t *);
struct __pthread_cancelation_handler **___pthread_get_cleanup_stack (void) attribute_hidden;

#if IS_IN (libpthread)
hidden_proto (__pthread_key_create)
hidden_proto (_pthread_mutex_init)
#endif

#endif	/* pthreadP.h */
