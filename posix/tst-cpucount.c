#include <sched.h>
#include <stdio.h>

static int
do_test (void)
{
  cpu_set_t c;

  CPU_ZERO (&c);

  for (int cnt = 0; cnt < 130; ++cnt)
    {
      int n = CPU_COUNT (&c);
      if (n != cnt)
	{
	  printf ("expected %d, not %d\n", cnt, n);
	  return 1;
	}

      CPU_SET (cnt, &c);
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
