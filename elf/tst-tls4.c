#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <tls.h>

#ifdef USE_TLS
# include "tls-macros.h"

/* This gives the executable a TLS segment so that even if the libc.so
   it loads has none (i.e. --with-tls --without-__thread), ld.so will
   permit loading of objects with TLS segments.  */
COMMON_INT_DEF(loser);
#endif

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
#ifdef USE_TLS
  static const char modname[] = "tst-tlsmod2.so";
  int result = 0;
  int *foop;
  int (*fp) (int, int *);
  void *h;

  h = dlopen (modname, RTLD_LAZY);
  if (h == NULL)
    {
      printf ("cannot open '%s': %s\n", modname, dlerror ());
      exit (1);
    }

  fp = dlsym (h, "in_dso");
  if (fp == NULL)
    {
      printf ("cannot get symbol 'in_dso': %s\n", dlerror ());
      exit (1);
    }

  result |= fp (0, NULL);

  foop = dlsym (h, "foo");
  if (foop == NULL)
    {
      printf ("cannot get symbol 'foo' the second time: %s\n", dlerror ());
      exit (1);
    }
  if (*foop != 16)
    {
      puts ("foo != 16");
      result = 1;
    }

  dlclose (h);

  return result;
#else
  return 0;
#endif
}


#include "../test-skeleton.c"
