/* Argp example #3 -- a program with options and arguments using argp */

#include <argp.h>

const char *argp_program_version =
  "argp-ex3 1.0";
const char *argp_program_bug_address =
  "<bug-gnu-utils@@prep.ai.mit.edu>";

/* Program documentation.  */
static char doc[] =
  "Argp example #3 -- a program with options and arguments using argp";

/* A description of the arguments we accept.  */
static char args_doc[] = "ARG1 ARG2";

/* The options we understand.  */
static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"quiet",    'q', 0,      0,  "Don't produce any output" },
  {"silent",   's', 0,      OPTION_ALIAS },
  {"output",   'o', "FILE", 0,
   "Output to FILE instead of standard output" },
  { 0 }
};

/* Used by @code{main} to communicate with @code{parse_opt}.  */
struct arguments
{
  char *args[2];		/* @var{arg1} & @var{arg2} */
  int silent, verbose;
  char *output_file;
};

/* Parse a single option.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the @var{input} argument from @code{argp_parse}, which we
     know is a pointer to our arguments structure.  */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q': case 's':
      arguments->silent = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'o':
      arguments->output_file = arg;
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 2)
	/* Too many arguments.  */
	argp_usage (state);

      arguments->args[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2)
	/* Not enough arguments.  */
	argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser.  */
static struct argp argp = { options, parse_opt, args_doc, doc };

int main (int argc, char **argv)
{
  struct arguments arguments;

  /* Default values.  */
  arguments.silent = 0;
  arguments.verbose = 0;
  arguments.output_file = "-";

  /* Parse our arguments; every option seen by @code{parse_opt} will
     be reflected in @code{arguments}.  */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  printf ("ARG1 = %s\nARG2 = %s\nOUTPUT_FILE = %s\n"
	  "VERBOSE = %s\nSILENT = %s\n",
	  arguments.args[0], arguments.args[1],
	  arguments.output_file,
	  arguments.verbose ? "yes" : "no",
	  arguments.silent ? "yes" : "no");

  exit (0);
}
