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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>

#include "pthreadP.h"


/* The following interfaces are defined to be cancellation points but
   tests are not yet implemented:

     aio_suspend()         clock_nanosleep()  close()
     connect()             creat()            fsync()
     msgrcv()              msgsnd()           msync()
     open()                pread()            pwrite()
     sendmsg()             sendto()
     tcdrain()

   Since STREAMS are not supported in the standard Linux kernel there
   is no need to test the STREAMS related functions.  This affects
     getmsg()              getpmsg()          putmsg()
     putpmsg()

   lockf() and fcntl() are tested in tst-cancel16.

   pthread_join() is tested in tst-join5.

   pthread_testcancel()'s only purpose is to allow cancellation.  This
   is tested in several places.

   sem_wait() and sem_timedwait() are checked in tst-cancel1[2345] tests.

   POSIX message queues aren't implemented yet.  This affects
     mq_receive()    mq_send()   mq_timedreceive()  mq_timedsend()
*/

/* Pipe descriptors.  */
static int fds[2];

/* Temporary file descriptor, to be closed after each round.  */
static int tempfd = -1;
static int tempfd2 = -1;
/* Name of temporary file to be removed after each round.  */
static char *tempfname;

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
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[0];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char buf[100];
  ssize_t s = read (fd, buf, sizeof (buf));

  printf ("%s: read returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_readv  (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[0];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char buf[100];
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  ssize_t s = readv (fd, iov, 1);

  printf ("%s: readv returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_write  (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[1];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char buf[100000];
  memset (buf, '\0', sizeof (buf));
  ssize_t s = write (fd, buf, sizeof (buf));

  printf ("%s: write returns with %zd\n", __FUNCTION__, s);

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_writev  (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[1];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char buf[100000];
  memset (buf, '\0', sizeof (buf));
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  ssize_t s = writev (fd, iov, 1);

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

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  sleep (arg == NULL ? 1000000 : 0);

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

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  usleep (arg == NULL ? (useconds_t) ULONG_MAX : 0);

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

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  struct timespec ts = { .tv_sec = arg == NULL ? 10000000 : 0, .tv_nsec = 0 };
  while (nanosleep (&ts, &ts) != 0)
    continue;

  pthread_cleanup_pop (0);

  printf ("%s: nanosleep returns\n", __FUNCTION__);

  exit (1);
}


static void *
tf_select (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[0];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  fd_set rfs;
  FD_ZERO (&rfs);
  FD_SET (fd, &rfs);

  pthread_cleanup_push (cl, NULL);

  int s = select (fd + 1, &rfs, NULL, NULL, NULL);

  printf ("%s: select returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_pselect (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[0];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  fd_set rfs;
  FD_ZERO (&rfs);
  FD_SET (fd, &rfs);

  pthread_cleanup_push (cl, NULL);

  int s = pselect (fd + 1, &rfs, NULL, NULL, NULL, NULL);

  printf ("%s: pselect returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_poll (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    fd = fds[0];
  else
    {
      char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
      tempfd = fd = mkstemp (fname);
      if (fd == -1)
	printf ("%s: mkstemp failed\n", __FUNCTION__);
      unlink (fname);

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  struct pollfd rfs[1] = { [0] = { .fd = fd, .events = POLLIN } };

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
  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
      /* Make the program disappear after a while.  */
      if (arg == NULL)
	sleep (10);
      exit (0);
    }

  int r;
  if (arg != NULL)
    {
      struct timespec  ts = { .tv_sec = 0, .tv_nsec = 100000000 };
      while (nanosleep (&ts, &ts) != 0)
	continue;

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
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

  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
      /* Make the program disappear after a while.  */
      if (arg == NULL)
	sleep (10);
      exit (0);
    }

  int r;
  if (arg != NULL)
    {
      struct timespec  ts = { .tv_sec = 0, .tv_nsec = 100000000 };
      while (nanosleep (&ts, &ts) != 0)
	continue;

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
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
  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
      /* Make the program disappear after a while.  */
      if (arg == NULL)
	sleep (10);
      exit (0);
    }

  int r;
  if (arg != NULL)
    {
      struct timespec  ts = { .tv_sec = 0, .tv_nsec = 100000000 };
      while (nanosleep (&ts, &ts) != 0)
	continue;

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  siginfo_t si;
  int s = waitid (P_PID, pid, &si, 0);

  printf ("%s: waitid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_sigpause (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  /* Just for fun block the cancellation signal.  We need to use
     __xpg_sigpause since otherwise we will get the BSD version.  */
  __xpg_sigpause (SIGCANCEL);

  pthread_cleanup_pop (0);

  printf ("%s: sigpause returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_sigsuspend (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  /* Just for fun block all signals.  */
  sigset_t mask;
  sigfillset (&mask);
  sigsuspend (&mask);

  pthread_cleanup_pop (0);

  printf ("%s: sigsuspend returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_sigwait (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  /* Block SIGUSR1.  */
  sigset_t mask;
  sigaddset (&mask, SIGUSR1);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL) != 0)
    {
      printf ("%s: pthread_sigmask failed\n", __FUNCTION__);
      exit (1);
    }

  /* Wait for SIGUSR1.  */
  int sig;
  sigwait (&mask, &sig);

  printf ("%s: sigwait returned with signal %d\n", __FUNCTION__, sig);

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_sigwaitinfo (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  /* Block SIGUSR1.  */
  sigset_t mask;
  sigaddset (&mask, SIGUSR1);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL) != 0)
    {
      printf ("%s: pthread_sigmask failed\n", __FUNCTION__);
      exit (1);
    }

  /* Wait for SIGUSR1.  */
  siginfo_t info;
  sigwaitinfo (&mask, &info);

  printf ("%s: sigwaitinfo returned with signal %d\n", __FUNCTION__,
	  info.si_signo);

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_sigtimedwait (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  /* Block SIGUSR1.  */
  sigset_t mask;
  sigaddset (&mask, SIGUSR1);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL) != 0)
    {
      printf ("%s: pthread_sigmask failed\n", __FUNCTION__);
      exit (1);
    }

  /* Wait for SIGUSR1.  */
  siginfo_t info;
  struct timespec ts = { .tv_sec = 60, .tv_nsec = 0 };
  sigtimedwait (&mask, &info, &ts);

  printf ("%s: sigtimedwait returned with signal %d\n", __FUNCTION__,
	  info.si_signo);

  pthread_cleanup_pop (0);

  exit (1);
}


static void *
tf_pause (void *arg)
{
  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  pause ();

  pthread_cleanup_pop (0);

  printf ("%s: pause returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_accept (void *arg)
{
  struct sockaddr_un sun;
  /* To test a non-blocking accept call we make the call file by using
     a datagrame socket.  */
  int pf = arg == NULL ? SOCK_STREAM : SOCK_DGRAM;

  tempfd = socket (AF_UNIX, pf, 0);
  if (tempfd == -1)
    {
      printf ("%s: socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int tries = 0;
  do
    {
      if (++tries > 10)
	{
	  printf ("%s: too many unsuccessful bind calls\n", __FUNCTION__);
	}

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-XXXXXX");
      if (mktemp (sun.sun_path) == NULL)
	{
	  printf ("%s: cannot generate temp file name\n", __FUNCTION__);
	  exit (1);
	}

      sun.sun_family = AF_UNIX;
    }
  while (bind (tempfd, (struct sockaddr *) &sun,
	       offsetof (struct sockaddr_un, sun_path)
	       + strlen (sun.sun_path) + 1) != 0);

  unlink (sun.sun_path);

  listen (tempfd, 5);

  socklen_t len = sizeof (sun);

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  accept (tempfd, (struct sockaddr *) &sun, &len);

  pthread_cleanup_pop (0);

  printf ("%s: accept returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_send (void *arg)
{
  struct sockaddr_un sun;

  tempfd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (tempfd == -1)
    {
      printf ("%s: first socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int tries = 0;
  do
    {
      if (++tries > 10)
	{
	  printf ("%s: too many unsuccessful bind calls\n", __FUNCTION__);
	}

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-XXXXXX");
      if (mktemp (sun.sun_path) == NULL)
	{
	  printf ("%s: cannot generate temp file name\n", __FUNCTION__);
	  exit (1);
	}

      sun.sun_family = AF_UNIX;
    }
  while (bind (tempfd, (struct sockaddr *) &sun,
	       offsetof (struct sockaddr_un, sun_path)
	       + strlen (sun.sun_path) + 1) != 0);

  listen (tempfd, 5);

  tempfd2 = socket (AF_UNIX, SOCK_STREAM, 0);
  if (tempfd2 == -1)
    {
      printf ("%s: second socket call failed\n", __FUNCTION__);
      exit (1);
    }

  if (connect (tempfd2, (struct sockaddr *) &sun, sizeof (sun)) != 0)
    {
      printf ("%s: connect failed\n", __FUNCTION__);
      exit(1);
    }

  unlink (sun.sun_path);

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  /* Very large block, so that the send call blocks.  */
  char mem[700000];

  send (tempfd2, mem, arg == NULL ? sizeof (mem) : 1, 0);

  pthread_cleanup_pop (0);

  printf ("%s: send returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_recv (void *arg)
{
  struct sockaddr_un sun;

  tempfd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (tempfd == -1)
    {
      printf ("%s: first socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int tries = 0;
  do
    {
      if (++tries > 10)
	{
	  printf ("%s: too many unsuccessful bind calls\n", __FUNCTION__);
	}

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-XXXXXX");
      if (mktemp (sun.sun_path) == NULL)
	{
	  printf ("%s: cannot generate temp file name\n", __FUNCTION__);
	  exit (1);
	}

      sun.sun_family = AF_UNIX;
    }
  while (bind (tempfd, (struct sockaddr *) &sun,
	       offsetof (struct sockaddr_un, sun_path)
	       + strlen (sun.sun_path) + 1) != 0);

  listen (tempfd, 5);

  tempfd2 = socket (AF_UNIX, SOCK_STREAM, 0);
  if (tempfd2 == -1)
    {
      printf ("%s: second socket call failed\n", __FUNCTION__);
      exit (1);
    }

  if (connect (tempfd2, (struct sockaddr *) &sun, sizeof (sun)) != 0)
    {
      printf ("%s: connect failed\n", __FUNCTION__);
      exit(1);
    }

  unlink (sun.sun_path);

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  char mem[70];

  recv (tempfd2, mem, arg == NULL ? sizeof (mem) : 0, 0);

  pthread_cleanup_pop (0);

  printf ("%s: recv returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_recvfrom (void *arg)
{
  struct sockaddr_un sun;

  tempfd = socket (AF_UNIX, SOCK_DGRAM, 0);
  if (tempfd == -1)
    {
      printf ("%s: first socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int tries = 0;
  do
    {
      if (++tries > 10)
	{
	  printf ("%s: too many unsuccessful bind calls\n", __FUNCTION__);
	}

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-XXXXXX");
      if (mktemp (sun.sun_path) == NULL)
	{
	  printf ("%s: cannot generate temp file name\n", __FUNCTION__);
	  exit (1);
	}

      sun.sun_family = AF_UNIX;
    }
  while (bind (tempfd, (struct sockaddr *) &sun,
	       offsetof (struct sockaddr_un, sun_path)
	       + strlen (sun.sun_path) + 1) != 0);

  tempfname = strdup (sun.sun_path);

  tempfd2 = socket (AF_UNIX, SOCK_DGRAM, 0);
  if (tempfd2 == -1)
    {
      printf ("%s: second socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  char mem[70];
  socklen_t len = sizeof (sun);

  recvfrom (tempfd2, mem, arg == NULL ? sizeof (mem) : 0, 0,
	    (struct sockaddr *) &sun, &len);

  pthread_cleanup_pop (0);

  printf ("%s: recvfrom returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_recvmsg (void *arg)
{
  struct sockaddr_un sun;

  tempfd = socket (AF_UNIX, SOCK_DGRAM, 0);
  if (tempfd == -1)
    {
      printf ("%s: first socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int tries = 0;
  do
    {
      if (++tries > 10)
	{
	  printf ("%s: too many unsuccessful bind calls\n", __FUNCTION__);
	}

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-XXXXXX");
      if (mktemp (sun.sun_path) == NULL)
	{
	  printf ("%s: cannot generate temp file name\n", __FUNCTION__);
	  exit (1);
	}

      sun.sun_family = AF_UNIX;
    }
  while (bind (tempfd, (struct sockaddr *) &sun,
	       offsetof (struct sockaddr_un, sun_path)
	       + strlen (sun.sun_path) + 1) != 0);

  tempfname = strdup (sun.sun_path);

  tempfd2 = socket (AF_UNIX, SOCK_DGRAM, 0);
  if (tempfd2 == -1)
    {
      printf ("%s: second socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  if (arg != NULL)
    {
      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
	  exit (1);
	}
    }

  pthread_cleanup_push (cl, NULL);

  char mem[70];
  struct iovec iov[1];
  iov[0].iov_base = mem;
  iov[0].iov_len = arg == NULL ? sizeof (mem) : 0;

  struct msghdr m;
  m.msg_name = &sun;
  m.msg_namelen = sizeof (sun);
  m.msg_iov = iov;
  m.msg_iovlen = 1;
  m.msg_control = NULL;
  m.msg_controllen = 0;

  recvmsg (tempfd2, &m, 0);

  pthread_cleanup_pop (0);

  printf ("%s: recvmsg returned\n", __FUNCTION__);

  exit (1);
}


static struct
{
  const char *name;
  void *(*tf) (void *);
  int nb;
  int only_early;
} tests[] =
{
#define ADD_TEST(name, nbar, early) { #name, tf_##name, nbar, early }
  ADD_TEST (read, 2, 0),
  ADD_TEST (readv, 2, 0),
  ADD_TEST (select, 2, 0),
  ADD_TEST (pselect, 2, 0),
  ADD_TEST (poll, 2, 0),
  ADD_TEST (write, 2, 0),
  ADD_TEST (writev, 2, 0),
  ADD_TEST (sleep, 2, 0),
  ADD_TEST (usleep, 2, 0),
  ADD_TEST (nanosleep, 2, 0),
  ADD_TEST (wait, 2, 0),
  ADD_TEST (waitid, 2, 0),
  ADD_TEST (waitpid, 2, 0),
  ADD_TEST (sigpause, 2, 0),
  ADD_TEST (sigsuspend, 2, 0),
  ADD_TEST (sigwait, 2, 0),
  ADD_TEST (sigwaitinfo, 2, 0),
  ADD_TEST (sigtimedwait, 2, 0),
  ADD_TEST (pause, 2, 0),
  ADD_TEST (accept, 2, 0),
  ADD_TEST (send, 2, 0),
  ADD_TEST (recv, 2, 0),
  ADD_TEST (recvfrom, 2, 0),
  ADD_TEST (recvmsg, 2, 0),
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
      if (tests[cnt].only_early)
	continue;

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

      printf ("in-time cancel test of '%s' successful\n", tests[cnt].name);

      if (tempfd != -1)
	{
	  close (tempfd);
	  tempfd = -1;
	}
      if (tempfd2 != -1)
	{
	  close (tempfd2);
	  tempfd2 = -1;
	}
      free (tempfname);
      tempfname = NULL;
    }

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
      if (pthread_create (&th, NULL, tests[cnt].tf, (void *) 1l) != 0)
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

      if (pthread_cancel (th) != 0)
	{
	  printf ("cancel for '%s' failed\n", tests[cnt].name);
	  result = 1;
	  continue;
	}

      r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed\n", __FUNCTION__);
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

      printf ("early cancel test of '%s' successful\n", tests[cnt].name);

      if (tempfd != -1)
	{
	  close (tempfd);
	  tempfd = -1;
	}
      if (tempfd2 != -1)
	{
	  close (tempfd2);
	  tempfd2 = -1;
	}
      free (tempfname);
      tempfname = NULL;
    }

  return result;
}

#define TIMEOUT 60
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
