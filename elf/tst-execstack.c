/* Test program for making nonexecutable stacks executable
   on load of a DSO that requires executable stacks.  */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <error.h>

static void
print_maps (void)
{
#if 0
  char *cmd = NULL;
  asprintf (&cmd, "cat /proc/%d/maps", getpid ());
  system (cmd);
  free (cmd);
#endif
}

static void deeper (void (*f) (void));

#if USE_PTHREADS
# include <pthread.h>

static void *
tryme_thread (void *f)
{
  (*((void (*) (void)) f)) ();

  return 0;
}

static pthread_barrier_t startup_barrier, go_barrier;
static void *
waiter_thread (void *arg)
{
  void **f = arg;
  pthread_barrier_wait (&startup_barrier);
  pthread_barrier_wait (&go_barrier);

  (*((void (*) (void)) *f)) ();

  return 0;
}
#endif

static int
do_test (void)
{
  static void *f;		/* Address of this is used in other threads. */

#if USE_PTHREADS
  /* Create some threads while stacks are nonexecutable.  */
  #define N 5
  pthread_t thr[N];

  pthread_barrier_init (&startup_barrier, NULL, N + 1);
  pthread_barrier_init (&go_barrier, NULL, N + 1);

  for (int i = 0; i < N; ++i)
    {
      int rc = pthread_create (&thr[i], NULL, &waiter_thread, &f);
      if (rc)
	error (1, rc, "pthread_create");
    }

  /* Make sure they are all there using their stacks.  */
  pthread_barrier_wait (&startup_barrier);
  puts ("threads waiting");
#endif

  print_maps ();

  /* Loading this module should force stacks to become executable.  */
  void *h = dlopen ("tst-execstack-mod.so", RTLD_LAZY);
  if (h == NULL)
    {
      printf ("cannot load: %s\n", dlerror ());
      return 1;
    }

  f = dlsym (h, "tryme");
  if (f == NULL)
    {
      printf ("symbol not found: %s\n", dlerror ());
      return 1;
    }

  /* Test if that really made our stack executable.
     The `tryme' function should crash if not.  */

  (*((void (*) (void)) f)) ();

  print_maps ();

  /* Test that growing the stack region gets new executable pages too.  */
  deeper ((void (*) (void)) f);

  print_maps ();

#if USE_PTHREADS
  /* Test that a fresh thread now gets an executable stack.  */
  {
    pthread_t th;
    int rc = pthread_create (&th, NULL, &tryme_thread, f);
    if (rc)
      error (1, rc, "pthread_create");
  }

  puts ("threads go");
  /* The existing threads' stacks should have been changed.
     Let them run to test it.  */
  pthread_barrier_wait (&go_barrier);

  pthread_exit (0);
#endif

  return 0;
}

static void
deeper (void (*f) (void))
{
  char stack[1100 * 1024];
  memfrob (stack, sizeof stack);
  (*f) ();
  memfrob (stack, sizeof stack);
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
