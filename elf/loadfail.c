#include <dlfcn.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  void *h;

  if (dlopen ("testobj1.so", RTLD_GLOBAL | RTLD_NOW) == NULL
      || dlopen ("testobj1.so", RTLD_GLOBAL | RTLD_NOW) == NULL
      || dlopen ("testobj2.so", RTLD_GLOBAL | RTLD_NOW) == NULL
      || dlopen ("testobj3.so", RTLD_GLOBAL | RTLD_NOW) == NULL
      || dlopen ("testobj4.so", RTLD_GLOBAL | RTLD_NOW) == NULL
      || dlopen ("testobj5.so", RTLD_GLOBAL | RTLD_NOW) == NULL)
    error (EXIT_FAILURE, 0, "failed to load shared object: %s", dlerror ());

  h = dlopen ("failobj.so", RTLD_GLOBAL | RTLD_NOW);

  printf ("h = %p, %s\n", h, h == NULL ? "ok" : "fail");

  return h != NULL;
}

int
foo (int a)
{
  return 10;
}
