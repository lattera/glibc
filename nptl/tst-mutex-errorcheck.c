/* Check that error checking mutexes are not subject to lock elision.
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

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

static int
do_test (void)
{
  struct timespec tms = { 0 };
  pthread_mutex_t mutex;
  pthread_mutexattr_t mutexattr;
  int ret = 0;

  if (pthread_mutexattr_init (&mutexattr) != 0)
    return 1;
  if (pthread_mutexattr_settype (&mutexattr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    return 1;

  if (pthread_mutex_init (&mutex, &mutexattr) != 0)
    return 1;
  if (pthread_mutexattr_destroy (&mutexattr) != 0)
    return 1;

  /* The call to pthread_mutex_timedlock erroneously enabled lock elision
     on the mutex, which then triggered an assertion failure in
     pthread_mutex_unlock.  It would also defeat the error checking nature
     of the mutex.  */
  if (pthread_mutex_timedlock (&mutex, &tms) != 0)
    return 1;
  if (pthread_mutex_timedlock (&mutex, &tms) != EDEADLK)
    {
      printf ("Failed error checking on locked mutex\n");
      ret = 1;
    }

  if (pthread_mutex_unlock (&mutex) != 0)
    ret = 1;

  return ret;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
