/* BZ #5424 */
#include <stdio.h>

#define N 2147483648

#define STRINGIFY(S) #S
#define MAKE_STR(S) STRINGIFY(S)

#define SN MAKE_STR(N)

static int
do_test (void)
{
  int ret;

  FILE *fp = fopen ("/dev/null", "w");
  if (fp == NULL)
    {
      puts ("cannot open /dev/null");
      return 1;
    }

  ret = fprintf (fp, "%" SN "d%" SN "d", 1, 1);

  printf ("ret = %d\n", ret);

  return ret != -1;
}

#define TIMEOUT 30
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
