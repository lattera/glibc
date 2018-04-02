/* Initialize a mutex.  Generic version.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <pt-internal.h>

int
_pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  *mutex = (pthread_mutex_t) __PTHREAD_MUTEX_INITIALIZER;

  if (attr == NULL
      || memcmp (attr, &__pthread_default_mutexattr, sizeof (*attr) == 0))
    /* The default attributes.  */
    return 0;

  if (mutex->__attr == NULL
      || mutex->__attr == __PTHREAD_ERRORCHECK_MUTEXATTR
      || mutex->__attr == __PTHREAD_RECURSIVE_MUTEXATTR)
    mutex->__attr = malloc (sizeof *attr);

  if (mutex->__attr == NULL)
    return ENOMEM;

  *mutex->__attr = *attr;
  return 0;
}

strong_alias (_pthread_mutex_init, pthread_mutex_init);
