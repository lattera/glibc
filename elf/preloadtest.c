#include <stdio.h>

extern int preload (int);

int
main (void)
{
  int res = preload (42);

  printf ("preload (42) = %d, %s\n", res, res == 92 ? "ok" : "wrong");

  return res != 92;
}

extern int foo (int a);
int
foo (int a)
{
  return a;
}
