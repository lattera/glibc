/* Semaphore type.  Generic version.
   Copyright (C) 2005-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_SEMAPHORE_H
#define _BITS_SEMAPHORE_H	1

#ifndef _SEMAPHORE_H
# error Never include <bits/semaphore.h> directly.
#endif

#include <bits/types/__pthread_spinlock_t.h>
#include <bits/pthread.h>

/* User visible part of a semaphore.  */
struct __semaphore
{
  __pthread_spinlock_t __lock;
  struct __pthread *__queue;
  int __pshared;
  int __value;
  void *__data;
};

typedef struct __semaphore sem_t;

#define SEM_FAILED ((void *) 0)

/* Initializer for a semaphore.  */
#define __SEMAPHORE_INITIALIZER(pshared, value) \
  { __PTHREAD_SPIN_LOCK_INITIALIZER, NULL, (pshared), (value), NULL }

#endif /* bits/semaphore.h */
