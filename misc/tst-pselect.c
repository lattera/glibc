#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <stdlib.h>


static volatile int handler_called;

static void
handler (int sig)
{
  handler_called = 1;
}


static int
do_test (void)
{
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);

  if (sigaction (SIGUSR1, &sa, NULL) != 0)
    {
      puts ("sigaction failed");
      return 1;
    }

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_NOCLDWAIT;

  if (sigaction (SIGCHLD, &sa, NULL) != 0)
    {
      puts ("2nd sigaction failed");
      return 1;
    }

  sigset_t ss_usr1;
  sigemptyset (&ss_usr1);
  sigaddset (&ss_usr1, SIGUSR1);
  if (sigprocmask (SIG_BLOCK, &ss_usr1, NULL) != 0)
    {
      puts ("sigprocmask failed");
      return 1;
    }

  int fds[2][2];

  if (pipe (fds[0]) != 0 || pipe (fds[1]) != 0)
    {
      puts ("pipe failed");
      return 1;
    }

  fd_set rfds;
  FD_ZERO (&rfds);

  sigset_t ss;
  sigprocmask (SIG_SETMASK, NULL, &ss);
  sigdelset (&ss, SIGUSR1);

  struct timespec to = { .tv_sec = 0, .tv_nsec = 500000000 };

  pid_t parent = getpid ();
  pid_t p = fork ();
  if (p == 0)
    {
      close (fds[0][1]);
      close (fds[1][0]);

      FD_SET (fds[0][0], &rfds);

      int e;
      do
	{
	  if (getppid () != parent)
	    exit (2);

	  errno = 0;
	  e = pselect (fds[0][0] + 1, &rfds, NULL, NULL, &to, &ss);
	}
      while (e == 0);

      if (e != -1)
	{
	  puts ("child: pselect did not fail");
	  return 0;
	}
      if (errno != EINTR)
	{
	  puts ("child: pselect did not set errno to EINTR");
	  return 0;
	}

      TEMP_FAILURE_RETRY (write (fds[1][1], "foo", 3));

      exit (0);
    }

  close (fds[0][0]);
  close (fds[1][1]);

  FD_SET (fds[1][0], &rfds);

  kill (p, SIGUSR1);

  int e = pselect (fds[1][0] + 1, &rfds, NULL, NULL, NULL, &ss);
  if (e == -1)
    {
      puts ("parent: pselect failed");
      return 1;
    }
  if (e != 1)
    {
      puts ("parent: pselect did not report readable fd");
      return 1;
    }
  if (!FD_ISSET (fds[1][0], &rfds))
    {
      puts ("parent: pselect reports wrong fd");
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
