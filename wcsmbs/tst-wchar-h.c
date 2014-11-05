#include <stdlib.h>
#include <wchar.h>

static int
do_test (void)
{
  mbstate_t x;
  return sizeof (x) - sizeof (mbstate_t);
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
