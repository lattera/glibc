/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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

#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t syncm = PTHREAD_MUTEX_INITIALIZER;


static void *
tf (void *a)
{
  int i = (long int) a;
  int err;

  printf ("child %d: lock\n", i);

  err = pthread_mutex_lock (&mut);
  if (err != 0)
    error (EXIT_FAILURE, err, "locking in child failed");

  printf ("child %d: unlock sync\n", i);

  err = pthread_mutex_unlock (&syncm);
  if (err != 0)
    error (EXIT_FAILURE, err, "child %d: unlock[1] failed", i);

  printf ("child %d: wait\n", i);

  err = pthread_cond_wait (&cond, &mut);
  if (err != 0)
    error (EXIT_FAILURE, err, "child %d: failed to wait", i);

  printf ("child %d: woken up\n", i);

  err = pthread_mutex_unlock (&mut);
  if (err != 0)
    error (EXIT_FAILURE, err, "child %d: unlock[2] failed", i);

  printf ("child %d: done\n", i);

  return NULL;
}


#define N 10


static int
do_test (void)
{
  pthread_t th[N];
  int i;
  int err;

  printf ("&cond = %p\n&mut = %p\n", &cond, &mut);

  puts ("first lock");

  err = pthread_mutex_lock (&syncm);
  if (err != 0)
    error (EXIT_FAILURE, err, "initial locking failed");

  for (i = 0; i < N; ++i)
    {
      printf ("create thread %d\n", i);

      err = pthread_create (&th[i], NULL, tf, (void *) (long int) i);
      if (err != 0)
	error (EXIT_FAILURE, err, "cannot create thread %d", i);

      printf ("wait for child %d\n", i);

      /* Lock and thereby wait for the child to start up and get the
	 mutex for the conditional variable.  */
      pthread_mutex_lock (&syncm);
      /* Unlock right away.  Yes, we can use barriers but then we
	 would test more functionality here.  */
      pthread_mutex_unlock (&syncm);
    }

  puts ("get lock outselves");

  err = pthread_mutex_lock (&mut);
  if (err != 0)
    error (EXIT_FAILURE, err, "mut locking failed");

  puts ("broadcast");

  /* Wake up all threads.  */
  err = pthread_cond_broadcast (&cond);
  if (err != 0)
    error (EXIT_FAILURE, err, "parent: broadcast failed");

  err = pthread_mutex_unlock (&mut);
  if (err != 0)
    error (EXIT_FAILURE, err, "mut unlocking failed");

  /* Join all threads.  */
  for (i = 0; i < N; ++i)
    {
      printf ("join thread %d\n", i);

      err = pthread_join (th[i], NULL);
      if (err != 0)
	error (EXIT_FAILURE, err, "join of child %d failed", i);
    }

  puts ("done");

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
