#include <errno.h>
#include <error.h>
#include <signal.h>
#include <stdlib.h>

volatile int count;

void
sh (int sig)
{
  ++count;
}

int
main (void)
{
  struct sigaction sa;
  sa.sa_handler = sh;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction (SIGUSR1, &sa, NULL) < 0)
    {
      printf ("sigaction failed: %m\n");
      exit (1);
    }
  if (raise (SIGUSR1) < 0)
    {
      printf ("first raise failed: %m\n");
      exit (1);
    }
  if (raise (SIGUSR1) < 0)
    {
      printf ("second raise failed: %m\n");
      exit (1);
    }
  if (count != 2)
    {
      printf ("signal handler not called 2 times\n");
      exit (1);
    }
  exit (0);
}
