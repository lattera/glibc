#include <dlfcn.h>

extern int bar (void);

void *h;

int
foo (void)
{
  return 42 + bar ();
}

int
baz (void)
{
  return -21;
}

void
__attribute__ ((__constructor__))
init (void)
{
  h = dlopen ("constload3.so", RTLD_GLOBAL | RTLD_LAZY);
}
