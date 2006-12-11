#include <stdio.h>
#include <stdlib.h>

static int
do_test (void)
{
  int status = 0;
  const char s[] = "0x";
  char *ep;
  double r = strtod (s, &ep);
  if (r != 0)
    {
      printf ("r = %g, expect 0\n", r);
      status = 1;
    }
  if (ep != s + 1)
    {
      printf ("strtod parsed %ju characters, expected 1\n", ep - s);
      status = 1;
    }
  return status;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
