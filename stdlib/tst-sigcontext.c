/* Copyright (C) 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maciej W. Rozycki <macro@codesourcery.com>.

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
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>

static volatile ucontext_t ctx[3];
static volatile int flags[3];
static volatile int oflag;
static volatile int flag;

static volatile int sig_errno_set;
static volatile int sig_errno;
static volatile int sig_ready;

static int was_in_f1;
static int was_in_f2;

static char st2[32768];

static long long int max;
static volatile long long int count;

/* These have to be run with the caller's frame.  */
static inline int sig_getcontext (int) __attribute__ ((always_inline));
static inline int sig_swapcontext (int, int) __attribute__ ((always_inline));

static void
sig_gethandler (int signo, siginfo_t *info, void *context)
{
  ucontext_t *ucp = context;

  if (! sig_ready)
    {
      alarm (1);
      return;
    }

  ctx[flag] = *ucp;
  flags[flag] = 1;

  sig_errno_set = 1;
  sig_errno = 0;
}

static void
sig_swaphandler (int signo, siginfo_t *info, void *context)
{
  ucontext_t *oucp = context;
  ucontext_t *ucp = (ucontext_t *) (ctx + flag);

  if (! sig_ready)
    {
      alarm (1);
      return;
    }

  ctx[oflag] = *oucp;
  flags[oflag] = 1;

  sig_errno_set = 1;
  sig_errno = setcontext (ucp);
}

static int
sig_getcontext (int uci)
{
  struct sigaction sa;

  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sig_gethandler;
  if (sigaction (SIGALRM, &sa, NULL) < 0)
    {
      printf ("%s: sigaction ALRM failed: %m\n", __FUNCTION__);
      exit (1);
    }

  flag = uci;
  sig_errno_set = 0;
  sig_ready = 0;
  alarm (1);

  /* Need to get the signal in this frame.  */
  sig_ready = 1;
  for (count = max; count > 0; count--)
    if (sig_errno_set)
      break;

  if (! sig_errno_set)
    {
      printf ("%s: ALRM signal not received\n", __FUNCTION__);
      exit (1);
    }

  return 0;
}

static int
sig_swapcontext (int ouci, int uci)
{
  struct sigaction sa;

  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sig_swaphandler;
  if (sigaction (SIGALRM, &sa, NULL) < 0)
    {
      printf ("%s: sigaction ALRM failed: %m\n", __FUNCTION__);
      exit (1);
    }

  flag = uci;
  oflag = ouci;
  sig_errno_set = 0;
  sig_ready = 0;
  alarm (1);

  /* Need to get the signal in this frame.  */
  sig_ready = 1;
  for (count = max; count > 0; count--)
    if (sig_errno_set)
      break;

  if (! sig_errno_set)
    {
      printf ("%s: ALRM signal not received\n", __FUNCTION__);
      exit (1);
    }

  return sig_errno;
}


static void
f1 (int a0, int a1, int a2, int a3)
{
  int status;

  printf ("start f1(a0=%x,a1=%x,a2=%x,a3=%x)\n", a0, a1, a2, a3);

  if (a0 != 1 || a1 != 2 || a2 != 3 || a3 != -4)
    {
      puts ("arg mismatch");
      exit (-1);
    }

  status = sig_swapcontext (1, 2);
  if (status != 0)
    {
      printf ("%s: swapping contexts failed: %s\n",
	      __FUNCTION__, strerror (status));
      exit (1);
    }
  puts ("finish f1");
  was_in_f1 = 1;
}

static void
f2 (void)
{
  int status;

  puts ("start f2");

  status = sig_swapcontext (2, 1);
  if (status != 0)
    {
      printf ("%s: swapping contexts failed: %s\n",
	      __FUNCTION__, strerror (status));
      exit (1);
    }
  puts ("finish f2");
  was_in_f2 = 1;
}


static int back_in_main;

static void
check_called (void)
{
  if (back_in_main == 0)
    {
      printf ("%s: program did not reach main again", __FUNCTION__);
      _exit (1);
    }
}


volatile int global;

int
main (void)
{
  char st1[32768];
  time_t t0, t1;
  int status;

  atexit (check_called);

  puts ("calibrating delay loop");
  for (max = 0x10000; max < LLONG_MAX / 2; max *= 2)
    {
      t0 = time (NULL);
      for (count = max; count > 0; count--);
      t1 = time (NULL);
      if (difftime (t1, t0) > 10.0)
	break;
    }

  puts ("making contexts");

  sig_getcontext (1);
  if (! flags[1])
    {
      printf ("%s: context not retrieved\n", __FUNCTION__);
      exit (1);
    }

  /* Play some tricks with this context.  */
  if (++global == 1)
    if (setcontext ((ucontext_t *) &ctx[1]) != 0)
      {
	if (errno == ENOSYS)
	  {
	    back_in_main = 1;
	    exit (0);
	  }
	printf ("%s: setcontext: %m\n", __FUNCTION__);
	exit (1);
      }
  if (global != 2)
    {
      printf ("%s: 'global' not incremented twice\n", __FUNCTION__);
      exit (1);
    }

  ctx[1].uc_stack.ss_sp = st1;
  ctx[1].uc_stack.ss_size = sizeof st1;
  ctx[1].uc_link = (ucontext_t *) &ctx[0];
  {
    ucontext_t tempctx = ctx[1];
    makecontext ((ucontext_t *) &ctx[1], (void (*) (void)) f1, 4, 1, 2, 3, -4);

    /* Without this check, a stub makecontext can make us spin forever.  */
    if (memcmp (&tempctx, (void *) &ctx[1], sizeof ctx[1]) == 0)
      {
        puts ("makecontext was a no-op, presuming not implemented");
        return 0;
      }
  }

  sig_getcontext (2);
  if (! flags[2])
    {
      printf ("%s: second context not retrieved\n", __FUNCTION__);
      exit (1);
    }

  ctx[2].uc_stack.ss_sp = st2;
  ctx[2].uc_stack.ss_size = sizeof st2;
  ctx[2].uc_link = (ucontext_t *) &ctx[1];
  makecontext ((ucontext_t *) &ctx[2], f2, 0);

  puts ("swapping contexts");
  status = sig_swapcontext (0, 2);
  if (status != 0)
    {
      printf ("%s: swapping contexts failed: %s\n",
	      __FUNCTION__, strerror (status));
      exit (1);
    }
  puts ("back at main program");
  back_in_main = 1;

  if (was_in_f1 == 0)
    {
      puts ("didn't reach f1");
      exit (1);
    }
  if (was_in_f2 == 0)
    {
      puts ("didn't reach f2");
      exit (1);
    }

  puts ("test succeeded");
  return 0;
}
