#include "testobj.h"

int
obj6func1 (int a __attribute__ ((unused)))
{
  return 77;
}

int
obj6func2 (int a)
{
  return foo (a) + 46;
}

int
preload (int a)
{
  return a;
}
