/* Test that the thread-local locale works right in the main thread
   when statically linked.  */

#include "../locale/tst-C-locale.c"

#include <pthread.h>

/* This is never called, just here to get pthreads linked in.  */
void
useless (void)
{
  pthread_create (0, 0, 0, 0);
}
