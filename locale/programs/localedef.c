/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "error.h"
#include "charset.h"
#include "locfile.h"
#include "../intl/loadinfo.h"

/* Undefine the following line in the production version.  */
/* #define NDEBUG 1 */
#include <assert.h>


/* List of locale definition files which are used in `copy' instructions.  */
struct copy_def_list_t
{
  struct copy_def_list_t *next;

  const char *name;
  int mask;

  struct localedef_t *locale;

  struct
  {
    void *data;
    size_t len;
  } binary[6];
};


/* List of copied locales.  */
struct copy_def_list_t *copy_list;

/* If this is defined be POSIX conform.  */
int posix_conformance;

/* If not zero give a lot more messages.  */
int verbose;



/* Long options.  */
static const struct option long_options[] =
{
  { "charmap", required_argument, NULL, 'f' },
  { "code-set-name", required_argument, NULL, 'u' },
  { "help", no_argument, NULL, 'h' },
  { "force", no_argument, NULL, 'c' },
  { "inputfile", required_argument, NULL, 'i' },
  { "posix", no_argument, &posix_conformance, 1 },
  { "verbose", no_argument, &verbose, 1},
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


/* Prototypes for global functions.  */
void *xmalloc (size_t __n);

/* Prototypes for local functions.  */
static void usage (int status) __attribute__ ((noreturn));
static void error_print (void);
static const char *construct_output_path (char *path);


int
main (int argc, char *argv[])
{
  int optchar;
  int do_help = 0;
  int do_version = 0;
  int force_output = 0;
  const char *charmap_file = NULL;
  const char *input_file = NULL;
  const char *ucs_csn = NULL;
  const char *output_path;
  int cannot_write_why;
  struct charset_t *charset;
  struct localedef_t *localedef;
  struct copy_def_list_t *act_add_locdef;

  /* Set initial values for global varaibles.  */
  copy_list = NULL;
  posix_conformance = getenv ("POSIXLY_CORRECT") != NULL;
  error_print_progname = error_print;
  verbose = 0;

  /* Set locale.  Do not set LC_ALL because the other categories must
     not be affected (acccording to POSIX.2).  */
  setlocale (LC_MESSAGES, "");
  setlocale (LC_CTYPE, "");

  /* Initialize the message catalog.  */
  textdomain (_libc_intl_domainname);

  while ((optchar = getopt_long (argc, argv, "cf:hi:u:vV", long_options, NULL))
         != EOF)
    switch (optchar)
      {
      case '\0':		/* Long option.  */
        break;

      case 'c':
	force_output = 1;
	break;

      case 'f':
        charmap_file = optarg;
        break;

      case 'h':
        do_help = 1;
        break;

      case 'i':
	input_file = optarg;
        break;

      case 'u':
	ucs_csn = optarg;
	break;

	case 'v':
        verbose = 1;
        break;

      case 'V':
        do_version = 1;
        break;

      default:
        usage (4);	/* A value >3 is forced by POSIX.  */
        break;
      }

  /* POSIX.2 requires to be verbose about missing characters in the
     character map.  */
  verbose |= posix_conformance;

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("localedef (GNU %s) %s\n", PACKAGE, VERSION);
      printf (_("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1995, 1996");
      printf (_("Written by %s.\n"), "Ulrich Drepper");

      exit (0);
    }

  /* Help is requested.  */
  if (do_help)
    /* Possible violation: POSIX.2 4.35.8 defines the return value 0 as
       "No errors occured and the locale(s) were successfully created."
       But giving a other value than 0 does not make sense here.  It
       is perhaps not that important because POSIX does not specify the
       -h option for localedef.  */
    usage (0);

  if (argc - optind != 1)
    /* We need exactly one non-option parameter.  */
    usage (4);

  /* The parameter describes the output path of the constructed files.
     If the described files cannot be written return a NULL pointer.  */
  output_path  = construct_output_path (argv[optind]);
  cannot_write_why = errno;

  /* Now that the parameters are processed we have to reset the local
     ctype locale.  (P1003.2 4.35.5.2)  */
  setlocale (LC_CTYPE, "POSIX");

  /* Look whether the system really allows locale definitions.  POSIX
     defines error code 3 for this situation so I think it must be
     a fatal error (see P1003.2 4.35.8).  */
  if (sysconf (_SC_2_LOCALEDEF) < 0)
    error (3, 0, _("FATAL: system does not define `_POSIX2_LOCALEDEF'"));

  /* Process charmap file.  */
  charset = charmap_read (charmap_file);

  /* Now read the locale file.  */
  localedef = locfile_read (input_file, charset);
  if (localedef->failed != 0)
    error (4, errno, _("cannot open locale definition file `%s'"), input_file);

  /* Perhaps we saw some `copy' instructions.  Process the given list.
     We use a very simple algorithm: we look up the list from the
     beginning every time.  */
  do
    {
      int cat;

      for (act_add_locdef = copy_list; act_add_locdef != NULL;
	   act_add_locdef = act_add_locdef->next)
	{
	  for (cat = LC_CTYPE; cat <= LC_MESSAGES; ++cat)
	    if ((act_add_locdef->mask & (1 << cat)) != 0)
	      {
		act_add_locdef->mask &= ~(1 << cat);
		break;
	      }
	  if (cat <= LC_MESSAGES)
	    break;
	}

      if (act_add_locdef != NULL)
	{
	  int avail = 0;

	  if (act_add_locdef->locale == NULL)
	    act_add_locdef->locale = locfile_read (act_add_locdef->name,
						   charset);

	  if (! act_add_locdef->locale->failed)
	    {
	      avail = act_add_locdef->locale->categories[cat].generic != NULL;
	      if (avail)
		localedef->categories[cat].generic
		  = act_add_locdef->locale->categories[cat].generic;
	    }

	  if (! avail)
	    {
	      const char *locale_names[] = { "LC_COLLATE", "LC_CTYPE",
					     "LC_MONETARY", "LC_NUMERIC",
					     "LC_TIME", "LC_MESSAGES" };
	      char *fname;
	      int fd;
	      struct stat st;

	      asprintf (&fname, LOCALE_PATH "/%s/%s", act_add_locdef->name,
			locale_names[cat]);
	      fd = open (fname, O_RDONLY);
	      if (fd == -1)
		{
		  free (fname);

		  asprintf (&fname, LOCALE_PATH "/%s/%s/SYS_%s",
			    act_add_locdef->name, locale_names[cat],
			    locale_names[cat]);

		  fd = open (fname, O_RDONLY);
		  if (fd == -1)
		    error (5, 0, _("\
locale file `%s', used in `copy' statement, not found"),
			   act_add_locdef->name);
		}

	      if (fstat (fd, &st) < 0)
		error (5, errno, _("\
cannot `stat' locale file `%s'"),
		       fname);

	      localedef->len[cat] = st.st_size;
	      localedef->categories[cat].generic
		= mmap (NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	      if (localedef->categories[cat].generic == (void *) -1)
		{
		  size_t left = st.st_size;
		  void *read_ptr;

		  localedef->categories[cat].generic
		    = xmalloc (st.st_size);
		  read_ptr = localedef->categories[cat].generic;

		  do
		    {
		      long int n;
		      n = read (fd, read_ptr, left);
		      if (n == 1)
			error (5, errno, _("cannot read locale file `%s'"),
			       fname);
		      read_ptr += n;
		      left -= n;
		    }
		  while (left > 0);
		}

	      close (fd);
	      free (fname);

	      localedef->binary |= 1 << cat;
	    }
	}
    }
  while (act_add_locdef != NULL);

  /* Check the categories we processed in source form.  */
  check_all_categories (localedef, charset);

  /* We are now able to write the data files.  If warning were given we
     do it only if it is explicitly requested (--force).  */
  if (error_message_count == 0 || force_output != 0)
    {
      if (cannot_write_why != 0)
	error (4, cannot_write_why, _("cannot write output files to `%s'"),
	       output_path);
      else
	write_all_categories (localedef, charset, output_path);
    }
  else
    error (4, 0, _("no output file produced because warning were issued"));

  /* This exit status is prescribed by POSIX.2 4.35.7.  */
  exit (error_message_count != 0);
}


void
def_to_process (const char *name, int category)
{
  struct copy_def_list_t *new, **rp;

  for (rp = &copy_list; *rp != NULL; rp = &(*rp)->next)
    if (strcmp (name, (*rp)->name) == 0)
      break;

  if (*rp == NULL)
    {
      size_t cnt;

      *rp = (struct copy_def_list_t *) xmalloc (sizeof (**rp));

      (*rp)->next = NULL;
      (*rp)->name = name;
      (*rp)->mask = 0;
      (*rp)->locale = NULL;

      for (cnt = 0; cnt < 6; ++cnt)
	{
	  (*rp)->binary[cnt].data = NULL;
	  (*rp)->binary[cnt].len = 0;
	}
    }
  new = *rp;

  if ((new->mask & category) != 0)
    /* We already have the information.  This cannot happen.  */
    error (5, 0, _("\
category data requested more than once: should not happen"));

  new->mask |= category;
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_invocation_name);
  else
    {
      printf (_("\
Usage: %s [OPTION]... name\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -c, --force               create output even if warning messages were issued\n\
  -h, --help                display this help and exit\n\
  -f, --charmap=FILE        symbolic character names defined in FILE\n\
  -i, --inputfile=FILE      source definitions are found in FILE\n\
  -u, --code-set-name=NAME  specify code set for mapping ISO 10646 elements\n\
  -v, --verbose             print more messages\n\
  -V, --version             output version information and exit\n\
      --posix               be strictly POSIX conform\n\
\n\
System's directory for character maps: %s\n\
                       locale files  : %s\n"),
	      program_invocation_name, CHARMAP_PATH, LOCALE_PATH);
      fputs (gettext ("Report bugs to <bug-glibc@prep.ai.mit.edu>.\n"),
	     stdout);
    }

  exit (status);
}


/* The address of this function will be assigned to the hook in the error
   functions.  */
static void
error_print ()
{
  /* We don't want the program name to be printed in messages.  Emacs'
     compile.el does not like this.  */
}


/* The parameter to localedef describes the output path.  If it does
   contain a '/' character it is a relativ path.  Otherwise it names the
   locale this definition is for.  */
static const char *
construct_output_path (char *path)
{
  const char *normal = NULL;
  char *result;

  if (strchr (path, '/') == NULL)
    {
      /* This is a system path.  First examine whether the locale name
	 contains a reference to the codeset.  This should be
	 normalized.  */
      char *startp, *endp;

      startp = path;
      /* We must be prepared for finding a CEN name or a location of
	 the introducing `.' where it is not possible anymore.  */
      while (*startp != '\0' && *startp != '@' && *startp != '.'
	     && *startp != '+' && *startp != ',')
	++startp;
      if (*startp == '.')
	{
	  /* We found a codeset specification.  Now find the end.  */
	  endp = ++startp;
	  while (*endp != '\0' && *endp != '@')
	    ++endp;

	  if (endp > startp)
	    normal = _nl_normalize_codeset (startp, endp - startp);
	}
      else
	/* This is to keep gcc quiet.  */
	endp = NULL;

      /* We put an additional '\0' at the end of the string because at
	 the end of the function we need another byte for the trailing
	 '/'.  */
      if (normal == NULL)
	asprintf (&result, "%s/%s\0", LOCALE_PATH, path);
      else
	asprintf (&result, "%s/%.*s%s%s\0", LOCALE_PATH, startp - path, path,
		  normal, endp);
    }
  else
    {
      /* This is a user path.  Please note the additional byte in the
	 memory allocation.  */
      result = xmalloc (strlen (path) + 2);
      strcpy (result, path);
    }

  errno = 0;

  if (euidaccess (result, W_OK) == -1)
    /* Perhaps the directory does not exist now.  Try to create it.  */
    if (errno == ENOENT)
      {
	errno = 0;
	mkdir (result, 0777);
      }

  strcat (result, "/");

  return result;
}
