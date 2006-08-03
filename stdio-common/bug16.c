#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  char buf[100];
  snprintf (buf, sizeof (buf), "%.0LA", 0x0.FFFFp+0L);

  if (strcmp (buf, "0X1P+0") != 0)
    {
      printf ("got \"%s\", expected \"0X1P+0\"\n", buf);
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
