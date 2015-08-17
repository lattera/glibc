#include <limits.h>
#include <stdio.h>
#include <time.h>


static const struct
{
  const char *fmt;
  long int gmtoff;
} tests[] =
  {
    { "1113472456 +1000", 36000 },
    { "1113472456 -1000", -36000 },
    { "1113472456 +10", 36000 },
    { "1113472456 -10", -36000 },
    { "1113472456 +1030", 37800 },
    { "1113472456 -1030", -37800 },
    { "1113472456 +0030", 1800 },
    { "1113472456 -0030", -1800 },
    { "1113472456 +1157", 43020 },
    { "1113472456 +1158", 43080 },
    { "1113472456 +1159", 43140 },
    { "1113472456 +1200", 43200 },
    { "1113472456 -1200", -43200 },
    { "1113472456 +1201", 43260 },
    { "1113472456 -1201", -43260 },
    { "1113472456 +1330", 48600 },
    { "1113472456 -1330", -48600 },
    { "1113472456 +1400", 50400 },
    { "1113472456 +1401", 50460 },
    { "1113472456 -2459", -89940 },
    { "1113472456 -2500", -90000 },
    { "1113472456 +2559", 93540 },
    { "1113472456 +2600", 93600 },
    { "1113472456 -99", -356400 },
    { "1113472456 +99", 356400 },
    { "1113472456 -9959", -359940 },
    { "1113472456 +9959", 359940 },
    { "1113472456 -1060", LONG_MAX },
    { "1113472456 +1060", LONG_MAX },
    { "1113472456  1030", LONG_MAX },
  };
#define ntests (sizeof (tests) / sizeof (tests[0]))


static int
do_test (void)
{
  int result = 0;

  for (int i = 0; i < ntests; ++i)
    {
      struct tm tm;

      if (strptime (tests[i].fmt, "%s %z", &tm) == NULL)
	{
	  if (tests[i].gmtoff != LONG_MAX)
	    {
	      printf ("round %d: strptime unexpectedly failed\n", i);
	      result = 1;
	    }
	  continue;
	}

      if (tm.tm_gmtoff != tests[i].gmtoff)
	{
	  printf ("round %d: tm_gmtoff is %ld\n", i, (long int) tm.tm_gmtoff);
	  result = 1;
	}
    }

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
