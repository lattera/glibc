/* Create simple DB database from textual input.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "nss_db/dummy-db.h"

/* Get libc version number.  */
#include "../version.h"

#define PACKAGE _libc_intl_domainname

/* If non-zero convert key to lower case.  */
static int to_lowercase;

/* If non-zero print content of input file, one entry per line.  */
static int do_undo;

/* If non-zero do not print informational messages.  */
static int be_quiet;

/* Name of output file.  */
static const char *output_name;

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { "fold-case", 'f', NULL, 0, N_("Convert key to lower case") },
  { "output", 'o', N_("NAME"), 0, N_("Write output to file NAME") },
  { "quiet", 'q', NULL, 0,
    N_("Do not print messages while building database") },
  { "undo", 'u', NULL, 0,
    N_("Print content of database file, one entry a line") },
  { NULL, 0, NULL, 0, NULL }
};

/* Short description of program.  */
static const char doc[] = N_("Create simple DB database from textual input.");

/* Strings for arguments in help texts.  */
static const char args_doc[] = N_("\
INPUT-FILE OUTPUT-FILE\n-o OUTPUT-FILE INPUT-FILE\n-u INPUT-FILE");

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Function to print some extra text in the help message.  */
static char *more_help (int key, const char *text, void *input);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, args_doc, doc, NULL, more_help
};


/* Prototypes for local functions.  */
static int process_input (FILE *input, const char *inname, NSS_DB *output,
			  int to_lowercase, int be_quiet);
static int print_database (NSS_DB *db);


int
main (int argc, char *argv[])
{
  const char *input_name;
  FILE *input_file;
  NSS_DB *db_file;
  int status;
  int remaining;
  int mode = 0666;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  textdomain (_libc_intl_domainname);

  /* Initialize local variables.  */
  input_name = NULL;

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  /* Determine file names.  */
  if (do_undo || output_name != NULL)
    {
      if (remaining + 1 != argc)
	{
	wrong_arguments:
	  error (0, 0, gettext ("wrong number of arguments"));
	  argp_help (&argp, stdout, ARGP_HELP_SEE,
		     program_invocation_short_name);
	  exit (1);
	}
      input_name = argv[remaining];
    }
  else
    {
      if (remaining + 2 != argc)
	goto wrong_arguments;

      input_name = argv[remaining++];
      output_name = argv[remaining];
    }

  /* First load the shared object to initialize version dependend
     variables.  */
  if (load_db () != NSS_STATUS_SUCCESS)
    error (EXIT_FAILURE, 0, gettext ("No usable database library found."));

  /* Special handling if we are asked to print the database.  */
  if (do_undo)
    {
      dbopen (input_name, db_rdonly, 0666, &db_file);
      if (db_file == NULL)
	error (EXIT_FAILURE, 0, gettext ("cannot open database file `%s': %s"),
	       input_name,
	       (errno == EINVAL ? gettext ("incorrectly formatted file")
		: strerror (errno)));

      status = print_database (db_file);

      db_file->close (db_file->db, 0);

      return status;
    }

  /* Open input file.  */
  if (strcmp (input_name, "-") == 0 || strcmp (input_name, "/dev/stdin") == 0)
    input_file = stdin;
  else
    {
      struct stat st;

      input_file = fopen (input_name, "r");
      if (input_file == NULL)
	error (EXIT_FAILURE, errno, gettext ("cannot open input file `%s'"),
	       input_name);

      /* Get the access rights from the source file.  The output file should
	 have the same.  */
      if (fstat (fileno (input_file), &st) >= 0)
	mode = st.st_mode & ACCESSPERMS;
    }

  /* Open output file.  This must not be standard output so we don't
     handle "-" and "/dev/stdout" special.  */
  dbopen (output_name, DB_CREATE | db_truncate, mode, &db_file);
  if (db_file == NULL)
    error (EXIT_FAILURE, errno, gettext ("cannot open output file `%s'"),
	   output_name);

  /* Start the real work.  */
  status = process_input (input_file, input_name, db_file, to_lowercase,
			  be_quiet);

  /* Close files.  */
  if (input_file != stdin)
    fclose (input_file);
  db_file->close (db_file->db, 0);

  return status;
}


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'f':
      to_lowercase = 1;
      break;
    case 'o':
      output_name = arg;
      break;
    case 'q':
      be_quiet = 1;
      break;
    case 'u':
      do_undo = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}


static char *
more_help (int key, const char *text, void *input)
{
  switch (key)
    {
    case ARGP_KEY_HELP_EXTRA:
      /* We print some extra information.  */
      return strdup (gettext ("\
For bug reporting instructions, please see:\n\
<http://www.gnu.org/software/libc/bugs.html>.\n"));
    default:
      break;
    }
  return (char *) text;
}

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "makedb (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "2000");
  fprintf (stream, gettext ("Written by %s.\n"), "Ulrich Drepper");
}


static int
process_input (input, inname, output, to_lowercase, be_quiet)
     FILE *input;
     const char *inname;
     NSS_DB *output;
     int to_lowercase;
     int be_quiet;
{
  char *line;
  size_t linelen;
  int status;
  size_t linenr;

  line = NULL;
  linelen = 0;
  status = EXIT_SUCCESS;
  linenr = 0;

  while (!feof (input))
    {
      DBT key;
      DBT val;
      char *cp;
      int n;

      n = getline (&line, &linelen, input);
      if (n < 0)
	/* This means end of file or some bug.  */
	break;
      if (n == 0)
	/* Short read.  Probably interrupted system call. */
	continue;

      ++linenr;

      if (line[n - 1] == '\n')
	/* Remove trailing newline.  */
	line[--n] = '\0';

      cp = line;
      while (isspace (*cp))
	++cp;

      if (*cp == '#')
	/* First non-space character in line '#': it's a comment.  */
	continue;

      key.data = cp;
      while (*cp != '\0' && !isspace (*cp))
	{
	  if (to_lowercase)
	    *cp = tolower (*cp);
	  ++cp;
	}

      if (key.data == cp)
	/* It's an empty line.  */
	continue;

      key.size = cp - (char *) key.data;
      key.flags = 0;

      while (isspace (*cp))
	++cp;

      val.data = cp;
      val.size = (&line[n] - cp) + 1;
      val.flags = 0;

      /* Store the value.  */
      status = output->put (output->db, NULL, &key, &val, db_nooverwrite);
      if (status != 0)
	{
	  if (status == db_keyexist)
	    {
	      if (!be_quiet)
		error_at_line (0, 0, inname, linenr,
			       gettext ("duplicate key"));
	      /* This is no real error.  Just give a warning.  */
	      status = 0;
	      continue;
	    }
	  else
	    error (0, status, gettext ("while writing database file"));

	  status = EXIT_FAILURE;

	  clearerr (input);
	  break;
	}
    }

  if (ferror (input))
    {
      error (0, 0, gettext ("problems while reading `%s'"), inname);
      status = EXIT_FAILURE;
    }

  return status;
}


static int
print_database (db)
     NSS_DB *db;
{
  DBT key;
  DBT val;
  NSS_DBC *cursor;
  int status;

  status = db->cursor (db->db, NULL, &cursor);
  if (status != 0)
    {
      error (0, status, gettext ("while reading database"));
      return EXIT_FAILURE;
    }

  key.flags = 0;
  val.flags = 0;
  status = cursor->c_get (cursor->cursor, &key, &val, db_first);
  while (status == 0)
    {
      printf ("%.*s %s\n", (int) key.size, (char *) key.data,
	      (char *) val.data);

      status = cursor->c_get (cursor->cursor, &key, &val, db_next);
    }

  if (status != db_notfound)
    {
      error (0, status, gettext ("while reading database"));
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
