/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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

/* NOTE: this tests functionality beyond POSIX.  POSIX does not allow
   exit to be called more than once.  */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/wait.h>

/* The following interfaces are defined to be cancellation points but
   tests are not yet implemented:

     accept()              aio_suspend()      clock_nanosleep()
     close()               connect()          creat()
     fcntl()               fsync()            getmsg()
     getpmsg()             lockf()            mq_receive()
     mq_send()             mq_timedreceive()  mq_timedsend()
     msgrcv()              msgsnd()           msync()
                           open()             pause()
                           pread()            pthread_cond_timedwait()
     pthread_cond_wait()   pthread_join()     pthread_testcancel()
     putmsg()              putpmsg()          pwrite()
                                              recv()
     recvfrom()            recvmsg()
     sem_timedwait()       sem_wait()         send()
     sendmsg()             sendto()           sigpause()
     sigsuspend()          sigtimedwait()     sigwait()
     sigwaitinfo()                            system()
     tcdrain()

   Since STREAMS are not supported in the standard Linux kernel there
   is no need to test the STREAMS related functions.
*/

/* Pipe descriptors.  */
static int fds[2];

/* Often used barrier for two threads.  */
static pthread_barrier_t b2;


static void *
tf_read  (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  char buf[100];
  ssize_t s = read (fds[0], buf, sizeof (buf));

  printf ("%s: read returns with %zd\n", __FUNCTION__, s);

  exit (1);
}


static void *
tf_readv  (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  char buf[100];
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  ssize_t s = readv (fds[0], iov, 1);

  printf ("%s: readv returns with %zd\n", __FUNCTION__, s);

  exit (1);
}


static void *
tf_write  (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  char buf[100000];
  memset (buf, '\0', sizeof (buf));
  ssize_t s = write (fds[1], buf, sizeof (buf));

  printf ("%s: write returns with %zd\n", __FUNCTION__, s);

  exit (1);
}


static void *
tf_writev  (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  char buf[100000];
  memset (buf, '\0', sizeof (buf));
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  ssize_t s = writev (fds[1], iov, 1);

  printf ("%s: writev returns with %zd\n", __FUNCTION__, s);

  exit (1);
}


static void *
tf_sleep (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  sleep (1000000);

  printf ("%s: sleep returns\n", __FUNCTION__);

  exit (1);
}


static void *
tf_usleep (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  usleep ((useconds_t) ULONG_MAX);

  printf ("%s: usleep returns\n", __FUNCTION__);

  exit (1);
}


static void *
tf_nanosleep (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  struct timespec ts = { .tv_sec = 10000000, .tv_nsec = 0 };
  while (nanosleep (&ts, &ts) != 0)
    continue;

  printf ("%s: nanosleep returns\n", __FUNCTION__);

  exit (1);
}


static void *
tf_select (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  fd_set rfs;
  FD_ZERO (&rfs);
  FD_SET (fds[0], &rfs);

  int s = select (fds[0] + 1, &rfs, NULL, NULL, NULL);

  printf ("%s: select returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static void *
tf_pselect (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  fd_set rfs;
  FD_ZERO (&rfs);
  FD_SET (fds[0], &rfs);

  int s = pselect (fds[0] + 1, &rfs, NULL, NULL, NULL, NULL);

  printf ("%s: pselect returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static void *
tf_poll (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  struct pollfd rfs[1] = { [0] = { .fd = fds[0], .events = POLLIN } };

  int s = poll (rfs, 1, -1);

  printf ("%s: poll returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static void *
tf_wait (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
      /* Make the program disappear after a while.  */
      sleep (10);
      exit (0);
    }

  int s = wait (NULL);

  printf ("%s: wait returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static void *
tf_waitpid (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
      /* Make the program disappear after a while.  */
      sleep (10);
      exit (0);
    }

  int s = waitpid (-1, NULL, 0);

  printf ("%s: waitpid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static void *
tf_waitid (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
      /* Make the program disappear after a while.  */
      sleep (10);
      exit (0);
    }

#ifndef WEXITED
# define WEXITED 0
#endif
  siginfo_t si;
  int s = waitid (P_PID, pid, &si, WEXITED);

  printf ("%s: waitid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static struct
{
  void *(*tf) (void *);
  int nb;
} tests[] =
{
  { tf_read, 2 },
  { tf_readv, 2 },
  { tf_select, 2 },
  { tf_pselect, 2 },
  { tf_poll, 2 },
  { tf_write, 2 },
  { tf_writev, 2},
  { tf_sleep, 2 },
  { tf_usleep, 2 },
  { tf_nanosleep, 2 },
  { tf_wait, 2 },
  { tf_waitid, 2 },
  { tf_waitpid, 2 },
};
#define ntest_tf (sizeof (tests) / sizeof (tests[0]))


static int
do_test (void)
{
  if (pipe (fds) != 0)
    {
      puts ("pipe failed");
      exit (1);
    }

  int cnt;
  for (cnt = 0; cnt < ntest_tf; ++cnt)
    {
      printf ("round %d\n", cnt);

      if (pthread_barrier_init (&b2, NULL, tests[cnt].nb) != 0)
	{
	  puts ("b2 init failed");
	  exit (1);
	}

      pthread_t th;
      if (pthread_create (&th, NULL, tests[cnt].tf, NULL) != 0)
	{
	  printf ("create for round %d test failed\n", cnt);
	  exit (1);
	}

      puts ("barrier waits");

      int r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}

      puts ("nanosleep delay");

      struct timespec ts = { .tv_sec = 0, .tv_nsec = 100000000 };
      while (nanosleep (&ts, &ts) != 0)
	continue;

      if (pthread_cancel (th) != 0)
	{
	  printf ("cancel in round %d failed\n", cnt);
	  exit (1);
	}

      void *status;
      if (pthread_join (th, &status) != 0)
	{
	  printf ("join in round %d failed\n", cnt);
	  exit (1);
	}
      if (status != PTHREAD_CANCELED)
	{
	  printf ("thread in round %d not canceled\n", cnt);
	  exit (1);
	}
      printf ("test %d successful\n", cnt);

      if (pthread_barrier_destroy (&b2) != 0)
	{
	  puts ("barrier_destroy failed");
	  exit (1);
	}
    }

  return 0;
}

#define TIMEOUT 60
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
