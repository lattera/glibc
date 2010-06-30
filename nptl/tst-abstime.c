/* Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@redhat.com>, 2010.

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

#include <assert.h>
#include <errno.h>
#include <pthread.h>

pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw1 = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t rw2 = PTHREAD_RWLOCK_INITIALIZER;

static void *
th (void *arg)
{
  int r;
  struct timespec t = { -2, 0 };

  r = pthread_mutex_timedlock (&m1, &t);
  assert (r == ETIMEDOUT);
  r = pthread_rwlock_timedrdlock (&rw1, &t);
  assert (r == ETIMEDOUT);
  r = pthread_rwlock_timedwrlock (&rw2, &t);
  assert (r == ETIMEDOUT);
  return 0;
}

int
do_test (void)
{
  int r;
  struct timespec t = { -2, 0 };
  pthread_t pth;

  pthread_mutex_lock (&m1);
  pthread_rwlock_wrlock (&rw1);
  pthread_rwlock_rdlock (&rw2);
  pthread_mutex_lock (&m2);
  pthread_create (&pth, 0, th, 0);
  r = pthread_cond_timedwait (&c, &m2, &t);
  assert (r == ETIMEDOUT);
  pthread_join (pth, 0);
  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
