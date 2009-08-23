#include <regex.h>

static int
do_test (void)
{
  regex_t r;
  int e = regcomp(&r, "xy\\{4,5,7\\}zabc", 0);
  char buf[100];
  regerror(e, &r, buf, sizeof (buf));
  printf ("e = %d (%s)\n", e, buf);
  return e != REG_BADBR;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
