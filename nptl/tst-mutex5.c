/* Copyright (C) 2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


static int
do_test (void)
{
  pthread_mutex_t m;
  struct timespec ts;
  struct timeval tv;
  struct timeval tv2;
  int err;

  if (pthread_mutex_init (&m, NULL) != 0)
    {
      puts ("mutex_init failed");
      return 1;
    }

  if (pthread_mutex_lock (&m) != 0)
    {
      puts ("mutex_lock failed");
      return 1;
    }

  if (pthread_mutex_trylock (&m) == 0)
    {
      puts ("mutex_trylock succeeded");
      return 1;
    }

  gettimeofday (&tv, NULL);
  TIMEVAL_TO_TIMESPEC (&tv, &ts);

  ts.tv_sec += 2;	/* Wait 2 seconds.  */

  err = pthread_mutex_timedlock (&m, &ts);
  if (err == 0)
    {
      puts ("timed_lock succeeded");
      return 1;
    }
  else if (err != ETIMEDOUT)
    {
      printf ("timed_lock error != ETIMEDOUT: %d\n", err);
      return 1;
    }
  else
    {
      int clk_tck = sysconf (_SC_CLK_TCK);

      gettimeofday (&tv2, NULL);

      tv2.tv_sec -= tv.tv_sec;
      tv2.tv_usec -= tv.tv_usec;
      if (tv2.tv_usec < 0)
	{
	  tv2.tv_usec += 1000000;
	  tv2.tv_sec -= 1;
	}

      /* Be a bit tolerant, add one CLK_TCK.  */
      tv2.tv_usec += 1000000 / clk_tck;
      if (tv2.tv_usec >= 1000000)
	{
	  tv2.tv_usec -= 1000000;
	  ++tv2.tv_sec;
	}

      if (tv2.tv_sec < 2)
	{
	  printf ("premature timeout: %ld.%06ld difference\n",
		  tv2.tv_sec, tv2.tv_usec);
	  return 1;
	}
    }

  if (pthread_mutex_unlock (&m) != 0)
    {
      puts ("mutex_unlock failed");
      return 1;
    }

  if (pthread_mutex_destroy (&m) != 0)
    {
      puts ("mutex_destroy failed");
      return 1;
    }

  return 0;
}

#define TIMEOUT 4
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
