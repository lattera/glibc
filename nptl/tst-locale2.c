/* Test that the thread-local locale works right in the main thread
   when statically linked.  */

#include "../argp/tst-argp1.c"

#include <pthread.h>

/* This is never called, just here to get pthreads linked in.  */
void
useless (void)
{
  pthread_create (0, 0, 0, 0);
}
