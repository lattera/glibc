/* PR libc/4005 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

void *
run_thread (void *a)
{
  while (1)
    {
      sleep (10);
    }
  return 0;
}

int
main (void)
{
  pthread_t thr;
  void *result;
  alarm (4);
  printf ("Starting thread.\n");
  pthread_create (&thr, 0, run_thread, 0);
  sleep (2);
  printf ("Canceling thread.\n");
  pthread_cancel (thr);
  pthread_join (thr, &result);
  if (result == PTHREAD_CANCELED)
    printf ("Thread canceled.\n");
  else
    printf ("Thread exited.\n");
  return 0;
}
