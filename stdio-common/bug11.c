#include <stdio.h>
#include <string.h>

main ()
{
  int ret;
  char buf [1024] = "Ooops";

  ret = sscanf ("static char Term_bits[] = {", "static char %s = {", buf);
  printf ("ret: %d, name: %s\n", ret, buf);

  return strcmp (buf, "Term_bits[]") != 0 || ret != 1;
}
