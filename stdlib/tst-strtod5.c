#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NBSP "\xc2\xa0"

static const struct
{
  const char *in;
  int group;
  double expected;
} tests[] =
  {
    { "0", 0, 0.0 },
    { "000", 0, 0.0 },
    { "-0", 0, -0.0 },
    { "-000", 0, -0.0 },
    { "0,", 0, 0.0 },
    { "-0,", 0, -0.0 },
    { "0,0", 0, 0.0 },
    { "-0,0", 0, -0.0 },
    { "0e-10", 0, 0.0 },
    { "-0e-10", 0, -0.0 },
    { "0,e-10", 0, 0.0 },
    { "-0,e-10", 0, -0.0 },
    { "0,0e-10", 0, 0.0 },
    { "-0,0e-10", 0, -0.0 },
    { "0e-1000000", 0, 0.0 },
    { "-0e-1000000", 0, -0.0 },
    { "0,0e-1000000", 0, 0.0 },
    { "-0,0e-1000000", 0, -0.0 },
    { "0", 1, 0.0 },
    { "000", 1, 0.0 },
    { "-0", 1, -0.0 },
    { "-000", 1, -0.0 },
    { "0e-10", 1, 0.0 },
    { "-0e-10", 1, -0.0 },
    { "0e-1000000", 1, 0.0 },
    { "-0e-1000000", 1, -0.0 },
    { "000"NBSP"000"NBSP"000", 1, 0.0 },
    { "-000"NBSP"000"NBSP"000", 1, -0.0 }
  };
#define NTESTS (sizeof (tests) / sizeof (tests[0]))


static int
do_test (void)
{
  if (setlocale (LC_ALL, "cs_CZ.UTF-8") == NULL)
    {
      puts ("could not set locale");
      return 1;
    }

  int status = 0;

  for (int i = 0; i < NTESTS; ++i)
    {
      char *ep;
      double r;

      if (tests[i].group)
	r = __strtod_internal (tests[i].in, &ep, 1);
      else
	r = strtod (tests[i].in, &ep);

      if (*ep != '\0')
	{
	  printf ("%d: got rest string \"%s\", expected \"\"\n", i, ep);
	  status = 1;
	}

      if (r != tests[i].expected
	  || copysign (10.0, r) != copysign (10.0, tests[i].expected))
	{
	  printf ("%d: got wrong results %g, expected %g\n",
		  i, r, tests[i].expected);
	  status = 1;
	}
    }

  return status;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
