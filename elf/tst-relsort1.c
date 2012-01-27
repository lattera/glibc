#include <dlfcn.h>
#include <stdio.h>


static int
do_test ()
{
  const char lib[] = "$ORIGIN/tst-relsort1mod1.so";
  void *h = dlopen (lib, RTLD_NOW);
  if (h == NULL)
    {
      puts (dlerror ());
      return 1;
    }
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
