#include <ansidecl.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char **argv)
{
  int aflag = 0;
  int bflag = 0;
  char *cvalue = NULL;
  int index;
  int c;

  while ((c = getopt (argc, argv, "abc:")) >= 0)
    switch (c) {
    case 'a':
      aflag = 1;
      break;
    case 'b':
      bflag = 1;
      break;
    case 'c':
      cvalue = optarg;
      break;
    case '?':
#if 0
      fprintf (stderr, "Unknown option %c.\n", optopt);
#else
      fputs ("Unknown option.\n", stderr);
#endif
      return -1;
    default:
      fprintf (stderr, "This should never happen!\n");
      return -1;
    }

  printf ("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);
  return 0;
}
