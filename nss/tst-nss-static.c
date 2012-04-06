/* glibc test for static NSS.  */
#include <stdio.h>

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
  struct passwd *pw;

  pw = getpwuid(0);
  return pw == NULL;
}


#include "../test-skeleton.c"
