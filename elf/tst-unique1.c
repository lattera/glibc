#include <config.h>
#include <dlfcn.h>
#include <stdio.h>

static int
do_test (void)
{
#ifdef HAVE_ASM_UNIQUE_OBJECT
  void *h1 = dlopen ("tst-unique1mod1.so", RTLD_LAZY);
  if (h1 == NULL)
    {
      puts ("cannot load tst-unique1mod1");
      return 1;
    }
  int *(*f1) (void) = dlsym (h1, "f");
  if (f1 == NULL)
    {
      puts ("cannot locate f in tst-unique1mod1");
      return 1;
    }
  void *h2 = dlopen ("tst-unique1mod2.so", RTLD_LAZY);
  if (h2 == NULL)
    {
      puts ("cannot load tst-unique1mod2");
      return 1;
    }
  int (*f2) (int *) = dlsym (h2, "f");
  if (f2 == NULL)
    {
      puts ("cannot locate f in tst-unique1mod2");
      return 1;
    }
  return f2 (f1 ());
#else
  return 0;
#endif
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
