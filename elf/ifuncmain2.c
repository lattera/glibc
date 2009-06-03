/* Test calling one STT_GNU_IFUNC function with 3 different
   STT_GNU_IFUNC definitions.  */

#include <stdlib.h>

int global = -1;

extern int foo1 (void);

int
main (void)
{
  if (foo1 () != -1)
    abort ();
  return 0;
}
