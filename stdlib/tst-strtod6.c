#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
do_test (void)
{
  static const char str[] = "NaN(blabla)something";
  char *endp;
  int result = 0;

  double d = strtod (str, &endp);
  if (!isnan (d))
    {
      puts ("strtod did not return NAN");
      result = 1;
    }
  if (strcmp (endp, "something") != 0)
    {
      puts  ("strtod set incorrect end pointer");
      result = 1;
    }

  float f = strtof (str, &endp);
  if (!isnanf (f))
    {
      puts ("strtof did not return NAN");
      result = 1;
    }
  if (strcmp (endp, "something") != 0)
    {
      puts  ("strtof set incorrect end pointer");
      result = 1;
    }

  long double ld = strtold (str, &endp);
  if (!isnan (ld))
    {
      puts ("strtold did not return NAN");
      result = 1;
    }
  if (strcmp (endp, "something") != 0)
    {
      puts  ("strtold set incorrect end pointer");
      result = 1;
    }

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
