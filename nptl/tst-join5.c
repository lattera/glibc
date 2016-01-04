/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


static void
wait_code (void)
{
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 200000000 };
  while (nanosleep (&ts, &ts) < 0)
    ;
}


#ifdef WAIT_IN_CHILD
static pthread_barrier_t b;
#endif


static void *
tf1 (void *arg)
{
#ifdef WAIT_IN_CHILD
  int e = pthread_barrier_wait (&b);
  if (e != 0 && e != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __func__);
      exit (1);
    }

  wait_code ();
#endif

  pthread_join ((pthread_t) arg, NULL);

  exit (42);
}


static void *
tf2 (void *arg)
{
#ifdef WAIT_IN_CHILD
  int e = pthread_barrier_wait (&b);
  if (e != 0 && e != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __func__);
      exit (1);
    }

  wait_code ();
#endif
  pthread_join ((pthread_t) arg, NULL);

  exit (43);
}


static int
do_test (void)
{
#ifdef WAIT_IN_CHILD
  if (pthread_barrier_init (&b, NULL, 2) != 0)
    {
      puts ("barrier_init failed");
      return 1;
    }
#endif

  pthread_t th;

  int err = pthread_join (pthread_self (), NULL);
  if (err == 0)
    {
      puts ("1st circular join succeeded");
      return 1;
    }
  if (err != EDEADLK)
    {
      printf ("1st circular join %d, not EDEADLK\n", err);
      return 1;
    }

  if (pthread_create (&th, NULL, tf1, (void *) pthread_self ()) != 0)
    {
      puts ("1st create failed");
      return 1;
    }

#ifndef WAIT_IN_CHILD
  wait_code ();
#endif

  if (pthread_cancel (th) != 0)
    {
      puts ("cannot cancel 1st thread");
      return 1;
    }

#ifdef WAIT_IN_CHILD
  int e = pthread_barrier_wait (&b);
  if (e != 0 && e != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __func__);
      return 1;
    }
#endif

  void *r;
  err = pthread_join (th, &r);
  if (err != 0)
    {
      printf ("cannot join 1st thread: %d\n", err);
      return 1;
    }
  if (r != PTHREAD_CANCELED)
    {
      puts ("1st thread not canceled");
      return 1;
    }

  err = pthread_join (pthread_self (), NULL);
  if (err == 0)
    {
      puts ("2nd circular join succeeded");
      return 1;
    }
  if (err != EDEADLK)
    {
      printf ("2nd circular join %d, not EDEADLK\n", err);
      return 1;
    }

  if (pthread_create (&th, NULL, tf2, (void *) pthread_self ()) != 0)
    {
      puts ("2nd create failed");
      return 1;
    }

#ifndef WAIT_IN_CHILD
  wait_code ();
#endif

  if (pthread_cancel (th) != 0)
    {
      puts ("cannot cancel 2nd thread");
      return 1;
    }

#ifdef WAIT_IN_CHILD
  e = pthread_barrier_wait (&b);
  if (e != 0 && e != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __func__);
      return 1;
    }
#endif

  if (pthread_join (th, &r) != 0)
    {
      puts ("cannot join 2nd thread");
      return 1;
    }
  if (r != PTHREAD_CANCELED)
    {
      puts ("2nd thread not canceled");
      return 1;
    }

  err = pthread_join (pthread_self (), NULL);
  if (err == 0)
    {
      puts ("3rd circular join succeeded");
      return 1;
    }
  if (err != EDEADLK)
    {
      printf ("3rd circular join %d, not EDEADLK\n", err);
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
