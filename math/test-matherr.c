#undef _SVID_SOURCE
#define _SVID_SOURCE
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

int
main (void)
{
  _LIB_VERSION = _SVID_;
  acos (2.0);
  return fail;
}
