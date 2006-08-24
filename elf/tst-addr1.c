#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  Dl_info i;
  if (dladdr (&printf, &i) == 0)
    {
      puts ("not found");
      return 1;
    }
  printf ("found symbol %s in %s\n", i.dli_sname, i.dli_fname);
  return i.dli_sname == NULL || strcmp (i.dli_sname, "printf") != 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
