/* Argp example #2 -- a pretty minimal program using argp */

#include <argp.h>

const char *argp_program_version =
  "argp-ex2 1.0";
const char *argp_program_bug_address =
  "<bug-gnu-utils@@prep.ai.mit.edu>";

/* Program documentation.  */
static char doc[] =
  "Argp example #2 -- a pretty minimal program using argp";

/* Our argpument parser.  The @code{options}, @code{parser}, and
   @code{args_doc} fields are zero because we have neither options or
   arguments; @code{doc} and @code{argp_program_bug_address} will be
   used in the output for @samp{--help}, and the @samp{--version}
   option will print out @code{argp_program_version}.  */
static struct argp argp = { 0, 0, 0, doc };

int main (int argc, char **argv)
{
  argp_parse (&argp, argc, argv, 0, 0, 0);
  exit (0);
}
