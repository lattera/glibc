/* pt_chmod - helper program for `grantpt'.
   Copyright (C) 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by C. Scott Ananian <cananian@alumni.princeton.edu>, 1998.

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

#include <argp.h>
#include <errno.h>
#include <error.h>
#include <grp.h>
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pty-private.h"

/* Get libc version number.  */
#include "../version.h"

#define PACKAGE _libc_intl_domainname

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Function to print some extra text in the help message.  */
static char *more_help (int key, const char *text, void *input);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  NULL, NULL, NULL, NULL, NULL, more_help
};


/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "pt_chown (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1999");
}

static char *
more_help (int key, const char *text, void *input)
{
  char *cp;

  switch (key)
    {
    case ARGP_KEY_HELP_PRE_DOC:
      asprintf (&cp, gettext ("\
Set the owner, group and access permission of the slave pseudo\
 terminal corresponding to the master pseudo terminal passed on\
 file descriptor `%d'.  This is the helper program for the\
 `grantpt' function.  It is not intended to be run directly from\
 the command line.\n"),
		PTY_FILENO);
      return cp;
    case ARGP_KEY_HELP_EXTRA:
      /* We print some extra information.  */
      asprintf (&cp, gettext ("\
The owner is set to the current user, the group is set to `%s',\
 and the access permission is set to `%o'.\n\n\
%s"),
		TTY_GROUP, S_IRUSR|S_IWUSR|S_IWGRP, gettext ("\
For bug reporting instructions, please see:\n\
<http://www.gnu.org/software/libc/bugs.html>.\n"));
      return cp;
    default:
      break;
    }
  return (char *) text;
}

static int
do_pt_chown (void)
{
  char *pty;
  struct stat st;
  struct group *p;
  gid_t gid;

  /* Check that PTY_FILENO is a valid master pseudo terminal.  */
  pty = ptsname (PTY_FILENO);
  if (pty == NULL)
    return errno == EBADF ? FAIL_EBADF : FAIL_EINVAL;

  /* Check that the returned slave pseudo terminal is a
     character device.  */
  if (stat (pty, &st) < 0 || !S_ISCHR(st.st_mode))
    return FAIL_EINVAL;

  /* Get the group ID of the special `tty' group.  */
  p = getgrnam (TTY_GROUP);
  gid = p ? p->gr_gid : getgid ();

  /* Set the owner to the real user ID, and the group to that special
     group ID.  */
  if (chown (pty, getuid (), gid) < 0)
    return FAIL_EACCES;

  /* Set the permission mode to readable and writable by the owner,
     and writable by the group.  */
  if (chmod (pty, S_IRUSR|S_IWUSR|S_IWGRP) < 0)
    return FAIL_EACCES;

  return 0;
}


int
main (int argc, char *argv[])
{
  uid_t euid = geteuid ();
  int remaining;

  /* Normal invocation of this program is with no arguments and
     with privileges.
     FIXME: Should use capable (CAP_CHOWN|CAP_FOWNER).  */
  if (argc == 1 && euid == 0)
    return do_pt_chown ();

  /* We aren't going to be using privileges, so drop them right now. */
  setuid (getuid ());

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  textdomain (PACKAGE);

  /* parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (remaining < argc)
    {
      /* We should not be called with any non-option parameters.  */
      error (0, 0, gettext ("too many arguments"));
      argp_help (&argp, stdout, ARGP_HELP_SEE | ARGP_HELP_EXIT_ERR,
		 program_invocation_short_name);
      return EXIT_FAILURE;
    }

  /* Check if we are properly installed.  */
  if (euid != 0)
    error (FAIL_EXEC, 0, gettext ("needs to be installed setuid `root'"));

  return EXIT_SUCCESS;
}
