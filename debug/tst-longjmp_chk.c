#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static jmp_buf b;


static void
__attribute__ ((noinline))
f (void)
{
  char buf[1000];
  asm volatile ("" : "=m" (buf));

  if (setjmp (b) != 0)
    {
      puts ("second longjmp succeeded");
      exit (1);
    }
}


static bool expected_to_fail;


static void
handler (int sig)
{
  if (expected_to_fail)
    _exit (0);
  else
    {
      static const char msg[] = "unexpected longjmp failure\n";
      TEMP_FAILURE_RETRY (write (STDOUT_FILENO, msg, sizeof (msg) - 1));
      _exit (1);
    }
}


int
main (void)
{
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);

  sigaction (SIGABRT, &sa, NULL);

  /* Avoid all the buffer overflow messages on stderr.  */
  int fd = open (_PATH_DEVNULL, O_WRONLY);
  if (fd == -1)
    close (STDERR_FILENO);
  else
    {
      dup2 (fd, STDERR_FILENO);
      close (fd);
    }
  setenv ("LIBC_FATAL_STDERR_", "1", 1);


  expected_to_fail = false;

  if (setjmp (b) == 0)
    {
      longjmp (b, 1);
      /* NOTREACHED */
      printf ("first longjmp returned\n");
      return 1;
    }


  expected_to_fail = true;

  f ();
  longjmp (b, 1);

  puts ("second longjmp returned");
  return 1;
}
