/* BZ #5424 */
#include <stdio.h>
#include <errno.h>

/* INT_MAX + 1 */
#define N 2147483648

/* (INT_MAX / 2) + 2 */
#define N2 1073741825

/* INT_MAX - 3 */
#define N3 2147483644

#define STRINGIFY(S) #S
#define MAKE_STR(S) STRINGIFY(S)

#define SN MAKE_STR(N)
#define SN2 MAKE_STR(N2)
#define SN3 MAKE_STR(N3)

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

  ret = fprintf (fp, "%" SN "d", 1);
  printf ("ret = %d\n", ret);
  if (ret != -1 || errno != EOVERFLOW)
	  return 1;

  ret = fprintf (fp, "%." SN "d", 1);
  printf ("ret = %d\n", ret);
  if (ret != -1 || errno != EOVERFLOW)
	  return 1;

  ret = fprintf (fp, "%." SN3 "d", 1);
  printf ("ret = %d\n", ret);
  if (ret != -1 || errno != EOVERFLOW)
	  return 1;

  ret = fprintf (fp, "%" SN2 "d%" SN2 "d", 1, 1);
  printf ("ret = %d\n", ret);

  return ret != -1 || errno != EOVERFLOW;
}

#define TIMEOUT 30
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
