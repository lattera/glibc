#include <stdio.h>
#include <stdlib.h>

main ()
{
  int n = -1;
  char c = '!';
  int ret;

  ret = sscanf ("0x", "%i%c", &n, &c);
  printf ("ret: %d, n: %d, c: %c\n", ret, n, c);
  if (ret != 2 || n != 0 || c != 'x')
    abort ();
  return 0;
}
