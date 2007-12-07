#include <stdio.h>

static int
do_test (void)
{
  static const char buf[] = " ";
  char *str;

  int r = sscanf (buf, "%as", &str);
  printf ("%d %p\n", r, str);

  return r != -1 || str != NULL;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
