#include <stdio.h>
#include <stdlib.h>

int
main(int arc, char *argv[])
{
  int res;
  unsigned int val;

  FILE *fp = fopen ("/dev/null", "r");

  val = 0;
  res = fscanf(fp, "%n", &val);

  printf("Result of fscanf %%n = %d\n", res);
  printf("Scanned format = %d\n", val);

  res = fscanf(fp, "");
  printf("Result of fscanf \"\" = %d\n", res);
  if (res != 0)
    abort ();

  res = fscanf(fp, "BLURB");
  printf("Result of fscanf \"BLURB\" = %d\n", res);
  if (res >= 0)
    abort ();

  fclose (fp);

  return 0;
  return 0;
}
