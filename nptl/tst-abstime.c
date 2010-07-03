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

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

static pthread_cond_t c = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t rw1 = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t rw2 = PTHREAD_RWLOCK_INITIALIZER;
static sem_t sem;

static void *
th (void *arg)
{
  long int res = 0;
  int r;
  struct timespec t = { -2, 0 };

  r = pthread_mutex_timedlock (&m1, &t);
  if (r != ETIMEDOUT)
    {
      puts ("pthread_mutex_timedlock did not return ETIMEDOUT");
      res = 1;
    }
  r = pthread_rwlock_timedrdlock (&rw1, &t);
  if (r != ETIMEDOUT)
    {
      puts ("pthread_rwlock_timedrdlock did not return ETIMEDOUT");
      res = 1;
    }
  r = pthread_rwlock_timedwrlock (&rw2, &t);
  if (r != ETIMEDOUT)
    {
      puts ("pthread_rwlock_timedwrlock did not return ETIMEDOUT");
      res = 1;
    }
  return (void *) res;
}

static int
do_test (void)
{
  int res = 0;
  int r;
  struct timespec t = { -2, 0 };
  pthread_t pth;

  sem_init (&sem, 0, 0);
  r = sem_timedwait (&sem, &t);
  if (r != -1 || errno != ETIMEDOUT)
    {
      puts ("sem_timedwait did not fail with ETIMEDOUT");
      res = 1;
    }

  pthread_mutex_lock (&m1);
  pthread_rwlock_wrlock (&rw1);
  pthread_rwlock_rdlock (&rw2);
  pthread_mutex_lock (&m2);
  if (pthread_create (&pth, 0, th, 0) != 0)
    {
      puts ("cannot create thread");
      return 1;
    }
  r = pthread_cond_timedwait (&c, &m2, &t);
  if (r != ETIMEDOUT)
    {
      puts ("pthread_cond_timedwait did not return ETIMEDOUT");
      res = 1;
    }
  void *thres;
  pthread_join (pth, &thres);
  return res | (thres != NULL);
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
