/* Copyright (C) 1995 Free Software Foundation, Inc.

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

#include <getopt.h>
#include <libintl.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "localedef.h"

/* The charmap file used.  If none given DEFAULT_CHARMAP is used.  */
static char *charmap_file;

/* If set output is always written, even when warning are given.  */
static int force_output;

/* The input file name.  */
static char *input_file;

/* Path leading to the destination directory for the produced files.  */
char *output_path;

/* If this is defined be POSIX conform.  */
int posix_conformance;

/* If not zero give a lot more messages.  */
int verbose;

/* Long options.  */
static const struct option long_options[] =
  {
    { "charmap", required_argument, NULL, 'f' },
    { "debug", no_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { "force", no_argument, NULL, 'c' },
    { "inputfile", required_argument, NULL, 'i' },
    { "posix", no_argument, &posix_conformance, 1 },
    { "verbose", no_argument, &verbose, 1},
    { "version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
  };


/* This is defined in error-msg.h.  */
extern int warning_cntr;


/* Prototypes for local functions.  */
static void usage (int status) __attribute__ ((noreturn));
static int construct_output_path (const char *path);

int
main(int argc, char *argv[])
{
  int optchar;
  int cannot_write;
  int do_help = 0;
  int do_version = 0;

  /* Set initial values for global varaibles.  */
  charmap_file = NULL;
  force_output = 0;
  input_file = 0;
  posix_conformance = getenv ("POSIXLY_CORRECT") != NULL;
  verbose = 0;

  /* Set locale.  Do not set LC_ALL because the other categories must
     not be affected (acccording to POSIX.2).  */
  setlocale (LC_MESSAGES, "");
  setlocale (LC_CTYPE, "");

  /* Initialize the message catalog.  */
  textdomain (PACKAGE);

  while ((optchar = getopt_long (argc, argv, "cdf:hi:vV", long_options, NULL))
	 != EOF)
    switch (optchar)
      {
      case '\0':
	break;
      case 'c':
	force_output = 1;
	break;
      case 'f':
	if (charmap_file != NULL)
	  error (0, 0, gettext ("\"%s %s\" overwrites old option \"%s\""),
		 "-f", optarg, charmap_file);
	charmap_file = optarg;
	break;
      case 'h':
	do_help = 1;
	break;
      case 'i':
	if (input_file != NULL)
	  error (0, 0, gettext ("\"%s %s\" overwrites old option \"%s\""),
		 "-i", optarg, input_file);
	input_file = optarg;
	break;
      case 'v':
	verbose = 1;
	break;
      case 'V':
	do_version = 1;
	break;
      default:
	usage (4);
	break;
      }

  /* POSIX.2 requires to be verbose about missing characters in the
     character map.  */
  verbose |= posix_conformance;

  /* Version information is requested.  */
  if (do_version)
    {
      fprintf (stderr, "GNU %s %s\n", PACKAGE, VERSION);
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (0);

  if (argc - optind != 1)
    /* We need exactly one non-option parameter.  */
    usage (4);

  /* The parameter describes the output path of the constructed files.
     If the files cannot be written return a non-zero value.  */
  cannot_write = construct_output_path (argv[optind]);

  /* Now that the parameters are processed we have to reset the local
     ctype locale.  (POSIX.2 4.35.5.2)  */
  setlocale (LC_CTYPE, "POSIX");

  /* Look whether the system really allows locale definitions.  */
  if (sysconf (_SC_2_LOCALEDEF) < 0)
    error (3, 0,
	   gettext ("warning: system does not define `_POSIX2_LOCALEDEF'"));

  /* Process charmap file.  */
  charmap_read (charmap_file);

  /* Now read the locale file.  */
  locfile_read (input_file);

  /* Check all categories for consistency.  */
  categories_check ();

  /* We are now able to write the data files.  If warning were given we
     do it only if it is explicitly requested (--force).  */
  if (warning_cntr == 0 || force_output != 0)
    if (cannot_write != 0)
      error (0, 0, gettext ("cannot write output file `%s': %s"),
	     output_path, strerror (cannot_write));
    else
      categories_write ();
  else
    error (0, 0,
	   gettext ("no output file produced because warning were issued"));

  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage(int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, gettext ("Try `%s --help' for more information.\n"),
	     program_invocation_name);
  else
    printf(gettext ("\
Usage: %s [OPTION]... name\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -c, --force           create output even if warning messages have been issued\n\
  -h, --help            display this help and exit\n\
  -V, --version         output version information and exit\n\
\n\
  -i, --inputfile=FILE  source definitions are found in FILE\n\
  -f, --charmap=FILE    symbolic character names defined in FILE\n\
\n\
  -v, --verbose         print more messages\n\
      --posix           be strictly POSIX conform\n\
\n\
System's directory for character maps: %s\n\
                       locale files  : %s\n\
"), program_invocation_name, CHARMAP_PATH, LOCALE_PATH);

  exit (status);
}


/* The parameter to localedef describes the output path.  If it does
   contain a '/' character it is a relativ path.  Otherwise it names the
   locale this definition is for.  */
static int
construct_output_path (const char *path)
{
  int result = 0;

  if (strchr (path, '/') == NULL)
    {
      /* This is a system path.  */
      int path_max_len = pathconf (LOCALE_PATH, _PC_PATH_MAX) + 1;
      output_path = (char *) xmalloc (path_max_len);

      snprintf (output_path, path_max_len, "%s/%s", LOCALE_PATH, path);
    }
  else
    {
      char *t;
      /* This is a user path.  */
      output_path = malloc (strlen (path) + 2);
      t = stpcpy (output_path, path);
      *t = '\0';
    }

  if (euidaccess (output_path, W_OK) == -1)
    /* Perhaps the directory does not exist now.  Try to create it.  */
    if (errno == ENOENT)
      {
	if (mkdir (output_path, 0777) == -1)
	  result = errno;
      }
    else
      result = errno;

  if (result == 0)
    strcat (output_path, "/");

  return result;
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
