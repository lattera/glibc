/* makedb -- create simple DB database from textual input.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <db.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <getopt.h>
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Get libc version number.  */
#include "../version.h"

#define PACKAGE _libc_intl_domainname

/* Long options.  */
static const struct option long_options[] =
{
  { "help", no_argument, NULL, 'h' },
  { "fold-case", no_argument, NULL, 'f' },
  { "output", required_argument, NULL, 'o' },
  { "quiet", no_argument, NULL, 'q' },
  { "undo", no_argument, NULL, 'u' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0}
};

/* Prototypes for local functions.  */
static void usage __P ((int status)) __attribute__ ((noreturn));
static int process_input __P ((FILE *input, const char *inname, DB *output,
			       int to_lowercase, int be_quiet));
static int print_database __P ((DB *db));


int
main (argc, argv)
     int argc;
     char *argv[];
{
  const char *output_name;
  const char *input_name;
  FILE *input_file;
  DB *db_file;
  int do_help;
  int do_version;
  int to_lowercase;
  int do_undo;
  int be_quiet;
  int status;
  int opt;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  textdomain (_libc_intl_domainname);

  /* Initialize local variables.  */
  do_help = 0;
  do_version = 0;
  to_lowercase = 0;
  do_undo = 0;
  be_quiet = 0;
  output_name = NULL;

  while ((opt = getopt_long (argc, argv, "fho:uV", long_options, NULL)) != EOF)
    switch (opt)
      {
      case '\0':        /* Long option.  */
        break;
      case 'h':
        do_help = 1;
        break;
      case 'f':
	to_lowercase = 1;
	break;
      case 'o':
        output_name = optarg;
        break;
      case 'q':
	be_quiet = 1;
	break;
      case 'u':
	do_undo = 1;
	break;
      case 'V':
        do_version = 1;
        break;
      default:
        usage (EXIT_FAILURE);
      }

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("makedb (GNU %s) %s\n", PACKAGE, VERSION);
      printf (_("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1996");
      printf (_("Written by %s.\n"), "Ulrich Drepper");

      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);
  else if (do_version)
    exit (EXIT_SUCCESS);

  /* Determine file names.  */
  if (do_undo || output_name != NULL)
    {
      if (optind + 1 != argc)
	{
	wrong_arguments:
	  error (0, 0, gettext ("wrong number of arguments"));
	  usage (EXIT_FAILURE);
	}
      input_name = argv[optind];
    }
  else
    {
      if (optind + 2 != argc)
	goto wrong_arguments;

      input_name = argv[optind++];
      output_name = argv[optind];
    }

  /* Special handling if we are asked to print the database.  */
  if (do_undo)
    {
      db_file = dbopen (input_name, O_RDONLY, 0666, DB_BTREE, NULL);
      if (db_file == NULL)
	error (EXIT_FAILURE, 0, gettext ("cannot open database file `%s': %s"),
	       input_name,
	       errno == EFTYPE ? gettext ("incorrectly formatted file")
			       : strerror (errno));

      status = print_database (db_file);

      db_file->close (db_file);

      return status;
    }

  /* Open input file.  */
  if (strcmp (input_name, "-") == 0 || strcmp (input_name, "/dev/stdin") == 0)
    input_file = stdin;
  else
    {
      input_file = fopen (input_name, "r");
      if (input_file == NULL)
	error (EXIT_FAILURE, errno, gettext ("cannot open input file `%s'"),
	       input_name);
    }

  /* Open output file.  This must not be standard output so we don't
     handle "-" and "/dev/stdout" special.  */
  db_file = dbopen (output_name, O_CREAT | O_RDWR | O_TRUNC, 0666,
		    DB_BTREE, NULL);
  if (db_file == NULL)
    error (EXIT_FAILURE, errno, gettext ("cannot open output file `%s'"));

  /* Start the real work.  */
  status = process_input (input_file, input_name, db_file, to_lowercase,
			  be_quiet);

  /* Close files.  */
  if (input_file != stdin)
    fclose (input_file);
  db_file->close (db_file);

  return status;
}


static void
usage (status)
     int status;
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, gettext ("Try `%s --help' for more information.\n"),
             program_invocation_name);
  else
    {
      printf (gettext ("\
Usage: %s [OPTION]... INPUT-FILE OUTPUT-FILE\n\
       %s [OPTION]... -o OUTPUT-FILE INPUT-FILE\n\
       %s [OPTION]... -u INPUT-FILE\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -f, --fold-case     convert key to lower case\n\
  -h, --help          display this help and exit\n\
  -o, --output=NAME   write output to file NAME\n\
      --quiet         don't print messages while building database\n\
  -u, --undo          print content of database file, one entry a line\n\
  -V, --version       output version information and exit\n\
If INPUT-FILE is -, input is read from standard input.\n"),
	      program_invocation_name, program_invocation_name,
	      program_invocation_name);
      fputs (gettext ("Report bugs to <bug-glibc@prep.ai.mit.edu>.\n"),
	     stdout);
    }

  exit (status);
}


static int
process_input (input, inname, output, to_lowercase, be_quiet)
     FILE *input;
     const char *inname;
     DB *output;
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

      while (isspace (*cp))
	++cp;

      val.data = cp;
      val.size = &line[n] - cp;

      /* Store the value.  */
      status = output->put (output, &key, &val, R_NOOVERWRITE);
      if (status != 0)
	{
	  if (status == 1)
	    {
	      if (!be_quiet)
		error_at_line (0, 0, inname, linenr,
			       gettext ("duplicate key"));
	      /* This is no real error.  Just give a warning.  */
	      status = 0;
	    }
	  else
	    error (0, errno, gettext ("while writing data base file"));

	  status = status ? EXIT_FAILURE : EXIT_SUCCESS;

	  clearerr (input);
	  break;
	}
    }

  if (ferror (input))
    {
      error (0, 0, gettext ("problems while reading `%s'"));
      status = EXIT_FAILURE;
    }

  return status;
}


static int
print_database (db)
     DB *db;
{
  DBT key;
  DBT val;
  int no_more;

  no_more = db->seq (db, &key, &val, R_FIRST);
  while (!no_more)
    {
      printf ("%.*s %.*s\n", (int) key.size, (char *) key.data, (int) val.size,
	      (char *) val.data);

      no_more = db->seq (db, &key, &val, R_NEXT);
    }

  if (no_more == -1)
    {
      error (0, errno, gettext ("while reading database"));
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
