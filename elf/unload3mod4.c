#include <stdio.h>

int
bar (int x)
{
  puts ("bar");
  fflush (stdout);
  x = foo (x - 4);
  puts ("bar after foo");
  return x;
}
