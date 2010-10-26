#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  char s[] __attribute__((aligned(16))) = "\xff";
  char *p = strchr (s, '\xfe');
  printf ("%p\n", p);
  return p != NULL;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
