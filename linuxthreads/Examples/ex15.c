#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

static void *worker (void *dummy) __attribute__ ((__noreturn__));

static void *
worker (void *dummy)
{
  exit (26);
}

#define TEST_FUNCTION do_test ()
#define TIMEOUT 10
static int
do_test (void)
{
  pthread_t th;
  pid_t pid;
  int status;

  switch ((pid = fork ()))
    {
    case -1:
      puts ("Could not fork");
      exit (1);
    case 0:
      if (pthread_create(&th, NULL, worker, NULL) != 0)
	{
	  puts ("Failed to start thread");
	  exit (1);
	}
      for (;;);
      exit (1);
    default:
      break;
    }

  if (waitpid (pid, &status, 0) != pid)
    {
      puts ("waitpid failed");
      exit (1);
    }

  if (!WIFEXITED (status) || WEXITSTATUS (status) != 26)
    {
      printf ("Wrong exit code %d\n", status);
      exit (1);
    }

  puts ("All OK");
  return 0;
}

#include "../../test-skeleton.c"
