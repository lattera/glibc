#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

int
main (int argc, char **argv)
{
  static const struct option options[] =
    {
      {"required", required_argument, NULL, 'r'},
      {"optional", optional_argument, NULL, 'o'},
      {"none",     no_argument,       NULL, 'n'},
      {NULL,       0,                 NULL, 0 }
    };

  int aflag = 0;
  int bflag = 0;
  char *cvalue = NULL;
  int index;
  int c;

  while ((c = getopt_long (argc, argv, "abc:", options, NULL)) >= 0)
    switch (c)
      {
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
	fputs ("Unknown option.\n", stderr);
	return 1;
      default:
	fprintf (stderr, "This should never happen!\n");
	return 1;

      case 'r':
	printf ("--required %s\n", optarg);
	break;
      case 'o':
	printf ("--optional %s\n", optarg);
	break;
      case 'n':
	puts ("--none");
	break;
      }

  printf ("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);

  return 0;
}
