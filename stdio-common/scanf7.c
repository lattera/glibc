#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  long long int n;
  int ret;

  n = -1;
  ret = sscanf ("1000", "%lld", &n);
  printf ("%%lld: ret: %d, n: %Ld\n", ret, n);
  if (ret != 1 || n != 1000L)
    abort ();

  n = -2;
  ret = sscanf ("1000", "%llld", &n);
  printf ("%%llld: ret: %d, n: %Ld\n", ret, n);
  if (ret > 0 || n >= 0L)
    abort ();

  return 0;
}
