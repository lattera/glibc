/* Test for pthread cancellation of mutex blocks.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t b;
int value = 0;

static void *
tf (void *arg)
{
  int r = pthread_barrier_wait (&b);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_mutex_lock (&lock);
  ++value;
  pthread_testcancel ();
  ++value;
  pthread_mutex_unlock (&lock);
  return NULL;
}


static int
do_test (void)
{
  pthread_mutex_lock (&lock);

  if (pthread_barrier_init (&b, NULL, 2) != 0)
    {
      puts ("barrier init failed");
      return 1;
    }

  pthread_t th;
  if (pthread_create (&th, NULL, tf, NULL) != 0)
    {
      puts ("pthread_create failed");
      return 1;
    }

  int r = pthread_barrier_wait (&b);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (pthread_cancel (th) != 0)
    {
      puts ("pthread_cancel failed");
      return 1;
    }

  pthread_mutex_unlock (&lock);

  void *status;
  if (pthread_join (th, &status) != 0)
    {
      puts ("join failed");
      return 1;
    }

  if (status != PTHREAD_CANCELED)
    {
      puts ("thread not canceled");
      return 1;
    }

  if (value == 0)
    {
      puts ("thread cancelled in the pthread_mutex_lock call");
      return 1;
    }

  if (value != 1)
    {
      puts ("thread not cancelled in pthread_testcancel call");
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
