#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
test (const char str[])
{
  char *endp;
  int result = 0;

  puts (str);

  double d = strtod (str, &endp);
  if (!isnan (d))
    {
      puts ("strtod did not return NAN");
      result = 1;
    }
  if (issignaling (d))
    {
      puts ("strtod returned a sNAN");
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
  if (issignaling (f))
    {
      puts ("strtof returned a sNAN");
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
  if (issignaling (ld))
    {
      puts ("strtold returned a sNAN");
      result = 1;
    }
  if (strcmp (endp, "something") != 0)
    {
      puts  ("strtold set incorrect end pointer");
      result = 1;
    }

  return result;
}

static int
do_test (void)
{
  int result = 0;

  result |= test ("NaN(blabla)something");
  result |= test ("NaN(1234)something");
  /* UINT32_MAX.  */
  result |= test ("NaN(4294967295)something");
  /* UINT64_MAX.  */
  result |= test ("NaN(18446744073709551615)something");
  /* The case of zero is special in that "something" has to be done to make the
     mantissa different from zero, which would mean infinity instead of
     NaN.  */
  result |= test ("NaN(0)something");

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
