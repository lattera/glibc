#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  char str[] = "this is a test";

  strfry (str);

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
