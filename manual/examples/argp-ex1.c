/* Argp example #1 -- a minimal program using argp */

#include <argp.h>

int main (int argc, char **argv)
{
  argp_parse (0, argc, argv, 0, 0, 0);
  exit (0);
}
