/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <error.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>


#define N	4

static char stacks[N][PTHREAD_STACK_MIN];
static ucontext_t ctx[N][2];
static volatile int failures;


static void
fct (long int n)
{
  /* Just to use the thread local descriptor.  */
  printf ("%ld: in %s now\n", n, __FUNCTION__);
  errno = 0;
}


static void *
tf (void *arg)
{
  int n = (int) (long int) arg;

  if (getcontext (&ctx[n][1]) != 0)
    {
      printf ("%d: cannot get context: %m\n", n);
      exit (1);
    }

  printf ("%d: %s: before makecontext\n", n, __FUNCTION__);

  ctx[n][1].uc_stack.ss_sp = stacks[n];
  ctx[n][1].uc_stack.ss_size = PTHREAD_STACK_MIN;
  ctx[n][1].uc_link = &ctx[n][0];
  makecontext (&ctx[n][1], (void (*) (void)) fct, 1, (long int) n);

  printf ("%d: %s: before swapcontext\n", n, __FUNCTION__);

  if (swapcontext (&ctx[n][0], &ctx[n][1]) != 0)
    {
      ++failures;
      printf ("%d: %s: swapcontext failed\n", n, __FUNCTION__);
    }
  else
    printf ("%d: back in %s\n", n, __FUNCTION__);

  return NULL;
}


static volatile int global;


static int
do_test (void)
{
  int n;
  pthread_t th[N];
  ucontext_t mctx;

  puts ("making contexts");
  if (getcontext (&mctx) != 0)
    {
      if (errno == ENOSYS)
	{
	  puts ("context handling not supported");
	  exit (0);
	}

      printf ("%s: getcontext: %m\n", __FUNCTION__);
      exit (1);
    }

  /* Play some tricks with this context.  */
  if (++global == 1)
    if (setcontext (&mctx) != 0)
      {
	puts ("setcontext failed");
	exit (1);
      }
  if (global != 2)
    {
      puts ("global not incremented twice");
      exit (1);
    }
  puts ("global OK");

  pthread_attr_t at;

  if (pthread_attr_init (&at) != 0)
    {
      puts ("attr_init failed");
      return 1;
    }

  if (pthread_attr_setstacksize (&at, 1 * 1024 * 1024) != 0)
    {
      puts ("attr_setstacksize failed");
      return 1;
    }

  for (n = 0; n < N; ++n)
    if (pthread_create (&th[n], &at, tf, (void *) (long int) n) != 0)
      {
	puts ("create failed");
	exit (1);
      }

  if (pthread_attr_destroy (&at) != 0)
    {
      puts ("attr_destroy failed");
      return 1;
    }

  for (n = 0; n < N; ++n)
    if (pthread_join (th[n], NULL) != 0)
      {
	puts ("join failed");
	exit (1);
      }

  return failures;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
