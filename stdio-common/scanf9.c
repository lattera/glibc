#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  main(int argc, char *argv[])  {
  int  matches;
  char  str[10];

  str[0] = '\0';
  matches = -9;
  matches = sscanf("x ]", "%[^] ]", str);
  printf("Matches = %d, string str = \"%s\".\n", matches, str);
  printf("str should be \"x\".\n");
  if (strcmp (str, "x")) abort ();
  str[0] = '\0';
  matches = -9;
  matches = sscanf(" ] x", "%[] ]", str);
  printf("Matches = %d, string str = \"%s\".\n", matches, str);
  printf("str should be \" ] \".\n");
  if (strcmp (str, " ] ")) abort ();
  exit(0);
  return 0;
}
