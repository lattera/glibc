#include <stdio.h>

int
main (int argc, char **argv)
{
  /* Complexity to keep gcc from optimizing this away.  */
  printf ("This is a test %s.\n", argc > 1 ? argv[1] : "null");
  return 0;
}
