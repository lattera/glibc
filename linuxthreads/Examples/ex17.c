#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>

static pthread_mutex_t synch = PTHREAD_MUTEX_INITIALIZER;

static void *
test_thread (void *v_param)
{
  pthread_mutex_lock (&synch);
  return NULL;
}

#define STACKSIZE 0x100000

int
main (void)
{
  pthread_t thread;
  pthread_attr_t attr;
  int status;
  void *stack, *stack2;
  size_t stacksize;

  pthread_attr_init (&attr);
  stack = mmap (NULL, STACKSIZE,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (stack == MAP_FAILED)
    {
      perror ("mmap failed");
      return 1;
    }

  status = pthread_attr_setstack (&attr, stack, STACKSIZE);
  if (status != 0)
    {
      printf ("pthread_attr_setstack failed: %s\n", strerror (status));
      return 1;
    }

  status = pthread_attr_getstack (&attr, &stack2, &stacksize);
  if (status != 0)
    {
      printf ("pthread_attr_getstack failed: %s\n", strerror (status));
      return 1;
    }

  if (stack2 != stack || stacksize != STACKSIZE)
    {
      printf ("first pthread_attr_getstack returned different stack (%p,%zx)\n"
	      "than was set by setstack (%p,%x)\n",
	      stack2, stacksize, stack, STACKSIZE);
      return 2;
    }

  status = pthread_mutex_lock (&synch);
  if (status != 0)
    {
      printf ("cannot get lock: %s\n", strerror (status));
      return 1;
    }

  status = pthread_create (&thread, &attr, test_thread, NULL);
  if (status != 0)
    {
      printf ("pthread_create failed: %s\n", strerror (status));
      return 1;
    }

  status = pthread_getattr_np (thread, &attr);
  if (status != 0)
    {
      printf ("pthread_getattr_np failed: %s\n", strerror (status));
      return 1;
    }

  status = pthread_attr_getstack (&attr, &stack2, &stacksize);
  if (status != 0)
    {
      printf ("pthread_attr_getstack failed: %s\n", strerror (status));
      return 1;
    }

  if (stack2 != stack || stacksize != STACKSIZE)
    {
      printf ("second pthread_attr_getstack returned different stack (%p,%zx)\n"
	      "than was set by setstack (%p,%x)\n",
	      stack2, stacksize, stack, STACKSIZE);
      return 3;
    }

  status = pthread_mutex_unlock (&synch);
  if (status != 0)
    {
      printf ("cannot release lock: %s\n", strerror (status));
      return 1;
    }

  /* pthread_detach (thread); */
  if (pthread_join (thread, NULL) != 0)
    {
      printf ("join failed\n");
      return 1;
    }
  return 0;
}
