/* This is a test of the special shutdown that occurs
   when all threads, including the main one, call
   pthread_exit(). It demonstrates that atexit
   handlers are properly called, and that the
   output is properly flushed even when stdout is
   redirected to a file, and therefore fully buffered. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 20		/* number of threads */

static void *
thread (void *arg)
{
  printf ("thread terminating\n");
  return 0;
}

static void
cleanup (void)
{
  printf ("atexit handler called\n");
}

int
main (void)
{
  int i;

  atexit (cleanup);

  for (i = 0; i < NTHREADS; i++)
    {
      pthread_t id;
      if (pthread_create (&id, 0, thread, 0) != 0)
	{
	  fprintf (stderr, "pthread_create failed\n");
	  abort ();
	}
    }

  pthread_exit (0);
}
