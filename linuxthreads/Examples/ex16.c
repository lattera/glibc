/* Tst case by Jakub Jelinek <jakub@redhat.com>.  */
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static void *
task (void *p)
{
  sleep (30);
  return NULL;
}

int
main (void)
{
  pthread_t t;
  int status;

  status = pthread_create (&t, NULL, task, NULL);
  if (status)
    exit (status);

  status = pthread_detach (t);
  pthread_kill_other_threads_np ();
  return status;
}
