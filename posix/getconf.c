/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct conf
  {
    CONST char *name;
    CONST int call_name;
    CONST enum { SYSCONF, CONFSTR, PATHCONF } call;
  };

static struct conf vars[] =
  {
    { "LINK_MAX", _PC_LINK_MAX, PATHCONF },
    { "MAX_CANON", _PC_MAX_CANON, PATHCONF },
    { "MAX_INPUT", _PC_MAX_INPUT, PATHCONF },
    { "NAME_MAX", _PC_NAME_MAX, PATHCONF },
    { "PATH_MAX", _PC_PATH_MAX, PATHCONF },
    { "PIPE_BUF", _PC_PIPE_BUF, PATHCONF },
    { "_POSIX_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED, PATHCONF },
    { "_POSIX_NO_TRUNC", _PC_NO_TRUNC, PATHCONF },
    { "_POSIX_VDISABLE", _PC_VDISABLE, PATHCONF },

    { "ARG_MAX", _SC_ARG_MAX, SYSCONF },
    { "CHILD_MAX", _SC_CHILD_MAX, SYSCONF },
    { "CLK_TCK", _SC_CLK_TCK, SYSCONF },
    { "NGROUPS_MAX", _SC_NGROUPS_MAX, SYSCONF },
    { "OPEN_MAX", _SC_OPEN_MAX, SYSCONF },
    { "_POSIX_JOB_CONTROL", _SC_JOB_CONTROL, SYSCONF },
    { "_POSIX_SAVED_IDS", _SC_SAVED_IDS, SYSCONF },
    { "_POSIX_VERSION", _SC_VERSION, SYSCONF },

    { "PATH", _CS_PATH, CONFSTR },

    { NULL, 0, SYSCONF }
  };

static CONST char *program;

static void
DEFUN_VOID(usage)
{
  fprintf (stderr, "Usage: %s variable_name [pathname]\n", program);
  exit (2);
}

int
DEFUN(main, (argc, argv), int argc AND char **argv)
{
  register CONST struct conf *c;

  program = strrchr (argv[0], '/');
  if (program == NULL)
    program = argv[0];
  else
    ++program;

  if (argc < 2 || argc > 3)
    usage ();

  for (c = vars; c->name != NULL; ++c)
    if (!strcmp (c->name, argv[1]))
      {
	long int value;
	size_t clen;
	char *cvalue;
	switch (c->call)
	  {
	  case PATHCONF:
	    if (argc < 3)
	      usage ();
	    value = pathconf (argv[2], c->call_name);
	    if (value == -1)
	      {
		fprintf (stderr, "%s: pathconf: %s: %s\n",
			 program, argv[2], strerror (errno));
		exit (3);
	      }
	    printf ("%ld\n", value);
	    exit (0);

	  case SYSCONF:
	    if (argc > 2)
	      usage ();
	    value = sysconf (c->call_name);
	    printf ("%ld\n", value);
	    exit (0);

	  case CONFSTR:
	    if (argc > 2)
	      usage ();
	    clen = confstr (c->call_name, (char *) NULL, 0);
	    cvalue = (char *) malloc (clen);
	    if (cvalue == NULL)
	      {
		fprintf (stderr, "%s: malloc: %s\n",
			 program, strerror (errno));
		exit (3);
	      }
	    if (confstr (c->call_name, cvalue, clen) != clen)
	      {
		fprintf (stderr, "%s: confstr: %s\n",
			 program, strerror (errno));
		exit (3);
	      }
	    printf ("%.*s\n", (int) clen, cvalue);
	    exit (0);
	  }
      }

  fprintf (stderr, "%s: Unrecognized variable `%s'\n", program, argv[1]);
  exit (2);
}
