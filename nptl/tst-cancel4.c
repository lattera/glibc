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
                           pread()
			   pthread_join()     pthread_testcancel()
     putmsg()              putpmsg()          pwrite()
                                              recv()
     recvfrom()            recvmsg()
     sem_timedwait()       sem_wait()         send()
     sendmsg()             sendto()           sigpause()
     sigsuspend()          sigtimedwait()     sigwait()
     sigwaitinfo()
     tcdrain()

   Since STREAMS are not supported in the standard Linux kernel there
   is no need to test the STREAMS related functions.
*/

/* Pipe descriptors.  */
static int fds[2];

/* Often used barrier for two threads.  */
static pthread_barrier_t b2;


/* Cleanup handling test.  */
static int cl_called;

static void
cl (void *arg)
{
  ++cl_called;
}


static void *
tf_read  (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char buf[100];
  ssize_t s = read (fds[0], buf, sizeof (buf));

  printf ("%s: read returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  char buf[100];
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  ssize_t s = readv (fds[0], iov, 1);

  printf ("%s: readv returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  char buf[100000];
  memset (buf, '\0', sizeof (buf));
  ssize_t s = write (fds[1], buf, sizeof (buf));

  printf ("%s: write returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  char buf[100000];
  memset (buf, '\0', sizeof (buf));
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  ssize_t s = writev (fds[1], iov, 1);

  printf ("%s: writev returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  sleep (1000000);

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  usleep ((useconds_t) ULONG_MAX);

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  struct timespec ts = { .tv_sec = 10000000, .tv_nsec = 0 };
  while (nanosleep (&ts, &ts) != 0)
    continue;

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  int s = select (fds[0] + 1, &rfs, NULL, NULL, NULL);

  printf ("%s: select returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  int s = pselect (fds[0] + 1, &rfs, NULL, NULL, NULL, NULL);

  printf ("%s: pselect returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  int s = poll (rfs, 1, -1);

  printf ("%s: poll returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  int s = wait (NULL);

  printf ("%s: wait returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  int s = waitpid (-1, NULL, 0);

  printf ("%s: waitpid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

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

  pthread_cleanup_push (cl, NULL);

  siginfo_t si;
  int s = waitid (P_PID, pid, &si, 0);

  printf ("%s: waitid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

  exit (1);
}


static struct
{
  const char *name;
  void *(*tf) (void *);
  int nb;
} tests[] =
{
#define ADD_TEST(name, nbar) { #name, tf_##name, nbar }
  ADD_TEST (read, 2),
  ADD_TEST (readv, 2),
  ADD_TEST (select, 2),
  ADD_TEST (pselect, 2),
  ADD_TEST (poll, 2),
  ADD_TEST (write, 2),
  ADD_TEST (writev, 2),
  ADD_TEST (sleep, 2),
  ADD_TEST (usleep, 2),
  ADD_TEST (nanosleep, 2),
  ADD_TEST (wait, 2),
  ADD_TEST (waitid, 2),
  ADD_TEST (waitpid, 2),
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

  int result = 0;
  size_t cnt;
  for (cnt = 0; cnt < ntest_tf; ++cnt)
    {
      if (pthread_barrier_init (&b2, NULL, tests[cnt].nb) != 0)
	{
	  puts ("b2 init failed");
	  exit (1);
	}

      /* Reset the counter for the cleanup handler.  */
      cl_called = 0;

      pthread_t th;
      if (pthread_create (&th, NULL, tests[cnt].tf, NULL) != 0)
	{
	  printf ("create for '%s' test failed\n", tests[cnt].name);
	  result = 1;
	  continue;
	}

      int r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  result = 1;
	  continue;
	}

      struct timespec  ts = { .tv_sec = 0, .tv_nsec = 100000000 };
      while (nanosleep (&ts, &ts) != 0)
	continue;

      if (pthread_cancel (th) != 0)
	{
	  printf ("cancel for '%s' failed\n", tests[cnt].name);
	  result = 1;
	  continue;
	}

      void *status;
      if (pthread_join (th, &status) != 0)
	{
	  printf ("join for '%s' failed\n", tests[cnt].name);
	  result = 1;
	  continue;
	}
      if (status != PTHREAD_CANCELED)
	{
	  printf ("thread for '%s' not canceled\n", tests[cnt].name);
	  result = 1;
	  continue;
	}

      if (pthread_barrier_destroy (&b2) != 0)
	{
	  puts ("barrier_destroy failed");
	  result = 1;
	  continue;
	}

      if (cl_called == 0)
	{
	  printf ("cleanup handler not called for '%s'\n", tests[cnt].name);
	  result = 1;
	  continue;
	}
      if (cl_called > 1)
	{
	  printf ("cleanup handler called more than once for '%s'\n",
		  tests[cnt].name);
	  result = 1;
	  continue;
	}

      printf ("test of '%s' successful\n", tests[cnt].name);
    }

  return result;
}

#define TIMEOUT 60
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
