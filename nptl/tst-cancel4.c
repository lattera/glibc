/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* NOTE: this tests functionality beyond POSIX.  POSIX does not allow
   exit to be called more than once.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/wait.h>


/* Since STREAMS are not supported in the standard Linux kernel and
   there we don't advertise STREAMS as supported is no need to test
   the STREAMS related functions.  This affects
     getmsg()              getpmsg()          putmsg()
     putpmsg()

   lockf() and fcntl() are tested in tst-cancel16.

   pthread_join() is tested in tst-join5.

   pthread_testcancel()'s only purpose is to allow cancellation.  This
   is tested in several places.

   sem_wait() and sem_timedwait() are checked in tst-cancel1[2345] tests.

   mq_send(), mq_timedsend(), mq_receive() and mq_timedreceive() are checked
   in tst-mqueue8{,x} tests.

   aio_suspend() is tested in tst-cancel17.

   clock_nanosleep() is tested in tst-cancel18.

   Linux sendmmsg and recvmmsg are checked in tst-cancel4_1.c and
   tst-cancel4_2.c respectively.
*/

#include "tst-cancel4-common.h"


#ifndef IPC_ADDVAL
# define IPC_ADDVAL 0
#endif


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

  ssize_t s;
  pthread_cleanup_push (cl, NULL);

  char buf[100];
  s = read (fd, buf, sizeof (buf));

  pthread_cleanup_pop (0);

  printf ("%s: read returns with %zd\n", __FUNCTION__, s);

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

  ssize_t s;
  pthread_cleanup_push (cl, NULL);

  char buf[100];
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  s = readv (fd, iov, 1);

  pthread_cleanup_pop (0);

  printf ("%s: readv returns with %zd\n", __FUNCTION__, s);

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

  ssize_t s;
  pthread_cleanup_push (cl, NULL);

  char buf[WRITE_BUFFER_SIZE];
  memset (buf, '\0', sizeof (buf));
  s = write (fd, buf, sizeof (buf));

  pthread_cleanup_pop (0);

  printf ("%s: write returns with %zd\n", __FUNCTION__, s);

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

  ssize_t s;
  pthread_cleanup_push (cl, NULL);

  char buf[WRITE_BUFFER_SIZE];
  memset (buf, '\0', sizeof (buf));
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  s = writev (fd, iov, 1);

  pthread_cleanup_pop (0);

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
  TEMP_FAILURE_RETRY (nanosleep (&ts, &ts));

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

  int s;
  pthread_cleanup_push (cl, NULL);

  s = select (fd + 1, &rfs, NULL, NULL, NULL);

  pthread_cleanup_pop (0);

  printf ("%s: select returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

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

  int s;
  pthread_cleanup_push (cl, NULL);

  s = pselect (fd + 1, &rfs, NULL, NULL, NULL, NULL);

  pthread_cleanup_pop (0);

  printf ("%s: pselect returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

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

  int s;
  pthread_cleanup_push (cl, NULL);

  s = poll (rfs, 1, -1);

  pthread_cleanup_pop (0);

  printf ("%s: poll returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

  exit (1);
}


static void *
tf_ppoll (void *arg)
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

  int s;
  pthread_cleanup_push (cl, NULL);

  s = ppoll (rfs, 1, NULL, NULL);

  pthread_cleanup_pop (0);

  printf ("%s: ppoll returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

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

  int s;
  pthread_cleanup_push (cl, NULL);

  s = wait (NULL);

  pthread_cleanup_pop (0);

  printf ("%s: wait returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

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

  int s;
 pthread_cleanup_push (cl, NULL);

  s = waitpid (-1, NULL, 0);

  pthread_cleanup_pop (0);

  printf ("%s: waitpid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

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

  int s;
  pthread_cleanup_push (cl, NULL);

#ifndef WEXITED
# define WEXITED 0
#endif
  siginfo_t si;
  s = waitid (P_PID, pid, &si, WEXITED);

  pthread_cleanup_pop (0);

  printf ("%s: waitid returns with %d (%s)\n", __FUNCTION__, s,
	  strerror (errno));

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

  sigpause (sigmask (SIGINT));

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

  /* Block SIGUSR1.  */
  sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL) != 0)
    {
      printf ("%s: pthread_sigmask failed\n", __FUNCTION__);
      exit (1);
    }

  int sig;
  pthread_cleanup_push (cl, NULL);

  /* Wait for SIGUSR1.  */
  sigwait (&mask, &sig);

  pthread_cleanup_pop (0);

  printf ("%s: sigwait returned with signal %d\n", __FUNCTION__, sig);

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

  /* Block SIGUSR1.  */
  sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL) != 0)
    {
      printf ("%s: pthread_sigmask failed\n", __FUNCTION__);
      exit (1);
    }

  siginfo_t info;
  pthread_cleanup_push (cl, NULL);

  /* Wait for SIGUSR1.  */
  sigwaitinfo (&mask, &info);

  pthread_cleanup_pop (0);

  printf ("%s: sigwaitinfo returned with signal %d\n", __FUNCTION__,
	  info.si_signo);

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

  /* Block SIGUSR1.  */
  sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);
  if (pthread_sigmask (SIG_BLOCK, &mask, NULL) != 0)
    {
      printf ("%s: pthread_sigmask failed\n", __FUNCTION__);
      exit (1);
    }

  /* Wait for SIGUSR1.  */
  siginfo_t info;
  struct timespec ts = { .tv_sec = 60, .tv_nsec = 0 };
  pthread_cleanup_push (cl, NULL);

  sigtimedwait (&mask, &info, &ts);

  pthread_cleanup_pop (0);

  printf ("%s: sigtimedwait returned with signal %d\n", __FUNCTION__,
	  info.si_signo);

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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-1-XXXXXX");
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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-2-XXXXXX");
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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-3-XXXXXX");
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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-4-XXXXXX");
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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-5-XXXXXX");
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

static void *
tf_open (void *arg)
{
  if (arg == NULL)
    {
      fifofd = mkfifo (fifoname, S_IWUSR | S_IRUSR);
      if (fifofd == -1)
	{
	  printf ("%s: mkfifo failed: %m\n", __func__);
	  exit (1);
	}
    }
  else
    {
      int r = pthread_barrier_wait (&b2);
      if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
	{
	  printf ("%s: barrier_wait failed: %m\n", __func__);
	  exit (1);
	}
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed: %m\n", __func__);
      exit (1);
    }

  pthread_cleanup_push (cl_fifo, NULL);

  open (arg ? "Makefile" : fifoname, O_RDONLY);

  pthread_cleanup_pop (0);

  printf ("%s: open returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_close (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which close()
    // blocks we can enable this test to run in both rounds.
    abort ();

  char fname[] = "/tmp/tst-cancel-fd-XXXXXX";
  tempfd = mkstemp (fname);
  if (tempfd == -1)
    {
      printf ("%s: mkstemp failed\n", __FUNCTION__);
      exit (1);
    }
  unlink (fname);

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  close (tempfd);

  pthread_cleanup_pop (0);

  printf ("%s: close returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_pread (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which pread()
    // blocks we can enable this test to run in both rounds.
    abort ();

  tempfd = open ("Makefile", O_RDONLY);
  if (tempfd == -1)
    {
      printf ("%s: cannot open Makefile\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char mem[10];
  pread (tempfd, mem, sizeof (mem), 0);

  pthread_cleanup_pop (0);

  printf ("%s: pread returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_pwrite (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which pwrite()
    // blocks we can enable this test to run in both rounds.
    abort ();

  char fname[] = "/tmp/tst-cancel4-fd-XXXXXX";
  tempfd = mkstemp (fname);
  if (tempfd == -1)
    {
      printf ("%s: mkstemp failed\n", __FUNCTION__);
      exit (1);
    }
  unlink (fname);

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char mem[10];
  pwrite (tempfd, mem, sizeof (mem), 0);

  pthread_cleanup_pop (0);

  printf ("%s: pwrite returned\n", __FUNCTION__);

  exit (1);
}

static void *
tf_preadv (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    /* XXX If somebody can provide a portable test case in which preadv
       blocks we can enable this test to run in both rounds.  */
    abort ();

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

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  ssize_t s;
  pthread_cleanup_push (cl, NULL);

  char buf[100];
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  s = preadv (fd, iov, 1, 0);

  pthread_cleanup_pop (0);

  printf ("%s: preadv returns with %zd\n", __FUNCTION__, s);

  exit (1);
}

static void *
tf_pwritev (void *arg)
{
  int fd;
  int r;

  if (arg == NULL)
    /* XXX If somebody can provide a portable test case in which pwritev
       blocks we can enable this test to run in both rounds.  */
    abort ();

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

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  ssize_t s;
  pthread_cleanup_push (cl, NULL);

  char buf[WRITE_BUFFER_SIZE];
  memset (buf, '\0', sizeof (buf));
  struct iovec iov[1] = { [0] = { .iov_base = buf, .iov_len = sizeof (buf) } };
  s = pwritev (fd, iov, 1, 0);

  pthread_cleanup_pop (0);

  printf ("%s: pwritev returns with %zd\n", __FUNCTION__, s);

  exit (1);
}

static void *
tf_fsync (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which fsync()
    // blocks we can enable this test to run in both rounds.
    abort ();

  tempfd = open ("Makefile", O_RDONLY);
  if (tempfd == -1)
    {
      printf ("%s: cannot open Makefile\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  fsync (tempfd);

  pthread_cleanup_pop (0);

  printf ("%s: fsync returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_fdatasync (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which fdatasync()
    // blocks we can enable this test to run in both rounds.
    abort ();

  tempfd = open ("Makefile", O_RDONLY);
  if (tempfd == -1)
    {
      printf ("%s: cannot open Makefile\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  fdatasync (tempfd);

  pthread_cleanup_pop (0);

  printf ("%s: fdatasync returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_msync (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which msync()
    // blocks we can enable this test to run in both rounds.
    abort ();

  tempfd = open ("Makefile", O_RDONLY);
  if (tempfd == -1)
    {
      printf ("%s: cannot open Makefile\n", __FUNCTION__);
      exit (1);
    }
  void *p = mmap (NULL, 10, PROT_READ, MAP_SHARED, tempfd, 0);
  if (p == MAP_FAILED)
    {
      printf ("%s: mmap failed\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  msync (p, 10, 0);

  pthread_cleanup_pop (0);

  printf ("%s: msync returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_sendto (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which sendto()
    // blocks we can enable this test to run in both rounds.
    abort ();

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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-6-XXXXXX");
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

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char mem[1];

  sendto (tempfd2, mem, arg == NULL ? sizeof (mem) : 1, 0,
	  (struct sockaddr *) &sun,
	  offsetof (struct sockaddr_un, sun_path) + strlen (sun.sun_path) + 1);

  pthread_cleanup_pop (0);

  printf ("%s: sendto returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_sendmsg (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which sendmsg()
    // blocks we can enable this test to run in both rounds.
    abort ();

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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-7-XXXXXX");
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

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char mem[1];
  struct iovec iov[1];
  iov[0].iov_base = mem;
  iov[0].iov_len = 1;

  struct msghdr m;
  m.msg_name = &sun;
  m.msg_namelen = (offsetof (struct sockaddr_un, sun_path)
		   + strlen (sun.sun_path) + 1);
  m.msg_iov = iov;
  m.msg_iovlen = 1;
  m.msg_control = NULL;
  m.msg_controllen = 0;

  sendmsg (tempfd2, &m, 0);

  pthread_cleanup_pop (0);

  printf ("%s: sendmsg returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_creat (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which sendmsg()
    // blocks we can enable this test to run in both rounds.
    abort ();

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  creat ("tmp/tst-cancel-4-should-not-exist", 0666);

  pthread_cleanup_pop (0);

  printf ("%s: creat returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_connect (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which connect()
    // blocks we can enable this test to run in both rounds.
    abort ();

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

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-2-XXXXXX");
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

  listen (tempfd, 5);

  tempfd2 = socket (AF_UNIX, SOCK_STREAM, 0);
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

  connect (tempfd2, (struct sockaddr *) &sun, sizeof (sun));

  pthread_cleanup_pop (0);

  printf ("%s: connect returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_tcdrain (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which tcdrain()
    // blocks we can enable this test to run in both rounds.
    abort ();

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

  /* Regardless of stderr being a terminal, the tcdrain call should be
     canceled.  */
  tcdrain (STDERR_FILENO);

  pthread_cleanup_pop (0);

  printf ("%s: tcdrain returned\n", __FUNCTION__);

  exit (1);
}


static void *
tf_msgrcv (void *arg)
{
  tempmsg = msgget (IPC_PRIVATE, 0666 | IPC_CREAT);
  if (tempmsg == -1)
    {
      printf ("%s: msgget failed: %s\n", __FUNCTION__, strerror (errno));
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

  ssize_t s;

  pthread_cleanup_push (cl, NULL);

  struct
  {
    long int type;
    char mem[10];
  } m;
  int randnr;
  /* We need a positive random number.  */
  do
    randnr = random () % 64000;
  while (randnr <= 0);
  do
    {
      errno = 0;
      s = msgrcv (tempmsg, (struct msgbuf *) &m, 10, randnr, 0);
    }
  while (errno == EIDRM || errno == EINTR);

  pthread_cleanup_pop (0);

  printf ("%s: msgrcv returned %zd with errno = %m\n", __FUNCTION__, s);

  msgctl (tempmsg, IPC_RMID, NULL);

  exit (1);
}


static void *
tf_msgsnd (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which msgsnd()
    // blocks we can enable this test to run in both rounds.
    abort ();

  tempmsg = msgget (IPC_PRIVATE, 0666 | IPC_CREAT);
  if (tempmsg == -1)
    {
      printf ("%s: msgget failed: %s\n", __FUNCTION__, strerror (errno));
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  struct
  {
    long int type;
    char mem[1];
  } m;
  /* We need a positive random number.  */
  do
    m.type = random () % 64000;
  while (m.type <= 0);
  msgsnd (tempmsg, (struct msgbuf *) &m, sizeof (m.mem), 0);

  pthread_cleanup_pop (0);

  printf ("%s: msgsnd returned\n", __FUNCTION__);

  msgctl (tempmsg, IPC_RMID, NULL);

  exit (1);
}


struct cancel_tests tests[] =
{
  ADD_TEST (read, 2, 0),
  ADD_TEST (readv, 2, 0),
  ADD_TEST (select, 2, 0),
  ADD_TEST (pselect, 2, 0),
  ADD_TEST (poll, 2, 0),
  ADD_TEST (ppoll, 2, 0),
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
  ADD_TEST (preadv, 2, 1),
  ADD_TEST (pwritev, 2, 1),
  ADD_TEST (open, 2, 1),
  ADD_TEST (close, 2, 1),
  ADD_TEST (pread, 2, 1),
  ADD_TEST (pwrite, 2, 1),
  ADD_TEST (fsync, 2, 1),
  ADD_TEST (fdatasync, 2, 1),
  ADD_TEST (msync, 2, 1),
  ADD_TEST (sendto, 2, 1),
  ADD_TEST (sendmsg, 2, 1),
  ADD_TEST (creat, 2, 1),
  ADD_TEST (connect, 2, 1),
  ADD_TEST (tcdrain, 2, 1),
  ADD_TEST (msgrcv, 2, 0),
  ADD_TEST (msgsnd, 2, 1),
};
#define ntest_tf (sizeof (tests) / sizeof (tests[0]))

#include "tst-cancel4-common.c"
