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

extern int __pthread_mutex_lock (pthread_mutex_t *__mutex);
extern int __pthread_mutex_unlock (pthread_mutex_t *__mutex);

typedef struct __cthread *__cthread_t;
typedef int __cthread_key_t;
typedef void *	(*__cthread_fn_t)(void *__arg);

__cthread_t __cthread_fork (__cthread_fn_t, void *);
void __cthread_detach (__cthread_t);
int __cthread_keycreate (__cthread_key_t *);
int __cthread_getspecific (__cthread_key_t, void **);
int __cthread_setspecific (__cthread_key_t, void *);

int __pthread_getattr_np (pthread_t, pthread_attr_t *);
int __pthread_attr_getstack (const pthread_attr_t *, void **, size_t *);

#endif	/* pthreadP.h */
