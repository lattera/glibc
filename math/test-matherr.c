#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int fail = 1;

int
matherr (struct exception *s)
{
  printf ("matherr is working\n");
  fail = 0;
  return 1;
}

static int
do_test (void)
{
  _LIB_VERSION = _SVID_;
  acos (2.0);
  return fail;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
