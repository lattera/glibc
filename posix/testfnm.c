#include <stdio.h>
#include "fnmatch.h"

int
main (c, v)
     int c;
     char **v;
{
  printf ("%d\n", fnmatch (v[1], v[2], FNM_PERIOD));
  printf ("%d\n", fnmatch (v[1], v[2], FNM_CASEFOLD|FNM_PERIOD));
  exit (0);
}
