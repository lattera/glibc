/* Copyright (c) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* nscd_nischeck: Check, if everybody has read permissions for NIS+ table.
   Return value:
    0: Everybody can read the NIS+ table
    1: Only authenticated users could read the NIS+ table */

#include <argp.h>
#include <error.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <rpcsvc/nis.h>

/* Get libc version number.  */
#include <version.h>

#define PACKAGE _libc_intl_domainname

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  NULL, NULL, NULL, NULL,
};

int
main (int argc, char **argv)
{
  int remaining;
  nis_result *res;
  char *tablename, *cp;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
  /* Set the text message domain.  */
  textdomain (PACKAGE);

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (remaining + 1 != argc)
    {
      error (0, 0, gettext ("wrong number of arguments"));
      argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
      exit (EXIT_FAILURE);
    }

  tablename = alloca (strlen (argv[1]) + 10);
  cp = stpcpy (tablename, argv[1]);
  strcpy (cp, ".org_dir");

  res = nis_lookup (tablename, EXPAND_NAME|FOLLOW_LINKS);

  if (res == NULL ||
      (res->status != NIS_SUCCESS && res->status != NIS_S_SUCCESS))
    return 0;

  if (NIS_NOBODY(NIS_RES_OBJECT(res)->zo_access, NIS_READ_ACC))
    return 0;
  else
    return 1;
}

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "nscd_nischeck (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1999");
  fprintf (stream, gettext ("Written by %s.\n"), "Thorsten Kukuk");
}
