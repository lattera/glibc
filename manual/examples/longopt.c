#include <stdio.h>

/* Flag set by @samp{--verbose}.  */
static int verbose_flag;

int
main (argc, argv)
     int argc;
     char **argv;
{
  int c;

  while (1)
    {
      static struct option long_options[] =
	{
	  /* These options set a flag.  */
	  {"verbose", 0, &verbose_flag, 1},
	  {"brief", 0, &verbose_flag, 0},
	  /* These options don't set a flag.
	     We distinguish them by their indices.  */
	  {"add", 1, 0, 0},
	  {"append", 0, 0, 0},
	  {"delete", 1, 0, 0},
	  {"create", 0, 0, 0},
	  {"file", 1, 0, 0},
	  {0, 0, 0, 0}
	};
      /* @code{getopt_long} stores the option index here.  */
      int option_index = 0;

      c = getopt_long (argc, argv, "abc:d:",
		       long_options, &option_index);

      /* Detect the end of the options.  */
      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  /* If this option set a flag, do nothing else now.  */
	  if (long_options[option_index].flag != 0)
	    break;
	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;

	case 'a':
	  puts ("option -a\n");
	  break;

	case 'b':
	  puts ("option -b\n");
	  break;

	case 'c':
	  printf ("option -c with value `%s'\n", optarg);
	  break;

	case 'd':
	  printf ("option -d with value `%s'\n", optarg);
	  break;

	case '?':
	  /* @code{getopt_long} already printed an error message.  */
	  break;

	default:
	  abort ();
	}
    }

  /* Instead of reporting @samp{--verbose}
     and @samp{--brief} as they are encountered,
     we report the final status resulting from them.  */
  if (verbose_flag)
    puts ("verbose flag is set");

  /* Print any remaining command line arguments (not options).  */
  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
	printf ("%s ", argv[optind++]);
      putchar ('\n');
    }

  exit (0);
}
